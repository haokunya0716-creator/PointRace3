#include <stdint.h>

#include "app_motor.h"
#include "app_speed.h"
#include "app_vl5310x.h"
#include "clock.h"

/*
 * 任务文件总说明
 * --------------------------------------------------------------------------
 * 本文件目前主要包含 Task1、Task2、Task4 的流程控制。
 *
 * Task2 是“4x4 场地遍历 + 挡板避障 + 从 C 口驶出”的状态机。场地坐标约定：
 *   1. x 从左到右为 0、1、2、3。
 *   2. y 从上到下为 0、1、2、3。
 *   3. 方向 dir 使用 0/1/2/3 表示 上/右/下/左。
 *   4. 小车从 A 口进入后，软件认为它先到达 (0,2)，车头朝右。
 *
 * Task2 的整体路线不是一次性写死的：
 *   1. 先从 A 口直行进入 (0,2)。
 *   2. 沿外圈固定走 12 格，外圈走的时候中间四格一直在车左侧，
 *      所以每到一个格子中心只用左侧 VL53L0X 扫描“外圈到中间四格”的挡板。
 *   3. 外圈扫完后开始进入中间四格：(1,1)、(2,1)、(1,2)、(2,2)。
 *   4. 在中间搜索阶段，每到一个格子中心停下，用前/左/右三个传感器测挡板；
 *      连续 3 次都在障碍距离范围内，才把这条格子边记录为不可走。
 *   5. 每次记录完挡板后，不会继续沿旧路线走，而是重新 BFS 找路。
 *   6. 中间四格全部访问完后，BFS 规划到出口格，再转向右侧驶出场地。
 *
 * BFS 的“最近”含义：
 *   1. 最近只按格子步数算，不按实际厘米距离，也不把转弯次数计入代价。
 *   2. 搜索队列按层展开，先找到的未访问中间格就是目标。
 *   3. 同样步数时，方向遍历顺序固定为 上、右、下、左。
 *   4. task2_block[x][y][dir] 为 1 的边不会被 BFS 使用；边界也在初始化时
 *      当成 block 处理，所以小车不会规划到场地外。
 *
 * Task4 在 Task2 基础上只改出口：
 *   1. Task4_SetExitByNumber(1) -> 出口格 (3,0)。
 *   2. Task4_SetExitByNumber(2) -> 出口格 (3,1)，这也是 Task2 的默认 C 口。
 *   3. Task4_SetExitByNumber(3) -> 出口格 (3,2)。
 *   4. Task4_SetExitByNumber(4) -> 出口格 (3,3)。
 *   5. Task4 启动时设置一次出口，然后持续调用 Task2() 复用同一套避障和 BFS。
 *   6. Task4 结束时会把出口复位为 Task2 默认值，避免影响后续单独执行 Task2。
 */

#define TASK2_GRID_CM 45.0f
#define TASK2_DEFAULT_EXIT_X  3
#define TASK2_DEFAULT_EXIT_Y  1
#define TASK2_DEFAULT_EXIT_DIR 1
#define TASK2_EXIT_X task2_exit_x
#define TASK2_EXIT_Y task2_exit_y
#define TASK2_EXIT_DIR task2_exit_dir
#define TASK2_TURN_WAIT_MS 150

extern uint8_t task1_flag;
extern uint8_t task2_flag;
extern uint8_t task4_flag;

static uint8_t task1_state = 0;

// Task2 默认从右侧 C 口中间格 (3,1) 出场；Task4 会临时改这三个变量。
static uint8_t task2_exit_x = TASK2_DEFAULT_EXIT_X;
static uint8_t task2_exit_y = TASK2_DEFAULT_EXIT_Y;
static uint8_t task2_exit_dir = TASK2_DEFAULT_EXIT_DIR;

// Task4 只是 Task2 的出口可配置包装层；task4_active 用来区分当前是谁启动了 Task2。
static uint8_t task4_active = 0;
static uint8_t task4_exit_number = 2;

static uint8_t task2_state = 0;
static uint8_t task2_x = 0;
static uint8_t task2_y = 2;
static uint8_t task2_dir = 1;
static uint8_t task2_target_dir = 1;
static uint8_t task2_after_turn_state = 0;
static uint8_t task2_turn_final_dir = 1;
static uint8_t task2_turn_final_state = 0;
static uint8_t task2_visited[4][4] = {0};
static uint8_t task2_block[4][4][4] = {0};
static uint8_t task2_path[16] = {0};
static uint8_t task2_path_len = 0;
static uint8_t task2_check_count = 0;
static uint8_t task2_front_count = 0;
static uint8_t task2_left_count = 0;
static uint8_t task2_right_count = 0;
static uint8_t task2_outer_step = 0;
static unsigned long task2_check_time = 0;
static unsigned long task2_turn_wait_time = 0;
static float task2_yaw_base = 0.0f;

// 从 A 口进入 (0,2) 后，按这个方向序列绕外圈一圈并回到 (0,2)。
static const uint8_t task2_outer_dir[12] = {
    2, 1, 1, 1, 0, 0, 0, 3, 3, 3, 2, 2
};

static void Task2_ResetExit(void)
{
    task2_exit_x = TASK2_DEFAULT_EXIT_X;
    task2_exit_y = TASK2_DEFAULT_EXIT_Y;
    task2_exit_dir = TASK2_DEFAULT_EXIT_DIR;
}

static void Task2_SetExit(uint8_t x, uint8_t y)
{
    task2_exit_x = x;
    task2_exit_y = y;
    task2_exit_dir = 1;
}

/*
 * 把 (x,y) 变成 0~15 的一维编号。
 * BFS 队列、used、prev、prev_dir 都用这个编号访问数组，
 * 比直接用二维数组更省空间，也方便从终点反推回起点。
 */
static uint8_t Task2_NodeId(uint8_t x, uint8_t y)
{
    return y * 4 + x;
}

/*
 * 根据当前格子和方向计算相邻格子。
 * dir 约定：
 *   0: 上，y - 1
 *   1: 右，x + 1
 *   2: 下，y + 1
 *   3: 左，x - 1
 * 如果目标格子越过 4x4 场地边界，返回 0；否则写出 nx、ny 并返回 1。
 */
static uint8_t Task2_NextCell(uint8_t x, uint8_t y, uint8_t dir, uint8_t *nx, uint8_t *ny)
{
    int8_t tx = x;
    int8_t ty = y;

    if(dir == 0){
        ty--;
    }else if(dir == 1){
        tx++;
    }else if(dir == 2){
        ty++;
    }else {
        tx--;
    }

    if(tx < 0 || tx > 3 || ty < 0 || ty > 3){
        return 0;
    }

    *nx = tx;
    *ny = ty;

    return 1;
}

// 返回某个方向的反方向，用于把挡板同时标到相邻格子的反向边上。
static uint8_t Task2_OppDir(uint8_t dir)
{
    return (dir + 2) % 4;
}

/*
 * 把软件方向 dir 转成陀螺仪 yaw 目标。
 * 车头朝右作为 0 度基准，因此：
 *   dir=0 上 -> -90 度
 *   dir=1 右 ->   0 度
 *   dir=2 下 ->  90 度
 *   dir=3 左 -> 180/-180 度
 * task2_yaw_base 是任务开始瞬间的车身朝向，用来吸收放车时的初始角度误差。
 */
static float Task2_GetYawRef(uint8_t dir)
{
    float angle = ((float)dir - 1.0f) * 90.0f;

    while(angle > 180.0f){
        angle -= 360.0f;
    }

    while(angle < -180.0f){
        angle += 360.0f;
    }

    return task2_yaw_base + angle;
}

/*
 * 记录一条不可通过的边。
 * 例如从 (0,2) 向右检测到挡板，不只记录 task2_block[0][2][右]，
 * 还要同步记录 task2_block[1][2][左]。这样 BFS 无论从哪一侧搜索，
 * 都不会穿过同一块挡板。
 */
static void Task2_MarkBlock(uint8_t x, uint8_t y, uint8_t dir)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    task2_block[x][y][dir] = 1;

    if(Task2_NextCell(x, y, dir, &nx, &ny)){
        task2_block[nx][ny][Task2_OppDir(dir)] = 1;
    }
}

// 判断某个方向是否可走：已经标记为 block 或者会越界时，都返回不可走。
static uint8_t Task2_CanGo(uint8_t x, uint8_t y, uint8_t dir)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    if(task2_block[x][y][dir]){
        return 0;
    }

    return Task2_NextCell(x, y, dir, &nx, &ny);
}

// 小车每真正到达一个格子中心后调用，表示这个格子已经访问过。
static void Task2_MarkVisited(uint8_t x, uint8_t y)
{
    task2_visited[x][y] = 1;
}

// 第二问只要求进入中间 2x2 区域；这里集中定义中间四格的范围。
static uint8_t Task2_IsMiddleCell(uint8_t x, uint8_t y)
{
    return (x >= 1 && x <= 2 && y >= 1 && y <= 2) ? 1 : 0;
}

// 检查中间四格是否都访问过；全部访问后就不再继续找未访问格，而是去出口。
static uint8_t Task2_MiddleDone(void)
{
    uint8_t x = 0;
    uint8_t y = 0;

    for(x = 1; x <= 2; x++){
        for(y = 1; y <= 2; y++){
            if(task2_visited[x][y] == 0){
                return 0;
            }
        }
    }

    return 1;
}

// 边界方向本来就是场地外墙，传感器即使看到边墙，也不再额外记录随机挡板。
static uint8_t Task2_IsBoundaryDir(uint8_t x, uint8_t y, uint8_t dir)
{
    if(dir == 0 && y == 0){
        return 1;
    }else if(dir == 1 && x == 3){
        return 1;
    }else if(dir == 2 && y == 3){
        return 1;
    }else if(dir == 3 && x == 0){
        return 1;
    }

    return 0;
}

/*
 * 直接根据 VL53L0X 当前距离判断是否在挡板范围。
 * 阈值来自 app_vl5310x.h：
 *   VL5310X_OBSTACLE_MIN_MM < distance < VL5310X_OBSTACLE_MAX_MM
 * 当前代码后面还会做 3 次连续确认，避免一次测距毛刺就把边误记成挡板。
 */
static uint8_t Task2_IsObstacle(VL5310X_SensorId_t id)
{
    uint16_t distance = App_VL5310X_GetDistance(id);

    if(distance > VL5310X_OBSTACLE_MIN_MM && distance < VL5310X_OBSTACLE_MAX_MM){
        return 1;
    }

    return 0;
}

/*
 * 把某个传感器方向看到的挡板写入地图。
 * only_middle=1 用于外圈扫描：外圈走的时候只想记录“外圈到中间四格”的挡板，
 * 不希望把外圈普通通道误记为挡板，所以目标格不是中间四格时直接忽略。
 * only_middle=0 用于中间搜索：前、左、右看到的有效挡板都记录。
 */
static void Task2_MarkSensorWall(uint8_t dir, uint8_t only_middle)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    if(Task2_IsBoundaryDir(task2_x, task2_y, dir)){
        return;
    }

    if(Task2_NextCell(task2_x, task2_y, dir, &nx, &ny) == 0){
        return;
    }

    // 外圈扫描时只记录朝向中间四格的边，避免把外圈普通边误记成挡板。
    if(only_middle && Task2_IsMiddleCell(nx, ny) == 0){
        return;
    }

    Task2_MarkBlock(task2_x, task2_y, dir);
}

/*
 * 在格子中心进行挡板确认。
 * only_left=1：外圈扫描阶段，只看左侧传感器，因为中间区域一直在车左边。
 * only_left=0：中间搜索阶段，看前、左、右三个方向。
 * 每 50ms 采样一次，总共采 3 次；某一路 3 次都命中，才认为这一边有挡板。
 */
static uint8_t Task2_CheckWallDone(uint8_t only_left)
{
    uint8_t front_dir = task2_dir;
    uint8_t left_dir = (task2_dir + 3) % 4;
    uint8_t right_dir = (task2_dir + 1) % 4;

    if(task2_check_count == 0){
        task2_front_count = 0;
        task2_left_count = 0;
        task2_right_count = 0;
    }

    if(task2_check_count == 0 || HAL_GetTick() - task2_check_time >= 50){
        task2_check_time = HAL_GetTick();
        App_VL5310X_Proc();

        // 每次停在格子中心时采 3 次，3 次都在障碍距离范围内才算挡板。
        if(only_left == 0 && Task2_IsObstacle(VL5310X_FRONT)){
            task2_front_count++;
        }

        if(Task2_IsObstacle(VL5310X_LEFT)){
            task2_left_count++;
        }

        if(only_left == 0 && Task2_IsObstacle(VL5310X_RIGHT)){
            task2_right_count++;
        }

        task2_check_count++;
    }

    if(task2_check_count >= 3){
        if(only_left == 0 && task2_front_count >= 3){
            Task2_MarkSensorWall(front_dir, 0);
        }

        if(task2_left_count >= 3){
            Task2_MarkSensorWall(left_dir, only_left);
        }

        if(only_left == 0 && task2_right_count >= 3){
            Task2_MarkSensorWall(right_dir, 0);
        }

        task2_check_count = 0;
        return 1;
    }

    return 0;
}

// 单格运动距离统一从这里取，后期实车如果 45cm 有误差，只改 TASK2_GRID_CM。
static float Task2_GetStepLen(void)
{
    return TASK2_GRID_CM;
}

/*
 * 初始化 Task2 地图和状态。
 * visited 清零表示中间四格还没访问；
 * block 清零后，再把 4x4 场地外边界全部标成不可走，防止 BFS 规划出界。
 */
static void Task2_InitMap(void)
{
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t d = 0;

    for(x = 0; x < 4; x++){
        for(y = 0; y < 4; y++){
            task2_visited[x][y] = 0;

            for(d = 0; d < 4; d++){
                task2_block[x][y][d] = 0;
            }
        }
    }

    // 四周边墙不能走，先当作已知挡板记录。
    for(x = 0; x < 4; x++){
        Task2_MarkBlock(x, 0, 0);
        Task2_MarkBlock(x, 3, 2);
    }

    for(y = 0; y < 4; y++){
        Task2_MarkBlock(0, y, 3);
        Task2_MarkBlock(3, y, 1);
    }

    task2_x = 0;
    task2_y = 2;
    task2_dir = 1;
    task2_target_dir = 1;
    task2_turn_final_dir = 1;
    task2_turn_final_state = 0;
    task2_path_len = 0;
    task2_check_count = 0;
    task2_front_count = 0;
    task2_left_count = 0;
    task2_right_count = 0;
    task2_outer_step = 0;
    task2_check_time = 0;
    task2_turn_wait_time = 0;
}

/*
 * BFS 路线规划。
 * find_unvisited=1：
 *   从当前格子开始找最近的、未访问过的中间四格。这里的最近是“最少格子步数”。
 * find_unvisited=0：
 *   从当前格子开始找指定目标 (target_x,target_y)，用于中间四格完成后去出口。
 *
 * BFS 的队列按层展开：
 *   第 0 层是当前位置；
 *   第 1 层是一步可到的格子；
 *   第 2 层是两步可到的格子；
 *   以此类推。所以第一次找到的目标一定是步数最少的目标。
 *
 * 平局规则：
 *   for(dir = 0; dir < 4; dir++) 固定按 上、右、下、左 入队。
 *   如果两个目标步数相同，谁先按这个顺序入队、先出队，谁就会被选中。
 *
 * 输出：
 *   task2_path[] 保存从当前位置到目标的方向序列；
 *   task2_path[0] 就是下一格要走的方向；
 *   task2_path_len 为方向个数。
 */
static uint8_t Task2_BuildPath(uint8_t find_unvisited, uint8_t target_x, uint8_t target_y)
{
    uint8_t que[16] = {0};
    uint8_t used[16] = {0};
    uint8_t prev[16] = {0};
    uint8_t prev_dir[16] = {0};
    uint8_t head = 0;
    uint8_t tail = 0;
    uint8_t found = 0xff;
    uint8_t start = Task2_NodeId(task2_x, task2_y);
    uint8_t i = 0;

    for(i = 0; i < 16; i++){
        prev[i] = 0xff;
        prev_dir[i] = 0xff;
    }

    que[tail++] = start;
    used[start] = 1;

    while(head < tail){
        uint8_t id = que[head++];
        uint8_t x = id % 4;
        uint8_t y = id / 4;
        uint8_t dir = 0;

        if(find_unvisited && Task2_IsMiddleCell(x, y) && task2_visited[x][y] == 0){
            found = id;
            break;
        }

        if(find_unvisited == 0 && x == target_x && y == target_y){
            found = id;
            break;
        }

        for(dir = 0; dir < 4; dir++){
            uint8_t nx = 0;
            uint8_t ny = 0;
            uint8_t nid = 0;

            if(Task2_CanGo(x, y, dir) == 0){
                continue;
            }

            Task2_NextCell(x, y, dir, &nx, &ny);
            nid = Task2_NodeId(nx, ny);

            if(used[nid]){
                continue;
            }

            used[nid] = 1;
            prev[nid] = id;
            prev_dir[nid] = dir;
            que[tail++] = nid;
        }
    }

    if(found == 0xff){
        task2_path_len = 0;
        return 0;
    }

    task2_path_len = 0;

    while(found != start){
        task2_path[task2_path_len++] = prev_dir[found];
        found = prev[found];
    }

    // BFS 是从终点往回找的，这里反过来，path[0] 才是下一步方向。
    for(i = 0; i < task2_path_len / 2; i++){
        uint8_t t = task2_path[i];
        task2_path[i] = task2_path[task2_path_len - 1 - i];
        task2_path[task2_path_len - 1 - i] = t;
    }

    return 1;
}

/*
 * 发起转向。
 * next_state 表示转向完成并等待 TASK2_TURN_WAIT_MS 后要进入哪个状态。
 * 如果目标方向和当前方向一样，不需要真正转向，直接进入 next_state。
 * 如果需要 180 度转向，为了减少实车超调，拆成两个 90 度：
 *   当前方向 -> 中间方向 -> 最终方向。
 */
static void Task2_StartTurn(uint8_t dir, uint8_t next_state)
{
    uint8_t diff = (dir + 4 - task2_dir) % 4;
    uint8_t mid_dir = 0;
    float angle = 0.0f;

    task2_target_dir = dir;
    task2_after_turn_state = next_state;

    if(diff == 0){
        task2_state = task2_after_turn_state;
        return;
    }

    // 第二问里 180 度直接转容易超调，所以拆成两次 90 度。
    if(diff == 2){
        task2_turn_final_dir = dir;
        task2_turn_final_state = next_state;
        mid_dir = (task2_dir + 1) % 4;
        task2_target_dir = mid_dir;
        task2_after_turn_state = 3;
        dir = mid_dir;
    }

    App_Update_Data();
    angle = Task2_GetYawRef(dir) - angle_yaw;

    while(angle > 180.0f){
        angle -= 360.0f;
    }

    while(angle < -180.0f){
        angle += 360.0f;
    }

    Set_Angle_SP(angle);
    task2_state = 2;
}

// 走完一格后，根据当前车头方向更新软件坐标，并把新格子标为已访问。
static void Task2_UpdatePos(void)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    if(Task2_NextCell(task2_x, task2_y, task2_dir, &nx, &ny)){
        task2_x = nx;
        task2_y = ny;
        Task2_MarkVisited(task2_x, task2_y);
    }
}

/*
 * 结束当前任务。
 * 普通 Task2 结束时只需要清 task2_flag。
 * Task4 借用 Task2 状态机时，会在 Task4() 包装函数里检测 task2_state 已回到 0，
 * 然后清 task4_flag 并复位出口，避免在 Task2_Finish() 里混入任务四专用逻辑。
 */
static void Task2_Finish(void)
{
    App_Speed_Set(0.0f, 0.0f);
    task2_state = 0;
    task2_flag = 0;
}

void Task1(void)
{
    if(task1_state == 0){
        Set_Position_SP(90.0f);
        task1_state = 1;
    }else if(task1_state == 1){
        App_Position_Pro();

        if(motor_pos_done){
            Set_Angle_SP(-16.0f);
            task1_state = 2;
        }
    }else if(task1_state == 2){
        App_Angle_Pro();

        if(motor_angle_done){
            Set_Position_SP(200.0f);
            task1_state = 3;
        }
    }else if(task1_state == 3){
        App_Position_Pro();

        if(motor_pos_done){
            App_Speed_Set(0.0f, 0.0f);
            task1_state = 0;
            task1_flag = 0;
        }
    }
}

void Task2(void)
{
    if(task2_state == 0){
        // 初始化地图，然后从 A 口进入左侧第三行的格子中心 (0,2)。
        // 进场距离不是标准一格，所以单独写 80cm；到位后才开始外圈扫描。
        Task2_InitMap();
        App_Update_Data();
        task2_yaw_base = angle_yaw;
        Set_Position_Angle_SP(80.0f, Task2_GetYawRef(task2_dir));
        task2_state = 1;
    }else if(task2_state == 1){
        // state 1：等待进场 80cm 完成。完成后标记 (0,2) 已访问，
        // 再转向外圈扫描路线的第一个方向。
        App_Position_Pro();

        if(motor_pos_done){
            Task2_MarkVisited(task2_x, task2_y);
            Task2_StartTurn(task2_outer_dir[task2_outer_step], 10);
        }
    }else if(task2_state == 10){
        // state 10：外圈每到一个格子中心先测挡板。
        // 外圈阶段只看左侧，因为中间区域一直在车左侧；
        // 连续 3 次确认完成后，才开始走下一格。
        if(Task2_CheckWallDone(1) == 0){
            return;
        }

        Set_Position_Angle_SP(Task2_GetStepLen(), Task2_GetYawRef(task2_dir));
        task2_state = 11;
    }else if(task2_state == 11){
        // state 11：外圈按当前方向走一格。
        // 到下一个格子中心后更新坐标，再决定继续外圈还是转入中间搜索。
        App_Position_Pro();

        if(motor_pos_done){
            Task2_UpdatePos();
            task2_outer_step++;

            if(task2_outer_step >= 12){
                task2_state = 20;
            }else {
                Task2_StartTurn(task2_outer_dir[task2_outer_step], 10);
            }
        }
    }else if(task2_state == 20){
        // state 20：中间搜索的决策状态。
        // 如果中间四格都已经访问过，就不再新增挡板数据，直接规划去出口；
        // 这样可以避免最后一个中间格附近的误读改变出场路径。
        if(Task2_MiddleDone()){
            if(task2_x == TASK2_EXIT_X && task2_y == TASK2_EXIT_Y){
                Task2_StartTurn(TASK2_EXIT_DIR, 30);
            }else if(Task2_BuildPath(0, TASK2_EXIT_X, TASK2_EXIT_Y)){
                Task2_StartTurn(task2_path[0], 21);
            }else {
                Task2_Finish();
            }
            return;
        }

        // 中间四格还没全访问完：停在格子中心，用前/左/右三个方向记录挡板，
        // 然后 BFS 找最近的未访问中间格。
        if(Task2_CheckWallDone(0) == 0){
            return;
        }

        if(Task2_BuildPath(1, 0, 0)){
            Task2_StartTurn(task2_path[0], 21);
        }else {
            Task2_Finish();
        }
    }else if(task2_state == 2){
        // state 2：公共转向状态。角度闭环完成后，更新软件方向 task2_dir，
        // 再停 150ms，等车身稳定后进入 task2_after_turn_state。
        App_Angle_Pro();

        if(motor_angle_done){
            task2_dir = task2_target_dir;
            task2_turn_wait_time = HAL_GetTick();
            task2_state = 4;
        }
    }else if(task2_state == 3){
        // state 3：180 度转向的第二个 90 度。
        // 第一个 90 度完成后会短暂停车，再从这里发起最终方向的第二次转向。
        App_Speed_Set(0.0f, 0.0f);
        Task2_StartTurn(task2_turn_final_dir, task2_turn_final_state);
    }else if(task2_state == 4){
        // state 4：每次真实转向后等待一小段时间，减少刚转完就测距/前进的抖动。
        App_Speed_Set(0.0f, 0.0f);
        if(HAL_GetTick() - task2_turn_wait_time >= TASK2_TURN_WAIT_MS){
            task2_state = task2_after_turn_state;
        }
    }else if(task2_state == 21){
        // state 21：BFS 已经给出下一格方向，并且 Task2_StartTurn 已经转到该方向。
        // 这里仍然只走一格；走完后回到 state 20 重新测距、重新规划。
        Set_Position_Angle_SP(Task2_GetStepLen(), Task2_GetYawRef(task2_dir));
        task2_state = 22;
    }else if(task2_state == 22){
        // state 22：等待当前单格运动完成，然后更新软件坐标。
        App_Position_Pro();

        if(motor_pos_done){
            Task2_UpdatePos();
            task2_state = 20;
        }
    }else if(task2_state == 30){
        // state 30：已经到达出口格并转向右侧，直行驶出场地。
        // Task4 只改变出口格，不改变出场方向；四个出口都按右侧驶出处理。
        Set_Position_Angle_SP(80.0f, Task2_GetYawRef(TASK2_EXIT_DIR));
        task2_state = 31;
    }else if(task2_state == 31){
        // state 31：等待驶出完成，然后统一收尾。
        App_Position_Pro();

        if(motor_pos_done){
            Task2_Finish();
        }
    }else {
        Task2_Finish();
    }
}

/*
 * 设置 Task4 出口编号。
 * number=1 -> (3,0)
 * number=2 -> (3,1)，和普通 Task2 的默认出口一致
 * number=3 -> (3,2)
 * number=4 -> (3,3)
 * 其他输入按 2 处理，避免非法数字把目标设到场地外。
 */
void Task4_SetExitByNumber(uint8_t number)
{
    if(number >= 1 && number <= 4){
        task4_exit_number = number;
    }else {
        task4_exit_number = 2;
    }
}

/*
 * Task4 不重新写一套遍历代码，而是复用 Task2：
 *   1. 第一次进入 Task4 时，根据 task4_exit_number 临时改出口。
 *   2. 之后每次主循环都继续调用 Task2()，让原来的外圈扫描、挡板记录、
 *      BFS 找中间四格、去出口、驶出场地流程继续运行。
 *   3. 当 Task2() 内部把 task2_state 置回 0 时，说明复用流程已经结束；
 *      此时 Task4 清 task4_flag、清 task4_active，并把出口恢复成 Task2 默认的 (3,1)。
 */
void Task4(void)
{
    if(task4_active == 0 && task2_state == 0){
        task4_active = 1;
        Task2_SetExit(3, task4_exit_number - 1);
    }

    Task2();

    if(task4_active && task2_state == 0){
        task4_active = 0;
        task4_flag = 0;
        task4_exit_number = 2;
        Task2_ResetExit();
    }
}
