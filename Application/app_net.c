#include "app_net.h"
#include "app_vl5310x.h"
#include "../keil/app_motor.h"

/*
 * 第二问遍历避障模块。
 *
 * 场地按 4x4 网格处理：
 *   x 方向：从左到右为 0,1,2,3
 *   y 方向：从上到下为 0,1,2,3
 *
 * A 口进入后默认到达 (0,2)，C 口默认在右侧中上位置，从 (3,1) 向右驶出。
 * 小车每次只做一件事：在格子中心判断前方能不能走；能走就走一格，不能走
 * 就把当前格子到下一格的这条边标为 blocked，然后重新 BFS 规划。
 */

#define NET_NODE_NUM   (NET_W * NET_H)
#define NET_START_X    0  // A 口进入后的起始格 x
#define NET_START_Y    2  // A 口进入后的起始格 y
#define NET_EXIT_X     3  // C 口所在格 x
#define NET_EXIT_Y     1  // C 口所在格 y

enum {
    NET_UP = 0,   // 车头朝上，y - 1
    NET_RIGHT,    // 车头朝右，x + 1
    NET_DOWN,     // 车头朝下，y + 1
    NET_LEFT      // 车头朝左，x - 1
};

volatile NetState_t net_state = NET_IDLE;
volatile uint8_t net_x = NET_START_X;
volatile uint8_t net_y = NET_START_Y;
volatile uint8_t net_dir = NET_RIGHT;
volatile uint8_t net_visited_num = 0;

static uint8_t visited[NET_W][NET_H];   // visited[x][y] = 1 表示该区块已经经过
static uint8_t block[NET_W][NET_H][4];  // block[x][y][dir] = 1 表示该格朝 dir 的边不能通过
static uint8_t plan[NET_NODE_NUM];      // BFS 得到的方向序列，每个元素是 NET_UP/RIGHT/DOWN/LEFT
static uint8_t plan_len = 0;            // 当前路径的总步数
static uint8_t plan_i = 0;              // 当前执行到路径中的第几步
static uint8_t move_active = 0;         // 1 表示当前有一段定距离运动正在执行
static uint8_t target_dir = NET_RIGHT;  // 本次准备走的方向
static float move_start_l = 0.0f;       // 本段运动开始时左轮累计位置，单位 cm
static float move_start_r = 0.0f;       // 本段运动开始时右轮累计位置，单位 cm
static float move_dist = 0.0f;          // 本段运动目标距离，单位 cm

extern float position_l; // app_motor 中更新的左轮累计距离，单位 cm
extern float position_r; // app_motor 中更新的右轮累计距离，单位 cm

// Cortex-M0+ 上尽量少依赖库函数，这里用一个简单绝对值函数。
static float AbsF(float x)
{
    return (x >= 0.0f) ? x : -x;
}

// 把二维坐标压成 0~15 的节点编号，方便 BFS 数组访问。
static uint8_t NodeId(uint8_t x, uint8_t y)
{
    return y * NET_W + x;
}

// 判断坐标是否仍在 4x4 地图内部。
static uint8_t CellOk(int8_t x, int8_t y)
{
    return (x >= 0 && x < NET_W && y >= 0 && y < NET_H) ? 1 : 0;
}

/*
 * 根据当前格子和方向计算下一格。
 * 返回 1 表示下一格合法，返回 0 表示该方向会越过场地边界。
 */
static uint8_t NextCell(uint8_t x, uint8_t y, uint8_t dir, uint8_t *nx, uint8_t *ny)
{
    int8_t tx = (int8_t)x;
    int8_t ty = (int8_t)y;

    if (dir == NET_UP) ty--;
    else if (dir == NET_RIGHT) tx++;
    else if (dir == NET_DOWN) ty++;
    else tx--;

    if (!CellOk(tx, ty))
        return 0;

    *nx = (uint8_t)tx;
    *ny = (uint8_t)ty;
    return 1;
}

// 当前方向的反方向，用于同步标记相邻格子的阻塞边。
static uint8_t OppDir(uint8_t dir)
{
    return (dir + 2) & 0x03;
}

/*
 * 标记一条不能走的边。
 * 例如从 (1,2) 向右被挡，则不仅 block[1][2][RIGHT] 要置 1，
 * 相邻的 (2,2) 向左也应置 1，保证 BFS 从两边都不会穿过挡板。
 */
static void MarkBlock(uint8_t x, uint8_t y, uint8_t dir)
{
    uint8_t nx, ny;

    block[x][y][dir] = 1;

    if (NextCell(x, y, dir, &nx, &ny))
        block[nx][ny][OppDir(dir)] = 1;
}

// 判断从某格朝某方向是否可以进入相邻格，边界和已知挡板都会返回不可走。
static uint8_t CanGo(uint8_t x, uint8_t y, uint8_t dir)
{
    uint8_t nx, ny;

    if (block[x][y][dir])
        return 0;

    return NextCell(x, y, dir, &nx, &ny);
}

// 初始化外墙：地图最外圈不能越界行驶，统一当作 blocked 边处理。
static void InitWall(void)
{
    for (uint8_t x = 0; x < NET_W; x++)
    {
        MarkBlock(x, 0, NET_UP);
        MarkBlock(x, NET_H - 1, NET_DOWN);
    }

    for (uint8_t y = 0; y < NET_H; y++)
    {
        MarkBlock(0, y, NET_LEFT);
        MarkBlock(NET_W - 1, y, NET_RIGHT);
    }
}

// 每进入一个格子中心，就调用一次；重复进入同一格不会重复计数。
static void MarkVisited(uint8_t x, uint8_t y)
{
    if (!visited[x][y])
    {
        visited[x][y] = 1;
        net_visited_num++;
    }
}

// 第二问要求遍历 16 个区块，计数达到 16 就可以去 C 口。
static uint8_t AllVisited(void)
{
    return (net_visited_num >= NET_NODE_NUM) ? 1 : 0;
}

/*
 * 在格子中心检测前方格子边是否有挡板。
 * 优先使用 app_vl5310x 维护的前方障碍标志；同时再直接读一次距离，
 * 方便你调 NET_BLOCK_MM 时观察阈值。这个函数只在 NET_CHECK 状态调用，
 * 不在行进过程中调用，避免快到下一格时误把下一条边判断成当前边。
 */
static uint8_t FrontBlocked(void)
{
    uint16_t d = App_VL5310X_GetDistance(VL5310X_FRONT);

    if (vl5310x_front_obstacle_flag)
        return 1;

    if (d > 0 && d <= NET_BLOCK_MM)
        return 1;

    return 0;
}

// 停车并清除当前运动标志。速度 0 是兜底，位置 SP 清零是为了让位置 PID 不继续追目标。
static void CarStop(void)
{
    Set_Position_SP_L(0.0f);
    Set_Position_SP_R(0.0f);
    App_Set_Speed(0.0f, 0.0f);
    move_active = 0;
}

/*
 * 开始一段直线定距离运动。
 * app_motor 的 Set_Position_SP_L/R 是相对距离设定，所以这里先记录起点，
 * 后面 MoveDone() 用左右轮累计位置判断这一段是否已经走够。
 */
static void StartMove(float dist_cm)
{
    App_Update_Data();

    move_start_l = position_l;
    move_start_r = position_r;
    move_dist = dist_cm;
    move_active = 1;

    Set_Position_SP_L(dist_cm);
    Set_Position_SP_R(dist_cm);
}

/*
 * 周期推进位置 PID，并判断当前直线运动是否完成。
 * 左右轮都走到目标距离附近才算完成，避免单侧轮打滑导致位置估计明显偏差。
 */
static uint8_t MoveDone(void)
{
    float dl, dr;

    if (!move_active)
        return 1;

    App_Position_Pro();
    App_Update_Data();

    dl = AbsF(position_l - move_start_l);
    dr = AbsF(position_r - move_start_r);

    if (dl + NET_POS_TOL_CM >= move_dist &&
        dr + NET_POS_TOL_CM >= move_dist)
    {
        CarStop();
        return 1;
    }

    return 0;
}

/*
 * 把车头转到指定方向。
 * net_dir 是软件内部记录的车头朝向；这里假设 Turn_Left_90/Turn_Right_90
 * 内部已经完成“转到位”这件事。如果你的转弯函数是非阻塞式，需要把这里
 * 拆成“发起转弯”和“等待转弯完成”两个状态。
 */
static void TurnTo(uint8_t dir)
{
    uint8_t diff = (dir + 4 - net_dir) & 0x03;

    if (diff == 1)
    {
        Turn_Right_90();
    }
    else if (diff == 2)
    {
        Turn_Right_90();
        Turn_Right_90();
    }
    else if (diff == 3)
    {
        Turn_Left_90();
    }

    net_dir = dir;
}

// 走完一格后更新软件坐标，并标记新格子已访问。
static void UpdatePos(uint8_t dir)
{
    uint8_t nx, ny;

    if (NextCell(net_x, net_y, dir, &nx, &ny))
    {
        net_x = nx;
        net_y = ny;
        MarkVisited(net_x, net_y);
    }
}

/*
 * BFS 路径规划。
 *
 * dst_x/dst_y >= 0：规划到指定目标格，例如最后去 C 口所在的 (3,1)。
 * dst_x < 0：规划到“最近的未访问格子”，用于遍历阶段。
 *
 * BFS 只使用目前已经知道的 blocked 边。随机挡板不需要一开始全部知道，
 * 车在格子中心探测到挡板后调用 MarkBlock()，再重新进入本函数规划即可。
 */
static uint8_t BuildPath(int8_t dst_x, int8_t dst_y)
{
    uint8_t que[NET_NODE_NUM];
    uint8_t used[NET_NODE_NUM] = {0};
    uint8_t prev[NET_NODE_NUM];
    uint8_t prev_dir[NET_NODE_NUM];
    uint8_t head = 0;
    uint8_t tail = 0;
    uint8_t found = 0xFF;
    uint8_t start = NodeId(net_x, net_y);

    for (uint8_t i = 0; i < NET_NODE_NUM; i++)
    {
        prev[i] = 0xFF;
        prev_dir[i] = 0xFF;
    }

    que[tail++] = start;
    used[start] = 1;

    // 标准 BFS：队列里每个节点最多入队一次，4x4 地图最多 16 个节点。
    while (head < tail)
    {
        uint8_t id = que[head++];
        uint8_t x = id % NET_W;
        uint8_t y = id / NET_W;

        if (dst_x < 0)
        {
            if (!visited[x][y])
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

        for (uint8_t d = 0; d < 4; d++)
        {
            uint8_t nx, ny, nid;

            if (!CanGo(x, y, d))
                continue;

            NextCell(x, y, d, &nx, &ny);
            nid = NodeId(nx, ny);

            if (used[nid])
                continue;

            used[nid] = 1;
            prev[nid] = id;
            prev_dir[nid] = d;
            que[tail++] = nid;
        }
    }

    if (found == 0xFF)
        return 0;

    // 从终点沿 prev[] 倒推回起点，得到的方向序列是反的。
    plan_len = 0;
    while (found != start)
    {
        plan[plan_len++] = prev_dir[found];
        found = prev[found];
    }

    // 反转方向序列，使 plan[0] 成为当前要走的第一步。
    for (uint8_t i = 0; i < plan_len / 2; i++)
    {
        uint8_t t = plan[i];
        plan[i] = plan[plan_len - 1 - i];
        plan[plan_len - 1 - i] = t;
    }

    plan_i = 0;
    return 1;
}

/*
 * 初始化地图和任务状态。
 * 注意这里不会让车运动，只是清空 visited/block/plan 等软件状态。
 */
void App_Net_Init(void)
{
    for (uint8_t x = 0; x < NET_W; x++)
    {
        for (uint8_t y = 0; y < NET_H; y++)
        {
            visited[x][y] = 0;

            for (uint8_t d = 0; d < 4; d++)
                block[x][y][d] = 0;
        }
    }

    InitWall();

    net_state = NET_IDLE;
    net_x = NET_START_X;
    net_y = NET_START_Y;
    net_dir = NET_RIGHT;
    net_visited_num = 0;
    plan_len = 0;
    plan_i = 0;
    move_active = 0;
}

/*
 * 启动第二问任务。
 * 启动后第一步是从准备区经 A 口进入起始格中心，距离由 NET_ENTER_CM 决定。
 */
void App_Net_Start(void)
{
    App_Net_Init();
    net_state = NET_ENTER;
    StartMove(NET_ENTER_CM);
}

/*
 * 第二问状态机。
 *
 * 主循环需要持续调用本函数，同时仍要周期调用 App_VL5310X_Proc() 更新测距。
 * 状态流转：
 *   ENTER -> PLAN -> STEP -> CHECK -> GO -> STEP ...
 *   16 格遍历完成后 -> EXIT_TURN -> EXIT_GO -> DONE
 */
void App_Net_Proc(void)
{
    if (net_state == NET_IDLE || net_state == NET_DONE || net_state == NET_ERROR)
        return;

    switch (net_state)
    {
        case NET_ENTER:
            // 从 A 口进入起始格中心，进入后标记起始格已访问。
            if (MoveDone())
            {
                MarkVisited(net_x, net_y);
                net_state = NET_PLAN;
            }
            break;

        case NET_PLAN:
            // 遍历完成后规划去出口；未完成时规划去最近的未访问格。
            if (AllVisited())
            {
                if (net_x == NET_EXIT_X && net_y == NET_EXIT_Y)
                    net_state = NET_EXIT_TURN;
                else if (BuildPath(NET_EXIT_X, NET_EXIT_Y))
                    net_state = NET_STEP;
                else
                    net_state = NET_ERROR;
            }
            else if (BuildPath(-1, -1))
            {
                net_state = NET_STEP;
            }
            else
            {
                net_state = NET_ERROR;
            }
            break;

        case NET_STEP:
            // 取 BFS 路径里的下一步方向，并先把车头转到该方向。
            if (plan_i >= plan_len)
            {
                net_state = NET_PLAN;
                break;
            }

            target_dir = plan[plan_i];
            TurnTo(target_dir);
            net_state = NET_CHECK;
            break;

        case NET_CHECK:
            // 只在格子中心探边：有挡板就记录 blocked 并重规划，没有挡板才走一格。
            if (FrontBlocked())
            {
                MarkBlock(net_x, net_y, target_dir);
                net_state = NET_PLAN;
            }
            else
            {
                StartMove(NET_CELL_CM);
                net_state = NET_GO;
            }
            break;

        case NET_GO:
            // 直线走一格。这里不再看前方测距，防止接近下一格时误判下一条边。
            if (MoveDone())
            {
                UpdatePos(target_dir);
                plan_i++;
                net_state = NET_STEP;
            }
            break;

        case NET_EXIT_TURN:
            // 到达 C 口所在格后，车头朝右，执行驶出场地距离。
            TurnTo(NET_RIGHT);
            StartMove(NET_EXIT_CM);
            net_state = NET_EXIT_GO;
            break;

        case NET_EXIT_GO:
            // 车身完全驶出 C 口后结束，NET_EXIT_CM 需要实车按车长调。
            if (MoveDone())
                net_state = NET_DONE;
            break;

        default:
            break;
    }
}

// 外部调试用：可以在串口或 LCD 上查看某格是否已经遍历。
uint8_t App_Net_IsVisited(uint8_t x, uint8_t y)
{
    if (x >= NET_W || y >= NET_H)
        return 0;

    return visited[x][y];
}
