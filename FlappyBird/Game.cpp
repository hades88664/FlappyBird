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
    , deltaTime(0.016f) {

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
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        if (!keyState.spaceDown) {
            keyState.spacePressed = true;
            keyState.spaceDown = true;
        }
    }
    else {
        keyState.spaceDown = false;
        keyState.spacePressed = false;
    }

    keyState.escapePressed = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
}

void Game::Update() {
    int passedCount = 0;
    // 临时存储小鸟位置，避免多次调用
    float birdX = bird->GetX();

    // 手动检查每个管道
    const auto& pipes = pipeManager->GetPipes();
    switch (currentState) {
    case GameState::MENU:
        if (keyState.spacePressed) {
            StartGame();
        }
        if (keyState.escapePressed) {
            isRunning = false;
        }
        break;

    case GameState::PLAYING:
        bird->Update(deltaTime);

        if (bird->GetY() <= 0) {
            GameOver();
            break;
        }
        if (bird->GetY() >= WINDOW_HEIGHT - 50) {
            GameOver();
            break;
        }

        pipeManager->Update(deltaTime);

        if (pipeManager->CheckCollision(bird->GetCollisionBox())) {
            GameOver();
            break;
        }

        // 关键修改：检测通过的管道并计分
       
        for (const auto& pipe : pipes) {
            if (birdX > pipe.posX && !pipe.passed) {
                // 这里需要修改管道状态，但我们不能直接修改const引用
                // 所以使用一个技巧：通过const_cast来修改
                const_cast<Pipe&>(pipe).passed = true;
                passedCount++;
            }
        }

        if (passedCount > 0) {
            AddScore(passedCount);
        }

        if (keyState.spacePressed) {
            bird->Jump();
        }

        if (keyState.escapePressed) {
            TogglePause();
        }
        break;

    case GameState::PAUSED:
        if (keyState.escapePressed) {
            TogglePause();
        }
        break;

    case GameState::GAME_OVER:
        if (keyState.spacePressed) {
            RestartGame();
        }
        if (keyState.escapePressed) {
            currentState = GameState::MENU;
        }
        break;
    }
}

void Game::Render() {
    // 清除背景
    cleardevice();

    // 绘制背景（天空）
    setfillcolor(RGB(135, 206, 235));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // 绘制地面
    setfillcolor(RGB(222, 184, 135));
    solidrectangle(0, WINDOW_HEIGHT - 50, WINDOW_WIDTH, WINDOW_HEIGHT);

    switch (currentState) {
    case GameState::MENU:
        // ... 菜单绘制代码 ...
        break;

    case GameState::PLAYING:
    case GameState::PAUSED:
        // 先绘制管道，再绘制小鸟（让小鸟在管道前面）
        pipeManager->Render();
        bird->Render();

        // 绘制分数
        settextcolor(WHITE);
        settextstyle(36, 0, _T("黑体"));
        TCHAR scoreStr[20];
        _stprintf_s(scoreStr, _T("得分: %d"), score);
        outtextxy(20, 20, scoreStr);

        if (currentState == GameState::PAUSED) {
            settextstyle(24, 0, _T("宋体"));
            outtextxy(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2, _T("游戏暂停"));
            outtextxy(WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 + 40, _T("按ESC键继续"));
        }
        break;

    case GameState::GAME_OVER:
        pipeManager->Render();
        bird->Render();

        // ... 游戏结束绘制代码 ...
        break;
    }

    // 关键：刷新缓冲区，一次性显示所有绘制的内容
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