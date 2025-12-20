#pragma once
#include <Windows.h>
#include <graphics.h>

// 游戏状态枚举
enum class GameState {
    MENU,      // 主菜单
    PLAYING,   // 游戏中
    PAUSED,    // 暂停
    GAME_OVER  // 游戏结束
};

// 小鸟类的前置声明
class Bird;

// 管道类的前置声明
class PipeManager;

// 键盘状态结构
struct KeyState {
    bool spacePressed;  // 空格键刚刚按下
    bool spaceDown;     // 空格键正被按住
    bool escapePressed; // ESC键按下
    bool escapeDown;    // [新添加] 用于记录 ESC 键的物理按下状态
};

class Game {
private:
    // 窗口尺寸
    const int WINDOW_WIDTH = 800;    // 窗口宽度
    const int WINDOW_HEIGHT = 600;   // 窗口高度

    // 游戏状态
    GameState currentState;   // 当前游戏状态
    int score;                // 当前分数
    int highScore;            // 最高分
    float gameSpeed;          // 游戏速度
    bool isRunning;           // 游戏是否运行

    // 游戏对象指针
    Bird* bird;                // 小鸟对象
    PipeManager* pipeManager;  // 管道管理器

    // 输入状态
    KeyState keyState;

    // 帧率控制
    DWORD lastTime;      // 上一帧的时间（毫秒）
    DWORD currentTime;   // 当前时间（毫秒）
    float deltaTime;     // 两帧之间的时间差（秒）

    //暂停菜单相关
    int selectedOption = 0;         // 当前选中的选项索引 (0: 继续, 1: 难度增加, 2: 难度减小, 3: 退出)
    const int MAX_OPTIONS = 6;      // 菜单选项总数
    bool upKeyPressed = false;      // 防抖动：向上键状态
    bool downKeyPressed = false;    // 防抖动：向下键状态
    bool enterKeyPressed = false;   // 防抖动：回车键状态

    // --- [新定义] 菜单布局常量 ---
    const int MENU_X = 200;       // 菜单左侧起点
    const int MENU_Y_START = 200; // 第一行高度
    const int ITEM_HEIGHT = 50;   // 每行高度
    const int ITEM_WIDTH = 400;   // 响应鼠标的宽度

    // --- [新定义] 鼠标状态 ---
    ExMessage msg;                // EasyX 消息对象，用于获取鼠标坐标

    //
    void ApplyMenuOption();

public:
    // 构造函数 - 创建游戏对象时调用
    Game();

    // 析构函数 - 游戏对象销毁时调用
    ~Game();

    // 初始化游戏
    bool Initialize();

    // 主游戏循环
    void Run();

    // 处理键盘输入
    void ProcessInput();

    // 更新游戏逻辑
    void Update();

    // 绘制游戏画面
    void Render();

    // ============ 游戏状态控制 ============
    void StartGame();      // 开始游戏
    void RestartGame();    // 重新开始游戏
    void GameOver();       // 游戏结束
    void TogglePause();    // 暂停/继续切换

    // ============ 获取信息的方法 ============
    float GetDeltaTime() const { return deltaTime; }      // 获取帧时间差
    int GetWindowWidth() const { return WINDOW_WIDTH; }   // 获取窗口宽度
    int GetWindowHeight() const { return WINDOW_HEIGHT; } // 获取窗口高度
    int GetScore() const { return score; }                // 获取当前分数
    GameState GetState() const { return currentState; }   // 获取游戏状态

    // 增加分数
    void AddScore(int points);

private:
    // 计算帧时间差
    void CalculateDeltaTime();
};