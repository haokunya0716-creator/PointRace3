	#include <stdint.h>

#include "app_motor.h"
#include "app_speed.h"
#include "app_vl5310x.h"
#include "app_task4.h"
#include "clock.h"
#include "app_PWM.h"
#include "vision.h"

#define TASK4_GRID_CM 45.0f
#define TASK4_EXIT_DIR 1
#define TASK4_TURN_WAIT_MS 150


extern uint8_t task3_set_exit_flag;
extern uint8_t task4_set_exit_flag;
extern uint8_t task3_flag;
extern uint8_t task4_flag;

volatile uint8_t drop_flag = 0;

static uint8_t task4_exit_x = 3;
static uint8_t task4_exit_y = 1;

static uint8_t task4_state = 0;
static uint8_t task4_x = 0;
static uint8_t task4_y = 2;
static uint8_t task4_dir = 1;
static uint8_t task4_target_dir = 1;
static uint8_t task4_after_turn_state = 0;
static uint8_t task4_turn_final_dir = 1;
static uint8_t task4_turn_final_state = 0;
static uint8_t task4_visited[4][4] = {0};
static uint8_t task4_block[4][4][4] = {0};
static uint8_t task4_path[16] = {0};
static uint8_t task4_path_len = 0;
static uint8_t task4_check_count = 0;
static uint8_t task4_front_count = 0;
static uint8_t task4_left_count = 0;
static uint8_t task4_right_count = 0;
static uint8_t task4_outer_step = 0;
static unsigned long task4_check_time = 0;
static unsigned long task4_turn_wait_time = 0;
static float task4_yaw_base = 0.0f;

static void Drop_Flag_Updata(void){

	if( vision.data == 0x01 ){drop_flag = 1; vision.data = 0;}
	else if(vision.data == 0x02){drop_flag = 2; vision.data = 0;}
	else if(vision.data == 0x03){drop_flag = 3; vision.data = 0;}
}

// 从 A 口进入 (0,2) 后，按这个方向序列绕外圈一圈并回到 (0,2)。
static const uint8_t task4_outer_dir[12] = {
    2, 1, 1, 1, 0, 0, 0, 3, 3, 3, 2, 2
};

static uint8_t Task4_NodeId(uint8_t x, uint8_t y)
{
    return y * 4 + x;
}

static uint8_t Task4_NextCell(uint8_t x, uint8_t y, uint8_t dir, uint8_t *nx, uint8_t *ny)
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

static uint8_t Task4_OppDir(uint8_t dir)
{
    return (dir + 2) % 4;
}

static float Task4_GetYawRef(uint8_t dir)
{
    float angle = ((float)dir - 1.0f) * 90.0f;

    while(angle > 180.0f){
        angle -= 360.0f;
    }

    while(angle < -180.0f){
        angle += 360.0f;
    }

    return task4_yaw_base + angle;
}

static void Task4_MarkBlock(uint8_t x, uint8_t y, uint8_t dir)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    task4_block[x][y][dir] = 1;

    if(Task4_NextCell(x, y, dir, &nx, &ny)){
        task4_block[nx][ny][Task4_OppDir(dir)] = 1;
    }
}

// 投放完成后，把这一格的四个方向都标记为不可通行，避免小车再次经过被卡住。
static void Task4_BlockCell(uint8_t x, uint8_t y)
{
    uint8_t dir = 0;

    for(dir = 0; dir < 4; dir++){
        Task4_MarkBlock(x, y, dir);
    }
}

static uint8_t Task4_CanGo(uint8_t x, uint8_t y, uint8_t dir)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    if(task4_block[x][y][dir]){
        return 0;
    }

    return Task4_NextCell(x, y, dir, &nx, &ny);
}

static void Task4_MarkVisited(uint8_t x, uint8_t y)
{
    task4_visited[x][y] = 1;
}

static uint8_t Task4_IsMiddleCell(uint8_t x, uint8_t y)
{
    return (x >= 1 && x <= 2 && y >= 1 && y <= 2) ? 1 : 0;
}

static uint8_t Task4_MiddleDone(void)
{
    uint8_t x = 0;
    uint8_t y = 0;

    for(x = 1; x <= 2; x++){
        for(y = 1; y <= 2; y++){
            if(task4_visited[x][y] == 0){
                return 0;
            }
        }
    }

    return 1;
}

static uint8_t Task4_IsBoundaryDir(uint8_t x, uint8_t y, uint8_t dir)
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

static uint8_t Task4_IsObstacle(VL5310X_SensorId_t id)
{
    uint16_t distance = App_VL5310X_GetDistance(id);

    if(distance > VL5310X_OBSTACLE_MIN_MM && distance < VL5310X_OBSTACLE_MAX_MM){
        return 1;
    }

    return 0;
}

static void Task4_MarkSensorWall(uint8_t dir, uint8_t only_middle)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    if(Task4_IsBoundaryDir(task4_x, task4_y, dir)){
        return;
    }

    if(Task4_NextCell(task4_x, task4_y, dir, &nx, &ny) == 0){
        return;
    }

    // 外圈扫描时只记录朝向中间四格的边，避免把外圈普通边误记成挡板。
    if(only_middle && Task4_IsMiddleCell(nx, ny) == 0){
        return;
    }

    Task4_MarkBlock(task4_x, task4_y, dir);
}

static uint8_t Task4_CheckWallDone(uint8_t only_left)
{
    uint8_t front_dir = task4_dir;
    uint8_t left_dir = (task4_dir + 3) % 4;
    uint8_t right_dir = (task4_dir + 1) % 4;

    if(task4_check_count == 0){
        task4_front_count = 0;
        task4_left_count = 0;
        task4_right_count = 0;
    }

    if(task4_check_count == 0 || HAL_GetTick() - task4_check_time >= 50){
        task4_check_time = HAL_GetTick();
        App_VL5310X_Proc();

        // 每次停在格子中心时采 3 次，3 次都在障碍距离范围内才算挡板。
        if(only_left == 0 && Task4_IsObstacle(VL5310X_FRONT)){
            task4_front_count++;
        }

        if(Task4_IsObstacle(VL5310X_LEFT)){
            task4_left_count++;
        }

        if(only_left == 0 && Task4_IsObstacle(VL5310X_RIGHT)){
            task4_right_count++;
        }

        task4_check_count++;
    }

    if(task4_check_count >= 3){
        if(only_left == 0 && task4_front_count >= 3){
            Task4_MarkSensorWall(front_dir, 0);
        }

        if(task4_left_count >= 3){
            Task4_MarkSensorWall(left_dir, only_left);
        }

        if(only_left == 0 && task4_right_count >= 3){
            Task4_MarkSensorWall(right_dir, 0);
        }

        task4_check_count = 0;
        return 1;
    }

    return 0;
}

static float Task4_GetStepLen(void)
{
    return TASK4_GRID_CM;
}

// 根据按键号设置出口：1->(3,0)，2->(3,1)（默认出口），3->(3,2)，4->(3,3)。
static void Task4_SetExit(uint8_t exit)
{
    switch(exit){
        case 1:
            task4_exit_x = 3;
            task4_exit_y = 0;
            break;
        case 3:
            task4_exit_x = 3;
            task4_exit_y = 2;
            break;
        case 4:
            task4_exit_x = 3;
            task4_exit_y = 3;
            break;
        case 2:
        default:
            task4_exit_x = 3;
            task4_exit_y = 1;
            break;
    }
}

// 1 号舵机投放动作，具体实现后续补充。
static void Task4_Drop1(void)
{
	  App_PWM_SetAngle(APP_PWM_SERVO1, 0.0f);
	  drop_flag = 0;
} 

// 2 号舵机投放动作，具体实现后续补充。
static void Task4_Drop2(void)
{
	 App_PWM_SetAngle(APP_PWM_SERVO2, 0.0f);
	 drop_flag = 0;
}

// 3 号舵机投放动作，具体实现后续补充。
static void Task4_Drop3(void)
{
	 App_PWM_SetAngle(APP_PWM_SERVO3, 0.0f);
	 drop_flag = 0;
}


// 在格子中心检测投放标志位：1/2/3 分别投放对应舵机，
// 投放后把当前格子标记为不可跨越，避免小车再次经过被卡住。
static void Task4_ProcessDrop(void)
{
    if(drop_flag == 0){
        return;
    }

    if(drop_flag == 1){
        Task4_Drop1();
    }else if(drop_flag == 2){
        Task4_Drop2();
    }else if(drop_flag == 3){
        Task4_Drop3();
    }

    Task4_BlockCell(task4_x, task4_y);
    drop_flag = 0;
}

static void Task4_InitMap(void)
{
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t d = 0;

    for(x = 0; x < 4; x++){
        for(y = 0; y < 4; y++){
            task4_visited[x][y] = 0;

            for(d = 0; d < 4; d++){
                task4_block[x][y][d] = 0;
            }
        }
    }

    // 四周边墙不能走，先当作已知挡板记录。
    for(x = 0; x < 4; x++){
        Task4_MarkBlock(x, 0, 0);
        Task4_MarkBlock(x, 3, 2);
    }

    for(y = 0; y < 4; y++){
        Task4_MarkBlock(0, y, 3);
        Task4_MarkBlock(3, y, 1);
    }

    task4_x = 0;
    task4_y = 2;
    task4_dir = 1;
    task4_target_dir = 1;
    task4_turn_final_dir = 1;
    task4_turn_final_state = 0;
    task4_path_len = 0;
    task4_check_count = 0;
    task4_front_count = 0;
    task4_left_count = 0;
    task4_right_count = 0;
    task4_outer_step = 0;
    task4_check_time = 0;
    task4_turn_wait_time = 0;
}

static uint8_t Task4_BuildPath(uint8_t find_unvisited, uint8_t target_x, uint8_t target_y)
{
    uint8_t que[16] = {0};
    uint8_t used[16] = {0};
    uint8_t prev[16] = {0};
    uint8_t prev_dir[16] = {0};
    uint8_t head = 0;
    uint8_t tail = 0;
    uint8_t found = 0xff;
    uint8_t start = Task4_NodeId(task4_x, task4_y);
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

        if(find_unvisited && Task4_IsMiddleCell(x, y) && task4_visited[x][y] == 0){
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

            if(Task4_CanGo(x, y, dir) == 0){
                continue;
            }

            Task4_NextCell(x, y, dir, &nx, &ny);
            nid = Task4_NodeId(nx, ny);

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
        task4_path_len = 0;
        return 0;
    }

    task4_path_len = 0;

    while(found != start){
        task4_path[task4_path_len++] = prev_dir[found];
        found = prev[found];
    }

    // BFS 是从终点往回找的，这里反过来，path[0] 才是下一步方向。
    for(i = 0; i < task4_path_len / 2; i++){
        uint8_t t = task4_path[i];
        task4_path[i] = task4_path[task4_path_len - 1 - i];
        task4_path[task4_path_len - 1 - i] = t;
    }

    return 1;
}

static void Task4_StartTurn(uint8_t dir, uint8_t next_state)
{
    uint8_t diff = (dir + 4 - task4_dir) % 4;
    uint8_t mid_dir = 0;
    float angle = 0.0f;

    task4_target_dir = dir;
    task4_after_turn_state = next_state;

    if(diff == 0){
        task4_state = task4_after_turn_state;
        return;
    }

    // 180 度直接转容易超调，所以拆成两次 90 度。
    if(diff == 2){
        task4_turn_final_dir = dir;
        task4_turn_final_state = next_state;
        mid_dir = (task4_dir + 1) % 4;
        task4_target_dir = mid_dir;
        task4_after_turn_state = 3;
        dir = mid_dir;
    }

    App_Update_Data();
    angle = Task4_GetYawRef(dir) - angle_yaw;

    while(angle > 180.0f){
        angle -= 360.0f;
    }

    while(angle < -180.0f){
        angle += 360.0f;
    }

    Set_Angle_SP(angle);
    task4_state = 2;
}

static void Task4_UpdatePos(void)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    if(Task4_NextCell(task4_x, task4_y, task4_dir, &nx, &ny)){
        task4_x = nx;
        task4_y = ny;
        Task4_MarkVisited(task4_x, task4_y);
    }
}

static void Task4_Finish(void)
{
    App_Speed_Set(0.0f, 0.0f);
    task4_state = 0;
    task3_flag = 0;
    task4_flag = 0;
	  App_PWM_SetAngle(APP_PWM_SERVO1, 180.0f);
    App_PWM_SetAngle(APP_PWM_SERVO2, 110.0f);
    App_PWM_SetAngle(APP_PWM_SERVO3, 125.0f);
	  vision.cmd = 0;
	  vision.data = 0;
	task3_set_exit_flag = 0;
	task4_set_exit_flag = 0;
}

void Task4(uint8_t exit)
{
	  Drop_Flag_Updata();
    if(task4_state == 0){
        // 初始化地图、根据按键号设置出口，然后从 A 口进入左侧第三行的格子中心。
        // 进场距离不是标准一格，所以仍然单独写 80cm。
        Task4_InitMap();
        Task4_SetExit(exit);
        App_Update_Data();
        task4_yaw_base = angle_yaw;
        Set_Position_Angle_SP(80.0f, Task4_GetYawRef(task4_dir));
        task4_state = 1;
    }else if(task4_state == 1){
        // 进场完成后，先转到外圈扫描路线的第一个方向。
        App_Position_Pro();

        if(motor_pos_done){
            Task4_MarkVisited(task4_x, task4_y);
            Task4_StartTurn(task4_outer_dir[task4_outer_step], 10);
        }
    }else if(task4_state == 10){
        // 外圈绕行时，中间区域一直在车左侧。
        // 每到格子中心只用左侧测距，连续 3 次命中才记录挡板。
        if(Task4_CheckWallDone(1) == 0){
            return;
        }

        // 到达格子中心，检测投放标志位。
        Task4_ProcessDrop();

        Set_Position_Angle_SP(Task4_GetStepLen(), Task4_GetYawRef(task4_dir));
        task4_state = 11;
    }else if(task4_state == 11){
        // 外圈每次只走一格，到下一个格子中心后停下来再测距。
        App_Position_Pro();

        if(motor_pos_done){
            Task4_UpdatePos();
            task4_outer_step++;

            if(task4_outer_step >= 12){
                task4_state = 20;
            }else {
                Task4_StartTurn(task4_outer_dir[task4_outer_step], 10);
            }
        }
    }else if(task4_state == 20){
        // 中间四格遍历完成后，直接前往出口，不再追加新的挡板信息。
        // 这样可以避免最后一格的误判数据改变出口路径。
        if(Task4_MiddleDone()){
            if(task4_x == task4_exit_x && task4_y == task4_exit_y){
                Task4_StartTurn(TASK4_EXIT_DIR, 30);
            }else if(Task4_BuildPath(0, task4_exit_x, task4_exit_y)){
                Task4_StartTurn(task4_path[0], 21);
            }else {
                Task4_Finish();
            }
            return;
        }

        // 中间区域搜索：停在格子中心，使用前/左/右三个传感器。
        if(Task4_CheckWallDone(0) == 0){
            return;
        }

        // 到达格子中心，检测投放标志位。
        Task4_ProcessDrop();

        if(Task4_BuildPath(1, 0, 0)){
            Task4_StartTurn(task4_path[0], 21);
        }else {
            Task4_Finish();
        }
    }else if(task4_state == 2){
        // 公共转向状态。转到目标方向后，再进入下一步。
        App_Angle_Pro();

        if(motor_angle_done){
            task4_dir = task4_target_dir;
            task4_turn_wait_time = HAL_GetTick();
            task4_state = 4;
        }
    }else if(task4_state == 3){
        // 180 度转向的第二个 90 度。
        App_Speed_Set(0.0f, 0.0f);
        Task4_StartTurn(task4_turn_final_dir, task4_turn_final_state);
    }else if(task4_state == 4){
        // 每次真正转向后都先停一小段时间，再继续。
        App_Speed_Set(0.0f, 0.0f);
        if(HAL_GetTick() - task4_turn_wait_time >= TASK4_TURN_WAIT_MS){
            task4_state = task4_after_turn_state;
        }
    }else if(task4_state == 21){
        // BFS 决定下一格方向后，仍然只走一格。
        // 单格距离统一由 TASK4_GRID_CM 控制，方便后面直接调参。
        Set_Position_Angle_SP(Task4_GetStepLen(), Task4_GetYawRef(task4_dir));
        task4_state = 22;
    }else if(task4_state == 22){
        App_Position_Pro();

        if(motor_pos_done){
            Task4_UpdatePos();
            task4_state = 20;
        }
    }else if(task4_state == 30){
        // 从出口驶出。
        Set_Position_Angle_SP(80.0f, Task4_GetYawRef(TASK4_EXIT_DIR));
        task4_state = 31;
    }else if(task4_state == 31){
        App_Position_Pro();

        if(motor_pos_done){
            Task4_Finish();
        }
    }else {
        Task4_Finish();
    }
}
