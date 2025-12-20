// Game.cpp
#include <windows.h>
#include "Game.h"
#include "Bird.h"
#include "PipeManager.h"
#include <graphics.h>
#include <conio.h>
#include <cmath>
#include <iostream>

using namespace std;

Game::Game()
    : currentState(GameState::MENU)
    , score(0)
    , highScore(0)
    , gameSpeed(200.0f)
    , isRunning(true)
    , bird(nullptr)
    , pipeManager(nullptr)
    , lastTime(0)
    , currentTime(0)
    , deltaTime(0.016f) 
    , selectedOption(0)
    , MAX_OPTIONS(4)
    , upKeyPressed(false)
    , downKeyPressed(false)
    , enterKeyPressed(false) {

    keyState.spacePressed = false;
    keyState.spaceDown = false;
    keyState.escapePressed = false;
}

Game::~Game() {
    if (bird) delete bird;
    if (pipeManager) delete pipeManager;
}

bool Game::Initialize() {
    // 修改窗口创建方式，使用双缓冲
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT, EW_SHOWCONSOLE);

    // 开始批量绘制（双缓冲）
    BeginBatchDraw();

    setbkcolor(RGB(135, 206, 235));
    cleardevice();

    bird = new Bird();
    pipeManager = new PipeManager();

    bird->Initialize(WINDOW_WIDTH * 0.3f, WINDOW_HEIGHT / 2.0f);
    pipeManager->Initialize(WINDOW_WIDTH);

    lastTime = GetTickCount();

    cout << "游戏初始化成功！" << endl;
    cout << "按空格键开始游戏" << endl;
    cout << "按ESC键退出游戏" << endl;

    return true;
}

void Game::Run() {
    while (isRunning) {
        CalculateDeltaTime();
        ProcessInput();
        Update();
        Render();
        Sleep(1);
    }
}

void Game::ProcessInput() {
    // --- 处理空格键 (SPACE) ---
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        if (!keyState.spaceDown) {
            keyState.spacePressed = true;  // 只有在刚按下的那一帧为 true
            keyState.spaceDown = true;     // 标记物理键已按下
        }
        else {
            keyState.spacePressed = false; // 持续按住时，触发信号设为 false
        }
    }
    else {
        keyState.spaceDown = false;
        keyState.spacePressed = false;
    }

    // --- 处理退出/暂停键 (ESC) --- [修正逻辑]
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        if (!keyState.escapeDown) {         // 如果上一帧没按下，这一帧按下了
            keyState.escapePressed = true;  // 发出一个触发信号
            keyState.escapeDown = true;     // 记录物理键当前是按下状态
        }
        else {
            keyState.escapePressed = false; // 物理键还没弹起，不再发出信号
        }
    }
    else {
        keyState.escapeDown = false;        // 物理键已弹起，重置状态
        keyState.escapePressed = false;
    }
}

void Game::Update() {
    int passedCount = 0;
    float birdX = bird->GetX(); // 获取小鸟当前的 X 坐标用于计分判定
    const auto& pipes = pipeManager->GetPipes(); // 获取管道列表

    switch (currentState) {
    case GameState::MENU:
        if (keyState.spacePressed) StartGame();   // 菜单状态下按空格开始
        if (keyState.escapePressed) isRunning = false; // 按ESC退出
        break;

    case GameState::PLAYING:
        // --- 游戏运行中的逻辑 ---
        bird->Update(deltaTime); // 更新小鸟物理状态（掉落、移动）

        // 边界检查：撞天花板或掉出地面则游戏结束
        if (bird->GetY() <= 0 || bird->GetY() >= WINDOW_HEIGHT - 50) {
            GameOver();
            break;
        }

        pipeManager->Update(deltaTime); // 移动管道

        // 碰撞检查：小鸟是否撞到管道
        if (pipeManager->CheckCollision(bird->GetCollisionBox())) {
            GameOver();
            break;
        }

        // 计分逻辑：小鸟越过管道 X 坐标则加分
        for (const auto& pipe : pipes) {
            if (birdX > pipe.posX && !pipe.passed) {
                const_cast<Pipe&>(pipe).passed = true; // 标记该管道已计分
                passedCount++;
            }
        }
        if (passedCount > 0) AddScore(passedCount);

        if (keyState.spacePressed) bird->Jump(); // 正常游戏时按空格跳跃
        if (keyState.escapePressed) TogglePause(); // 按ESC进入暂停界面
        break;

    case GameState::PAUSED:
        // --- 暂停界面（调试菜单）逻辑 ---

        // 1. 处理鼠标消息：peekmessage 用于非阻塞地获取鼠标移动和点击
        while (peekmessage(&msg, EM_MOUSE)) {
            if (msg.message == WM_MOUSEMOVE) {
                if (msg.x >= (WINDOW_WIDTH / 2 - 200) && msg.x <= (WINDOW_WIDTH / 2 + 200)) {
                    int startY = WINDOW_HEIGHT / 2 - 120; // [修改] 菜单起点上移，因为选项变多了
                    int relativeY = msg.y - startY;
                    // 现在有 6 行，每行高度 55
                    if (relativeY >= 0 && relativeY < 6 * 55) {
                        selectedOption = relativeY / 55;
                    }
                }
            }
            // 鼠标左键点击时，执行对应的功能（一键开挂或返回）
            if (msg.message == WM_LBUTTONDOWN) {
                // 点击 2(上帝), 3(复原), 5(返回) 时触发逻辑 (4号排行暂无交互)
                if (selectedOption == 2 || selectedOption == 3 || selectedOption == 5) {
                    ApplyMenuOption();
                }
            }
        }
        // 2. 处理键盘微调：根据当前鼠标选中的行，按左右键增减参数
        if (selectedOption == 0) { // 修改重力
            if (GetAsyncKeyState(VK_LEFT) & 0x8000) bird->SetGravity(bird->GetGravity() - 2.0f);
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) bird->SetGravity(bird->GetGravity() + 2.0f);
        }
        else if (selectedOption == 1) { // 修改跳跃力
            if (GetAsyncKeyState(VK_LEFT) & 0x8000) bird->SetJumpForce(bird->GetJumpForce() + 2.0f);
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) bird->SetJumpForce(bird->GetJumpForce() - 2.0f);
        }

        // 暂停时再次按 ESC 则回到游戏
        if (keyState.escapePressed) TogglePause();
        break;

    case GameState::GAME_OVER:
        if (keyState.spacePressed) RestartGame();
        if (keyState.escapePressed) currentState = GameState::MENU;
        break;
    }
}

void Game::Render() {
    // ---------------------------------------------------------
    // 1. 基础准备：清屏并绘制背景
    // ---------------------------------------------------------
    cleardevice(); // 清除缓冲区

    // 绘制天空
    setfillcolor(RGB(135, 206, 235));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // 绘制地面
    setfillcolor(RGB(222, 184, 135));
    solidrectangle(0, WINDOW_HEIGHT - 50, WINDOW_WIDTH, WINDOW_HEIGHT);

    // ---------------------------------------------------------
    // 2. 根据游戏状态绘制核心内容
    // ---------------------------------------------------------
    switch (currentState) {
    case GameState::MENU:
        // 如果需要，可以在这里添加主菜单文字提示
        settextcolor(WHITE);
        settextstyle(50, 0, _T("黑体"));
        outtextxy(WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 - 50, _T("Flappy Bird"));
        settextstyle(20, 0, _T("微软雅黑"));
        outtextxy(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 50, _T("按 空格 开始游戏"));
        break;

    case GameState::PLAYING:
    case GameState::GAME_OVER:
        pipeManager->Render();
        bird->Render();
    case GameState::PAUSED:
        // A. 绘制游戏世界物体 (先画管道，再画小鸟)
        pipeManager->Render();
        bird->Render();

        // B. 绘制得分 UI (左上角)
        settextcolor(WHITE);
        settextstyle(36, 0, _T("Impact")); // 使用较硬朗的字体
        TCHAR scoreStr[20];
        _stprintf_s(scoreStr, _T("%d"), score);
        outtextxy(25, 20, scoreStr);

        // C. [核心逻辑] 如果是暂停状态，绘制美化后的控制面板
        if (currentState == GameState::PAUSED) {
            // --- 1. 面板尺寸调整 (6个选项需要更高面板) ---
            int panelW = 440;
            int panelH = 450; // [修改] 高度增加到 450
            int pX = (WINDOW_WIDTH - panelW) / 2;
            int pY = (WINDOW_HEIGHT - panelH) / 2;
            COLORREF panelBG = RGB(30, 32, 48);

            // 绘制主面板
            setfillcolor(panelBG);
            setlinecolor(RGB(60, 70, 100)); // 调暗面板边框，不抢眼
            setlinestyle(PS_SOLID, 1);
            solidroundrect(pX, pY, pX + panelW, pY + panelH, 15, 15);

            // 强制设置文字背景透明
            setbkmode(TRANSPARENT);

            // 标题文字（带一点发光感的蓝色）
            settextcolor(RGB(0, 180, 255));
            settextstyle(32, 0, _T("黑体"));
            outtextxy(pX + (panelW - 128) / 2, pY + 30, _T("控制中心"));

            // --- 2. 循环绘制 6 个功能选项 ---
            for (int i = 0; i < 6; i++) {
                int itemX = pX + 40;
                int itemY = pY + 95 + i * 52;
                int itemW = panelW - 80;
                int itemH = 44;
                bool isSelected = (i == selectedOption);

                // 【关键改动】：只有选中时才绘制高亮框
                if (isSelected) {
                    // 画一个半透明感的蓝色背景块
                    setfillcolor(RGB(45, 55, 85));
                    setlinecolor(RGB(0, 255, 255)); // 只有选中项有亮青色边框
                    solidroundrect(itemX - 10, itemY - 5, itemX + itemW + 10, itemY + itemH - 5, 5, 5);
                    settextcolor(WHITE);
                }
                else {
                    // 【重要】：不选中时，绝对不调用任何画矩形的函数！
                    settextcolor(RGB(130, 140, 160)); // 文字设为暗淡的蓝灰色
                }

                // 绘制选项文字
                settextstyle(20, 0, _T("微软雅黑"));
                TCHAR label[64];

                if (i == 0) {
                    outtextxy(itemX, itemY, _T("重力系数"));
                    // 进度条背景：只是一条细线
                    setlinecolor(RGB(55, 60, 80));
                    line(itemX + 110, itemY + 12, itemX + 340, itemY + 12);
                    // 进度填充：略粗的线
                    float v = bird->GetGravity() / 2000.0f;
                    setlinecolor(isSelected ? RGB(0, 210, 255) : RGB(100, 110, 130));
                    setlinestyle(PS_SOLID, 4); // 进度条厚度
                    line(itemX + 110, itemY + 12, itemX + 110 + (int)(230 * v), itemY + 12);
                    setlinestyle(PS_SOLID, 1); // 还原线型
                }
                else if (i == 1) {
                    outtextxy(itemX, itemY, _T("推进强度"));
                    setlinecolor(RGB(55, 60, 80));
                    line(itemX + 110, itemY + 12, itemX + 340, itemY + 12);
                    float v = abs(bird->GetJumpForce()) / 600.0f;
                    setlinecolor(isSelected ? RGB(255, 160, 0) : RGB(100, 110, 130));
                    setlinestyle(PS_SOLID, 4);
                    line(itemX + 110, itemY + 12, itemX + 110 + (int)(230 * v), itemY + 12);
                    setlinestyle(PS_SOLID, 1);
                }
                else if (i == 2) {
                    outtextxy(itemX, itemY, _T("一键开桂"));
                }
                else if (i == 3) {
                    outtextxy(itemX, itemY, _T("参数复原 "));
                }
                else if (i == 4) {
                    outtextxy(itemX, itemY, _T("分数排行"));
                }
                else if (i == 5) {
                    outtextxy(itemX, itemY, _T("返回主菜单"));
                }
            }

            // 底部操作引导
            settextcolor(RGB(80, 90, 110));
            settextstyle(14, 0, _T("微软雅黑"));
            outtextxy(pX + 105, pY + panelH - 35, _T("鼠标点击左键选择菜单  左右键微调参数"));
        }
    }
    FlushBatchDraw();
}
void Game::CalculateDeltaTime() {
    currentTime = GetTickCount();
    deltaTime = (currentTime - lastTime) / 1000.0f;

    if (deltaTime > 0.05f) {
        deltaTime = 0.05f;
    }

    lastTime = currentTime;
}

void Game::StartGame() {
    currentState = GameState::PLAYING;
    score = 0;
    bird->Reset();
    pipeManager->Reset();
    cout << "游戏开始！" << endl;
}

void Game::RestartGame() {
    currentState = GameState::PLAYING;
    score = 0;
    bird->Reset();
    pipeManager->Reset();
    cout << "重新开始游戏！" << endl;
}

void Game::GameOver() {
    currentState = GameState::GAME_OVER;
    bird->Kill();

    if (score > highScore) {
        highScore = score;
        cout << "新纪录！最高分：" << highScore << endl;
    }

    cout << "游戏结束！得分: " << score << endl;
}

void Game::TogglePause() {
    if (currentState == GameState::PLAYING) {
        currentState = GameState::PAUSED;
        cout << "游戏暂停" << endl;
    }
    else if (currentState == GameState::PAUSED) {
        currentState = GameState::PLAYING;
        cout << "游戏继续" << endl;
    }
}

void Game::AddScore(int points) {
    score += points;

    // 在控制台输出调试信息
    char buffer[100];
    sprintf_s(buffer, "得分 +%d，当前总分: %d\n", points, score);
    OutputDebugStringA(buffer);  // 输出到VS调试窗口

    // 也在控制台输出
    cout << "得分 +" << points << "，当前总分: " << score << endl;
}

void Game::ApplyMenuOption() {
    switch (selectedOption) {
    case 2: // 一键开桂
        bird->SetGravity(700.0f);    // 就是桂，怎么了
        bird->SetJumpForce(-200.0f); // 精心调试的傻瓜参数，谁玩谁知道
        cout << "开桂成功！重力:700, 跳跃:-200" << endl;
        break;

    case 3: // [新选项] 参数复原
        bird->SetGravity(980.0f);    // 恢复初始重力
        bird->SetJumpForce(-375.0f); // 恢复初始跳跃力
        cout << "参数已恢复默认" << endl;
        break;

    case 4: // 分数排行
        // 暂时无需设置交互
        break;

    case 5: // 退出到主菜单
        currentState = GameState::MENU;
        break;
    }
}