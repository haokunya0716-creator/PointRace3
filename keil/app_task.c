#include <stdint.h>

#include "app_motor.h"
#include "app_vl5310x.h"
#include "clock.h"

/* task1_flag 在按键任务里置 1。这里用 extern 引进来，用来判断任务是否还在执行。 */
extern uint8_t task1_flag;
extern uint8_t task2_flag;

/*
 * Task1 的动作流程：
 * 1. 位置闭环前进 78cm。
 * 2. 左轮速度设为 0r/s，右轮速度设为 0.5r/s，靠时间控制左转角度。
 * 3. 左右轮都设为 1r/s，再直行一段时间。
 * 4. 任务结束后停车，并把 task1_flag 清 0。
 */
typedef enum {
    TASK1_IDLE = 0,      /* 空闲状态：任务还没开始，或者已经结束。 */
    TASK1_FORWARD_78,    /* 第一段：用位置闭环前进 78cm。 */
    TASK1_LEFT_TURN,     /* 第二段：左轮停、右轮转，实现左转。 */
    TASK1_STRAIGHT_2     /* 第三段：速度闭环继续直行。 */
} Task1_State_t;

/* 当前 Task1 执行到哪一个状态。static 表示这个变量只给本文件使用。 */
static Task1_State_t task1_state = TASK1_IDLE;

/* 记录进入当前状态的时间，用来计算当前状态已经执行了多久。 */
static unsigned long task1_start_time = 0;

/* 记录上一次发送速度命令的时间，避免每次 while 循环都疯狂发串口命令。 */
static unsigned long task1_speed_tx_time = 0;

/* 进入 78cm 位置闭环前，先记住左右轮当前的位置，后面用它判断走了多少 cm。 */
static float task1_start_position_l = 0.0f;
static float task1_start_position_r = 0.0f;

/*
 * 按固定周期给电机发送一次速度。
 *
 * left_rps  ：左轮目标速度，单位 r/s。
 * right_rps ：右轮目标速度，单位 r/s。
 *
 * 这里的 30UL 表示 30ms 发一次速度命令。不要每次主循环都发，否则电机串口容易被塞满。
 */
static void Task1_SendSpeed(float left_rps, float right_rps)
{
    unsigned long now = 0;

    mspm0_get_clock_ms(&now);
    if ((now - task1_speed_tx_time) >= 30UL)
    {
        task1_speed_tx_time = now;
        App_Set_Speed(left_rps, right_rps);
    }
}

/*
 * 进入一个新的状态时，只做“刚切进这个状态那一瞬间”需要做的事情。
 * 例如：
 * - 进入位置闭环状态时，记录起点位置，并设置 78cm 的位置目标。
 * - 进入速度状态时，立即发一次速度命令。
 */
static void Task1_Enter(Task1_State_t next_state)
{
    task1_state = next_state;
    task1_speed_tx_time = 0;
    mspm0_get_clock_ms(&task1_start_time);

    switch (task1_state)
    {
        case TASK1_FORWARD_78:
            /*
             * 第一段使用位置闭环。
             *
             * App_Update_Data()：先把最新编码器值换算成 position_l / position_r。
             * App_PID_Reset()  ：清掉位置 PID 上一次留下的积分、误差等内部状态。
             * task1_start_position_l/r：记录当前位置，后面用当前位置减起点判断实际走了多少。
             */
            App_Update_Data();
            App_PID_Reset();
            task1_start_position_l = position_l;
            task1_start_position_r = position_r;

            /*
             * 设置位置闭环目标。
             *
             * 78.0f 是第一段要走的距离，单位 cm。
             * 如果想把第一段改成 80cm，就只改这里两个 78.0f。
             */
            Set_Position_SP_L(78.0f);
            Set_Position_SP_R(78.0f);
            break;

        case TASK1_LEFT_TURN:
            /*
             * 左转段使用速度控制。
             *
             * 左轮 0.0r/s，右轮 0.5r/s。
             * 左轮不动，右轮往前走，车就会向左转。
             */
            App_Set_Speed(0.0f, 0.5f);
            break;

        case TASK1_STRAIGHT_2:
            /*
             * 最后一段继续直行。
             *
             * 左右轮都给 1.0r/s，所以车向前直行。
             */
            App_Set_Speed(1.0f, 1.0f);
            break;

        default:
            /* 异常状态保护：如果状态不对，就先停车。 */
            App_Set_Speed(0.0f, 0.0f);
            break;
    }
}

/*
 * Task1() 不是 while 死循环任务。
 *
 * 它每次被 main 的 while(1) 调用时，只执行当前状态的一小步：
 * - 如果正在走 78cm，就跑一次位置闭环并判断是否到位。
 * - 如果正在转弯，就补发速度并判断转弯时间够不够。
 * - 如果正在最后直行，就补发速度并判断直行时间够不够。
 *
 * 这样写的好处是不会卡死 main 循环，按键、测距、灰度、LED 等周期任务还能继续运行。
 */
void Task1(void)
{
    unsigned long now = 0;
    unsigned long dt = 0;

    /*
     * 第一次进入 Task1 时，状态还是 IDLE。
     * 这里把状态切到第一段：位置闭环前进 78cm。
     */
    if (task1_state == TASK1_IDLE)
    {
        Task1_Enter(TASK1_FORWARD_78);
        return;
    }

    /*
     * dt 是当前状态已经执行的时间，单位 ms。
     * 每次切状态时，Task1_Enter() 都会重新记录 task1_start_time。
     */
    mspm0_get_clock_ms(&now);
    dt = now - task1_start_time;

    switch (task1_state)
    {
        case TASK1_FORWARD_78:
        {
            float left_distance = 0.0f;
            float right_distance = 0.0f;

            /*
             * 先更新编码器位置，再计算当前状态已经走了多少距离。
             *
             * 左轮编码器方向和右轮相反，所以：
             * - 左轮走过距离 = 起点位置 - 当前左轮位置
             * - 右轮走过距离 = 当前右轮位置 - 起点位置
             */
            App_Update_Data();
            left_distance = task1_start_position_l - position_l;
            right_distance = position_r - task1_start_position_r;

            /*
             * 真正执行位置闭环。
             *
             * App_Position_Pro() 内部会根据 Set_Position_SP_L/R() 设置的目标，
             * 算出左右轮速度，并通过 Motor_Set_Speeds() 发给电机。
             */
            App_Position_Pro();

            /*
             * 第一段结束条件：
             * 1. 左右轮都走到 77cm 以上，就认为 78cm 到位。
             *    这里 1.0f 是允许误差，也就是 78cm - 1cm = 77cm。
             * 2. 如果 6000ms 还没到位，也强制进入下一段，防止程序一直卡住。
             */
            if (((left_distance >= (78.0f - 1.0f)) &&
                 (right_distance >= (78.0f - 1.0f))) ||
                (dt >= 6000UL))
            {
                App_Set_Speed(0.0f, 0.0f);
                Task1_Enter(TASK1_LEFT_TURN);
            }
            break;
        }

        case TASK1_LEFT_TURN:
            /*
             * 左转段：
             * - 左轮 0.0r/s。
             * - 右轮 0.5r/s。
             * - 1500UL 表示左转持续 1500ms。
             *
             * 想让角度更大，就把 1500UL 调大。
             * 想让角度更小，就把 1500UL 调小。
             */
            Task1_SendSpeed(0.0f, 0.5f);
            if (dt >= 1500UL)
            {
                Task1_Enter(TASK1_STRAIGHT_2);
            }
            break;

        case TASK1_STRAIGHT_2:
            /*
             * 最后一段直行：
             * - 左轮 1.0r/s。
             * - 右轮 1.0r/s。
             * - 3000UL 表示直行持续 3000ms。
             */
            Task1_SendSpeed(1.0f, 1.0f);
            if (dt >= 3000UL)
            {
                /*
                 * 任务完成：
                 * - 停车。
                 * - 清掉 task1_flag，让 main 不再调用 Task1()。
                 * - 状态回到 IDLE，方便下一次按键后重新执行。
                 */
                App_Set_Speed(0.0f, 0.0f);
                task1_flag = 0;
                task1_state = TASK1_IDLE;
                task1_speed_tx_time = 0;
            }
            break;

        default:
            /*
             * 理论上不会进入 default。
             * 如果真的进来了，说明状态变量异常，直接停车并结束任务。
             */
            App_Set_Speed(0.0f, 0.0f);
            task1_flag = 0;
            task1_state = TASK1_IDLE;
            task1_speed_tx_time = 0;
            break;
    }
}

/*
 * ==============================
 * 第二问：4x4 全遍历 + 前方测距绕障
 * ==============================
 *
 * 场地图按 4x4 方格处理：
 *
 *   x 从左到右：0,1,2,3
 *   y 从上到下：0,1,2,3
 *
 * 根据题目图 1：
 * - 小车从左侧 A 口进入，进入后认为位于 (0,2)，车头朝右。
 * - C 口在右侧中上位置，最终从 (3,1) 朝右驶出。
 * - 两块随机挡板放在网格线上，刚好相当于“两个相邻格子之间的边不能走”。
 *
 * 这份 Task2 不用位置闭环，只用速度 + 时间：
 * - 前进：左右轮都 1.0r/s。
 * - 后退：左右轮都 -1.0r/s。
 * - 左转：左轮 -1.0r/s，右轮 1.0r/s。
 * - 右转：左轮 1.0r/s，右轮 -1.0r/s。
 *
 * 所有时间都直接写在对应判断处，后续实车调参时直接改这些数字。
 */
typedef enum {
    TASK2_IDLE = 0,        /* 空闲：任务未启动。 */
    TASK2_ENTER,           /* 从准备区经 A 口进入第一个格子中心。 */
    TASK2_PLAN,            /* 根据已知挡板，规划到最近未访问格子或出口。 */
    TASK2_STEP,            /* 取出规划路径里的下一步。 */
    TASK2_TURN,            /* 用 1r/s 原地转向到下一步方向。 */
    TASK2_CHECK_FRONT,     /* 在格子中心用前方激光判断前方边是否有挡板。 */
    TASK2_GO,              /* 前方无障碍，按 1r/s 前进一格。 */
    TASK2_BACK,            /* 安全兜底：如果行进中误遇障碍，后退一小段。 */
    TASK2_EXIT_TURN,       /* 16 格遍历完成后，转向 C 口方向。 */
    TASK2_EXIT_GO          /* 从 C 口驶出场地。 */
} Task2_State_t;

typedef enum {
    TASK2_UP = 0,          /* 车头朝上，下一格 y - 1。 */
    TASK2_RIGHT,           /* 车头朝右，下一格 x + 1。 */
    TASK2_DOWN,            /* 车头朝下，下一格 y + 1。 */
    TASK2_LEFT             /* 车头朝左，下一格 x - 1。 */
} Task2_Dir_t;

static Task2_State_t task2_state = TASK2_IDLE;

/* 当前所在格子的坐标。启动后从 A 口进入，默认进入 (0,2)。 */
static uint8_t task2_x = 0;
static uint8_t task2_y = 2;

/* 当前车头方向。A 口在左侧，进入场地时车头朝右。 */
static uint8_t task2_dir = TASK2_RIGHT;

/* 已经经过的格子记录。visited[x][y] = 1 表示这个格子已经遍历过。 */
static uint8_t task2_visited[4][4];
static uint8_t task2_visited_num = 0;

/*
 * 挡板记录。
 *
 * task2_block[x][y][dir] = 1 表示从 (x,y) 朝 dir 方向不能走。
 * 例如 task2_block[1][2][TASK2_RIGHT] = 1：
 * 表示 (1,2) 和 (2,2) 中间有挡板，或者是场地边墙。
 */
static uint8_t task2_block[4][4][4];

/*
 * BFS 规划出来的方向序列。
 *
 * task2_plan[0] 是下一步该走的方向；
 * task2_plan_len 是路径总步数；
 * task2_plan_i 是当前执行到第几步。
 */
static uint8_t task2_plan[16];
static uint8_t task2_plan_len = 0;
static uint8_t task2_plan_i = 0;

/* 本次准备走向哪个方向。先转到这个方向，再测距，再前进。 */
static uint8_t task2_target_dir = TASK2_RIGHT;

/* 状态计时：每进入一个新状态都会刷新，用来做定时前进、定时转弯。 */
static unsigned long task2_start_time = 0;
static unsigned long task2_speed_tx_time = 0;

/* 180 度转弯会拆成两次 90 度，所以这里记录还剩几次 90 度要转。 */
static uint8_t task2_turn_steps_left = 0;
static float task2_turn_speed_l = 0.0f;
static float task2_turn_speed_r = 0.0f;
static Task2_State_t task2_after_turn_state = TASK2_CHECK_FRONT;

/*
 * 简单状态切换函数。
 *
 * 每次切换状态时：
 * - 记录进入状态的时间；
 * - 清空速度发送计时；
 * 后面各状态就可以用 dt = now - task2_start_time 判断执行了多久。
 */
static void Task2_SetState(Task2_State_t next_state)
{
    task2_state = next_state;
    task2_speed_tx_time = 0;
    mspm0_get_clock_ms(&task2_start_time);
}

/*
 * 速度命令不要每次 while(1) 都发。
 *
 * 这里每 30ms 补发一次 App_Set_Speed()，既能保持速度命令持续有效，
 * 又不会把电机串口塞爆。
 */
static void Task2_SendSpeed(float left_rps, float right_rps)
{
    unsigned long now = 0;

    mspm0_get_clock_ms(&now);
    if ((now - task2_speed_tx_time) >= 30UL)
    {
        task2_speed_tx_time = now;
        App_Set_Speed(left_rps, right_rps);
    }
}

/* 第二问所有速度动作都从这里停车，避免不同状态切换时残留上一段速度。 */
static void Task2_Stop(void)
{
    App_Set_Speed(0.0f, 0.0f);
    task2_speed_tx_time = 0;
}

/* 把二维格子坐标压成 0~15 的编号，方便 BFS 用数组保存前驱节点。 */
static uint8_t Task2_NodeId(uint8_t x, uint8_t y)
{
    return (uint8_t)(y * 4U + x);
}

/* 判断坐标是否还在 4x4 场地内部。 */
static uint8_t Task2_CellOk(int8_t x, int8_t y)
{
    return (x >= 0 && x < 4 && y >= 0 && y < 4) ? 1U : 0U;
}

/*
 * 根据当前格子和方向，计算下一格坐标。
 *
 * 返回 1：下一格合法；
 * 返回 0：这个方向会撞到外墙，不能走。
 */
static uint8_t Task2_NextCell(uint8_t x, uint8_t y, uint8_t dir, uint8_t *nx, uint8_t *ny)
{
    int8_t tx = (int8_t)x;
    int8_t ty = (int8_t)y;

    if (dir == TASK2_UP)
        ty--;
    else if (dir == TASK2_RIGHT)
        tx++;
    else if (dir == TASK2_DOWN)
        ty++;
    else
        tx--;

    if (!Task2_CellOk(tx, ty))
        return 0U;

    *nx = (uint8_t)tx;
    *ny = (uint8_t)ty;
    return 1U;
}

/* 当前方向的反方向。右的反方向是左，上的反方向是下。 */
static uint8_t Task2_OppDir(uint8_t dir)
{
    return (uint8_t)((dir + 2U) & 0x03U);
}

/*
 * 标记某条边不能走。
 *
 * 如果从 (1,2) 朝右被挡，那么不仅 (1,2) 的右边不能走，
 * 相邻格 (2,2) 的左边也不能走。这样 BFS 从两边都不会穿过挡板。
 */
static void Task2_MarkBlock(uint8_t x, uint8_t y, uint8_t dir)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    task2_block[x][y][dir] = 1U;

    if (Task2_NextCell(x, y, dir, &nx, &ny))
        task2_block[nx][ny][Task2_OppDir(dir)] = 1U;
}

/*
 * 判断某格朝某方向能不能走。
 *
 * 外墙和已知挡板都会返回 0；普通相邻格返回 1。
 */
static uint8_t Task2_CanGo(uint8_t x, uint8_t y, uint8_t dir)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    if (task2_block[x][y][dir])
        return 0U;

    return Task2_NextCell(x, y, dir, &nx, &ny);
}

/* 初始化外墙。4x4 最外圈边界都不能越界，统一当作 blocked 边。 */
static void Task2_InitWall(void)
{
    uint8_t x = 0;
    uint8_t y = 0;

    for (x = 0; x < 4; x++)
    {
        Task2_MarkBlock(x, 0, TASK2_UP);
        Task2_MarkBlock(x, 3, TASK2_DOWN);
    }

    for (y = 0; y < 4; y++)
    {
        Task2_MarkBlock(0, y, TASK2_LEFT);
        Task2_MarkBlock(3, y, TASK2_RIGHT);
    }
}

/* 每到达一个格子中心，就标记一次已遍历。重复经过同一格不会重复计数。 */
static void Task2_MarkVisited(uint8_t x, uint8_t y)
{
    if (!task2_visited[x][y])
    {
        task2_visited[x][y] = 1U;
        task2_visited_num++;
    }
}

/* 第二问要求 16 个格子都遍历，计数到 16 后就可以去 C 口。 */
static uint8_t Task2_AllVisited(void)
{
    return (task2_visited_num >= 16U) ? 1U : 0U;
}

/*
 * 在格子中心探测前方是否有挡板。
 *
 * 280U 的单位是 mm。
 * 因为格子边长 45cm，小车在格子中心时，前方网格线大概在 22.5cm 前方；
 * 传感器装在车头上会更靠近挡板，所以阈值需要实车调。
 *
 * 如果误判太敏感：把 280U 调小；
 * 如果挡板检测不到：把 280U 调大。
 */
static uint8_t Task2_FrontBlocked(void)
{
    uint16_t d = App_VL5310X_GetDistance(VL5310X_FRONT);

    if (vl5310x_front_obstacle_flag)
        return 1U;

    return (d > 0U && d <= 280U) ? 1U : 0U;
}

/*
 * BFS 规划路径。
 *
 * dst_x/dst_y >= 0：规划到指定格子，例如最后去 C 口所在格 (3,1)。
 * dst_x < 0      ：规划到最近的未访问格子，用于 16 格遍历阶段。
 *
 * BFS 只根据“已经知道”的挡板规划。
 * 未知挡板会在实际准备走某条边时，用前方激光发现，然后 MarkBlock 再重规划。
 */
static uint8_t Task2_BuildPath(int8_t dst_x, int8_t dst_y)
{
    uint8_t que[16];
    uint8_t used[16] = {0};
    uint8_t prev[16];
    uint8_t prev_dir[16];
    uint8_t head = 0;
    uint8_t tail = 0;
    uint8_t found = 0xFFU;
    uint8_t start = Task2_NodeId(task2_x, task2_y);
    uint8_t i = 0;

    for (i = 0; i < 16; i++)
    {
        prev[i] = 0xFFU;
        prev_dir[i] = 0xFFU;
    }

    que[tail++] = start;
    used[start] = 1U;

    while (head < tail)
    {
        uint8_t id = que[head++];
        uint8_t x = (uint8_t)(id % 4U);
        uint8_t y = (uint8_t)(id / 4U);
        uint8_t d = 0;

        if (dst_x < 0)
        {
            if (!task2_visited[x][y])
            {
                found = id;
                break;
            }
        }
        else if (x == (uint8_t)dst_x && y == (uint8_t)dst_y)
        {
            found = id;
            break;
        }

        for (d = 0; d < 4; d++)
        {
            uint8_t nx = 0;
            uint8_t ny = 0;
            uint8_t nid = 0;

            if (!Task2_CanGo(x, y, d))
                continue;

            Task2_NextCell(x, y, d, &nx, &ny);
            nid = Task2_NodeId(nx, ny);

            if (used[nid])
                continue;

            used[nid] = 1U;
            prev[nid] = id;
            prev_dir[nid] = d;
            que[tail++] = nid;
        }
    }

    if (found == 0xFFU)
        return 0U;

    task2_plan_len = 0;
    while (found != start)
    {
        task2_plan[task2_plan_len++] = prev_dir[found];
        found = prev[found];
    }

    for (i = 0; i < task2_plan_len / 2U; i++)
    {
        uint8_t t = task2_plan[i];
        task2_plan[i] = task2_plan[task2_plan_len - 1U - i];
        task2_plan[task2_plan_len - 1U - i] = t;
    }

    task2_plan_i = 0;
    return 1U;
}

/*
 * 初始化第二问所有软件状态。
 *
 * 注意这里不会让车动，只是清空地图、挡板、路径、计数等信息。
 */
static void Task2_Init(void)
{
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t d = 0;

    for (x = 0; x < 4; x++)
    {
        for (y = 0; y < 4; y++)
        {
            task2_visited[x][y] = 0U;

            for (d = 0; d < 4; d++)
                task2_block[x][y][d] = 0U;
        }
    }

    Task2_InitWall();

    task2_x = 0;
    task2_y = 2;
    task2_dir = TASK2_RIGHT;
    task2_target_dir = TASK2_RIGHT;
    task2_visited_num = 0;
    task2_plan_len = 0;
    task2_plan_i = 0;
    task2_turn_steps_left = 0;
    task2_turn_speed_l = 0.0f;
    task2_turn_speed_r = 0.0f;
    task2_after_turn_state = TASK2_CHECK_FRONT;
}

/*
 * 准备转向到某个方向。
 *
 * 如果目标方向和当前方向一样，就不用转，直接去测距。
 * 如果差 90 度，就转一次。
 * 如果差 180 度，就连续转两次 90 度。
 */
static void Task2_StartTurnTo(uint8_t dir, Task2_State_t after_turn_state)
{
    uint8_t diff = (uint8_t)((dir + 4U - task2_dir) & 0x03U);

    task2_target_dir = dir;
    task2_after_turn_state = after_turn_state;

    if (diff == 0U)
    {
        Task2_SetState(task2_after_turn_state);
        return;
    }

    if (diff == 1U)
    {
        /* 右转：左轮前进 1r/s，右轮后退 1r/s。 */
        task2_turn_speed_l = 1.0f;
        task2_turn_speed_r = -1.0f;
        task2_turn_steps_left = 1U;
    }
    else if (diff == 2U)
    {
        /* 180 度转向：用两次右转 90 度实现。 */
        task2_turn_speed_l = 1.0f;
        task2_turn_speed_r = -1.0f;
        task2_turn_steps_left = 2U;
    }
    else
    {
        /* 左转：左轮后退 1r/s，右轮前进 1r/s。 */
        task2_turn_speed_l = -1.0f;
        task2_turn_speed_r = 1.0f;
        task2_turn_steps_left = 1U;
    }

    Task2_SetState(TASK2_TURN);
    App_Set_Speed(task2_turn_speed_l, task2_turn_speed_r);
}

/* 按方向走完一格后，更新当前坐标并标记新格子已经遍历。 */
static void Task2_UpdatePos(uint8_t dir)
{
    uint8_t nx = 0;
    uint8_t ny = 0;

    if (Task2_NextCell(task2_x, task2_y, dir, &nx, &ny))
    {
        task2_x = nx;
        task2_y = ny;
        Task2_MarkVisited(task2_x, task2_y);
    }
}

/*
 * Task2() 是第二问主状态机。
 *
 * 它必须像 Task1() 一样在 main 的 while(1) 里被持续调用，
 * 不能在里面写 while 死循环。这样激光测距、按键、LED 等周期任务仍然能运行。
 */
void Task2(void)
{
    unsigned long now = 0;
    unsigned long dt = 0;

    if (task2_state == TASK2_IDLE)
    {
        Task2_Init();

        /*
         * 从准备区穿过 A 口，进入 (0,2) 格子中心。
         *
         * 3500UL 是进入场地的时间，单位 ms。
         * 你实车调车时，让车刚好到第一个格子中心即可。
         */
        Task2_SetState(TASK2_ENTER);
        App_Set_Speed(1.0f, 1.0f);
        return;
    }

    mspm0_get_clock_ms(&now);
    dt = now - task2_start_time;

    switch (task2_state)
    {
        case TASK2_ENTER:
            Task2_SendSpeed(1.0f, 1.0f);
            if (dt >= 3500UL)
            {
                Task2_Stop();
                Task2_MarkVisited(task2_x, task2_y);
                Task2_SetState(TASK2_PLAN);
            }
            break;

        case TASK2_PLAN:
            /*
             * 还没遍历完 16 格：规划到最近的未访问格。
             * 已经遍历完：规划到 C 口所在格 (3,1)。
             */
            if (Task2_AllVisited())
            {
                if (task2_x == 3U && task2_y == 1U)
                {
                    Task2_SetState(TASK2_EXIT_TURN);
                }
                else if (Task2_BuildPath(3, 1))
                {
                    Task2_SetState(TASK2_STEP);
                }
                else
                {
                    Task2_Stop();
                    task2_flag = 0;
                    task2_state = TASK2_IDLE;
                }
            }
            else if (Task2_BuildPath(-1, -1))
            {
                Task2_SetState(TASK2_STEP);
            }
            else
            {
                Task2_Stop();
                task2_flag = 0;
                task2_state = TASK2_IDLE;
            }
            break;

        case TASK2_STEP:
            /*
             * 从规划路径里取出下一步方向。
             * 先转向，转完后再进入 TASK2_CHECK_FRONT 用激光探测。
             */
            if (task2_plan_i >= task2_plan_len)
            {
                Task2_SetState(TASK2_PLAN);
            }
            else
            {
                task2_target_dir = task2_plan[task2_plan_i];
                Task2_StartTurnTo(task2_target_dir, TASK2_CHECK_FRONT);
            }
            break;

        case TASK2_TURN:
            /*
             * 原地转弯。
             *
             * 900UL 是一次 90 度转弯时间，单位 ms。
             * 角度不够就调大，角度过了就调小。
             */
            Task2_SendSpeed(task2_turn_speed_l, task2_turn_speed_r);
            if (dt >= 900UL)
            {
                Task2_Stop();

                if (task2_turn_steps_left > 0U)
                    task2_turn_steps_left--;

                if (task2_turn_steps_left == 0U)
                {
                    task2_dir = task2_target_dir;
                    Task2_SetState(task2_after_turn_state);
                }
                else
                {
                    Task2_SetState(TASK2_TURN);
                    App_Set_Speed(task2_turn_speed_l, task2_turn_speed_r);
                }
            }
            break;

        case TASK2_CHECK_FRONT:
            /*
             * 只在格子中心判断前方是否有挡板。
             * 有挡板：记录这条边不能走，然后重新规划。
             * 没挡板：前进一格。
             */
            if (Task2_FrontBlocked())
            {
                Task2_MarkBlock(task2_x, task2_y, task2_target_dir);
                Task2_SetState(TASK2_PLAN);
            }
            else
            {
                Task2_SetState(TASK2_GO);
                App_Set_Speed(1.0f, 1.0f);
            }
            break;

        case TASK2_GO:
            /*
             * 前进一格。
             *
             * 2200UL 是 1r/s 速度下走 45cm 的时间，单位 ms。
             * 这个值一定要实车调，因为真实速度、地面摩擦、轮径都会影响距离。
             *
             * 注意：这里故意不在半路继续看前方测距。
             * 因为快到下一格中心时，前方传感器可能会看到“下一条网格边”上的挡板，
             * 如果这时误认为“当前这条边”被挡，就会把地图标错。
             */
            Task2_SendSpeed(1.0f, 1.0f);
            if (dt >= 2200UL)
            {
                Task2_Stop();
                Task2_UpdatePos(task2_target_dir);
                task2_plan_i++;
                Task2_SetState(TASK2_STEP);
            }
            break;

        case TASK2_BACK:
            /*
             * 后退兜底。
             *
             * 500UL 表示后退 500ms，单位 ms。
             * 后退结束后不更新格子坐标，因为认为车还在原来的格子附近。
             */
            Task2_SendSpeed(-1.0f, -1.0f);
            if (dt >= 500UL)
            {
                Task2_Stop();
                Task2_SetState(TASK2_PLAN);
            }
            break;

        case TASK2_EXIT_TURN:
            /*
             * 16 格遍历完，最后要从右侧 C 口出去。
             * 所以先把车头转到右方。
             */
            Task2_StartTurnTo(TASK2_RIGHT, TASK2_EXIT_GO);
            break;

        case TASK2_EXIT_GO:
            /*
             * 从 C 口驶出。
             *
             * 3400UL 是从 (3,1) 格子中心完全驶出场地的时间，单位 ms。
             * 如果车身还没完全出去就停了，把它调大。
             */
            Task2_SendSpeed(1.0f, 1.0f);
            if (dt >= 3400UL)
            {
                Task2_Stop();
                task2_flag = 0;
                task2_state = TASK2_IDLE;
            }
            break;

        default:
            Task2_Stop();
            task2_flag = 0;
            task2_state = TASK2_IDLE;
            break;
    }
}
