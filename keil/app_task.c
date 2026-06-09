#include <stdint.h>

#include "app_motor.h"
#include "app_speed.h"
#include "app_vl5310x.h"
#include "clock.h"

#define TASK2_GRID_CM 45.0f
#define TASK2_EXIT_X  3
#define TASK2_EXIT_Y  1
#define TASK2_EXIT_DIR 1
#define TASK2_TURN_WAIT_MS 150

extern uint8_t task1_flag;
extern uint8_t task2_flag;

static uint8_t task1_state = 0;

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

static uint8_t Task2_NodeId(uint8_t x, uint8_t y)
{
    return y * 4 + x;
}

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

static uint8_t Task2_OppDir(uint8_t dir)
{
    return (dir + 2) % 4;
}

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

static void Task2_MarkBlock(uint8_t x, uint8_t y, uint8_t dir)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    task2_block[x][y][dir] = 1;

    if(Task2_NextCell(x, y, dir, &nx, &ny)){
        task2_block[nx][ny][Task2_OppDir(dir)] = 1;
    }
}

static uint8_t Task2_CanGo(uint8_t x, uint8_t y, uint8_t dir)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    if(task2_block[x][y][dir]){
        return 0;
    }

    return Task2_NextCell(x, y, dir, &nx, &ny);
}

static void Task2_MarkVisited(uint8_t x, uint8_t y)
{
    task2_visited[x][y] = 1;
}

static uint8_t Task2_IsMiddleCell(uint8_t x, uint8_t y)
{
    return (x >= 1 && x <= 2 && y >= 1 && y <= 2) ? 1 : 0;
}

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

static uint8_t Task2_IsObstacle(VL5310X_SensorId_t id)
{
    uint16_t distance = App_VL5310X_GetDistance(id);

    if(distance > VL5310X_OBSTACLE_MIN_MM && distance < VL5310X_OBSTACLE_MAX_MM){
        return 1;
    }

    return 0;
}

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

static float Task2_GetStepLen(void)
{
    return TASK2_GRID_CM;
}

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
        // 初始化地图，然后从 A 口进入左侧第三行的格子中心。
        // 进场距离不是标准一格，所以仍然单独写 80cm。
        Task2_InitMap();
        App_Update_Data();
        task2_yaw_base = angle_yaw;
        Set_Position_Angle_SP(80.0f, Task2_GetYawRef(task2_dir));
        task2_state = 1;
    }else if(task2_state == 1){
        // 进场完成后，先转到外圈扫描路线的第一个方向。
        App_Position_Pro();

        if(motor_pos_done){
            Task2_MarkVisited(task2_x, task2_y);
            Task2_StartTurn(task2_outer_dir[task2_outer_step], 10);
        }
    }else if(task2_state == 10){
        // 外圈绕行时，中间区域一直在车左侧。
        // 每到格子中心只用左侧测距，连续 3 次命中才记录挡板。
        if(Task2_CheckWallDone(1) == 0){
            return;
        }

        Set_Position_Angle_SP(Task2_GetStepLen(), Task2_GetYawRef(task2_dir));
        task2_state = 11;
    }else if(task2_state == 11){
        // 外圈每次只走一格，到下一个格子中心后停下来再测距。
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
        // Middle cells done: go to C directly, do not add new wall data here.
        // This avoids a false reading at the last middle cell changing the exit path.
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

        // Middle search: stop at cell center and use front/left/right sensors.
        if(Task2_CheckWallDone(0) == 0){
            return;
        }

        if(Task2_BuildPath(1, 0, 0)){
            Task2_StartTurn(task2_path[0], 21);
        }else {
            Task2_Finish();
        }
    }else if(task2_state == 2){
        // 公共转向状态。转到目标方向后，再进入下一步。
        App_Angle_Pro();

        if(motor_angle_done){
            task2_dir = task2_target_dir;
            task2_turn_wait_time = HAL_GetTick();
            task2_state = 4;
        }
    }else if(task2_state == 3){
        // 180 度转向的第二个 90 度。
        App_Speed_Set(0.0f, 0.0f);
        Task2_StartTurn(task2_turn_final_dir, task2_turn_final_state);
    }else if(task2_state == 4){
        // Wait a short time after every real turn, then continue.
        App_Speed_Set(0.0f, 0.0f);
        if(HAL_GetTick() - task2_turn_wait_time >= TASK2_TURN_WAIT_MS){
            task2_state = task2_after_turn_state;
        }
    }else if(task2_state == 21){
        // BFS 决定下一格方向后，仍然只走一格。
        // 单格距离统一由 TASK2_GRID_CM 控制，方便后面直接调参。
        Set_Position_Angle_SP(Task2_GetStepLen(), Task2_GetYawRef(task2_dir));
        task2_state = 22;
    }else if(task2_state == 22){
        App_Position_Pro();

        if(motor_pos_done){
            Task2_UpdatePos();
            task2_state = 20;
        }
    }else if(task2_state == 30){
        // Exit from C.
        Set_Position_Angle_SP(80.0f, Task2_GetYawRef(TASK2_EXIT_DIR));
        task2_state = 31;
    }else if(task2_state == 31){
        App_Position_Pro();

        if(motor_pos_done){
            Task2_Finish();
        }
    }else {
        Task2_Finish();
    }
}
