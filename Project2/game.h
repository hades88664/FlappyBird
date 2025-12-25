// game.h
#pragma once

#ifndef GAME_H
#define GAME_H

#include <graphics.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <windows.h>
#include "bird.h"
#include "pipemanager.h"
#include "constants.h"

// 分数记录结构体
struct ScoreEntry {
    std::string playerName;
    int score;
    int level;
    int playTime;
    time_t date;

    ScoreEntry() : score(0), level(1), playTime(0), date(time(0)) {}
    ScoreEntry(const std::string& name, int s, int lvl, int time)
        : playerName(name), score(s), level(lvl), playTime(time), date(::time(0)) {
    }

    // 用于排序：按分数降序，分数相同时按时间升序
    bool operator<(const ScoreEntry& other) const {
        if (score != other.score)
            return score > other.score;  // 分数高的在前
        return playTime < other.playTime; // 时间短的在前
    }
};

// 粒子类
class Particle {
private:
    float x, y;           // 位置
    float vx, vy;         // 速度
    float life;          // 当前生命周期
    float maxLife;       // 最大生命周期
    COLORREF color;      // 颜色
    int size;           // 大小
    int type;           // 类型（0:圆形，1:星星，2:线条）

public:
    Particle(float px, float py, COLORREF col, int t);
    void update(float deltaTime);
    void draw() const;
    void drawStar(int cx, int cy, int radius) const;

    // 添加公有访问器
    bool shouldRemove() const { return life <= 0; }
    float getLife() const { return life; }
    float getMaxLife() const { return maxLife; }
};

// 云朵类
class Cloud {
private:
    float x, y;         // 位置
    float speed;        // 移动速度
    int size;          // 大小
    int alpha;         // 透明度

public:
    Cloud();
    void update(float deltaTime);
    void draw() const;
};

// 游戏主类
class Game {
private:
    // 游戏对象
    Bird* bird;
    PipeManager* pipeManager;

    // 游戏状态
    GameState currentState;

    // 游戏数据
    int score;
    int highScore;
    int coins;
    int level;
    float gameSpeed;
    float pipeTimer;
    float gameTime;
    int pipesPassed;
    int nextPipeID;

    // UI数据
    std::string playerName;
    int selectedMenu;
    int selectedSetting;

    // 动画效果
    float animationTime;
    float shakeTime;
    float shakeIntensity;

    // 游戏设置
    float birdGravity;
    float birdJumpForce;
    bool showFPS;
    bool showHitboxes;
    int difficulty;

    // 输入处理
    bool keys[256];
    bool keyPressed[256];

    // 游戏元素
    std::vector<Particle> particles;
    std::vector<Cloud> clouds;
    std::vector<ScoreEntry> leaderboard;

    // 私有方法
    void updateInput();
    void handleInput();
    void handleMenuInput();
    void handleGameInput();
    void handlePauseInput();
    void handleGameOverInput();
    void handleLeaderboardInput();
    void handleSettingsInput();
    void handleHelpInput();
    void handleCreditsInput();
    void adjustSetting(int direction);
    void applyDifficulty();
    void updateGameplay(float deltaTime);
    void updateParticles(float deltaTime);
    void drawSkyBackground();
    void drawGround();
    void drawGameUI();
    void drawHitboxes();
    void drawFPS();
    void drawShakeEffect(int shakeX, int shakeY);
    void drawMenu();
    void drawPauseMenu();
    void drawGameOver();
    void drawLeaderboard();
    void drawSettings();
    void drawHelp();
    void drawCredits();
    void drawProgressBar(int x, int y, int width, int height, int type);

public:
    Game();
    ~Game();

    void init();
    void loadLeaderboard();
    void saveLeaderboard();
    void addToLeaderboard();
    void createParticles(float x, float y, int count, COLORREF color, int type);
    void shakeScreen(float intensity);
    void startNewGame();
    void gameOver();

    void update(float deltaTime);
    void render();
    void run();
};

#endif // GAME_H