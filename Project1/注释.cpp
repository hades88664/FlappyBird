///1.

#define _CRT_SECURE_NO_WARNINGS // 禁用安全警告（如sprintf等）

#include <graphics.h>  // EasyX图形库
#include <conio.h>     // 控制台输入输出
#include <time.h>      // 时间相关
#include <windows.h>   // Windows API（用于高精度计时和GetAsyncKeyState）
#include <vector>
#include <string>
#include <iostream>
#include <fstream>     // 文件流（用于存档）
#include <sstream>
#include <algorithm>   // 排序算法
#include <cmath>
#include <map>

using namespace std;

// --- 基础常量 ---
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int GROUND_HEIGHT = 60; // 地面高度
const int FPS = 60;           // 目标帧率

// --- 游戏状态机枚举 ---
enum GameState {
    STATE_MENU = 0,       // 主菜单
    STATE_PLAYING = 1,    // 游戏中
    STATE_PAUSED = 2,     // 暂停
    STATE_GAME_OVER = 3,  // 游戏结束
    STATE_LEADERBOARD = 4,// 排行榜
    STATE_SETTINGS = 5,   // 设置
    STATE_HELP = 6,       // 帮助
    STATE_CREDITS = 7     // 制作人员
};

// --- 颜色常量定义 ---
const COLORREF COLOR_SKY_START = RGB(135, 206, 235);
const COLORREF COLOR_SKY_END = RGB(70, 130, 180);
const COLORREF COLOR_GROUND = RGB(222, 184, 135);
const COLORREF COLOR_GRASS = RGB(124, 252, 0);
const COLORREF COLOR_CLOUD = RGB(255, 255, 255);
const COLORREF COLOR_BIRD_BODY = RGB(255, 165, 0);
const COLORREF COLOR_BIRD_WING = RGB(255, 140, 0);
const COLORREF COLOR_BIRD_EYE = RGB(255, 255, 255);
const COLORREF COLOR_BIRD_BEAK = RGB(255, 69, 0);
const COLORREF COLOR_PIPE_GREEN = RGB(50, 205, 50);
const COLORREF COLOR_PIPE_BLUE = RGB(30, 144, 255);
const COLORREF COLOR_PIPE_PURPLE = RGB(148, 0, 211);
const COLORREF COLOR_PIPE_RED = RGB(220, 20, 60);
const COLORREF COLOR_TEXT_WHITE = RGB(255, 255, 255);
const COLORREF COLOR_TEXT_YELLOW = RGB(255, 255, 0);
const COLORREF COLOR_TEXT_RED = RGB(255, 0, 0);
const COLORREF COLOR_TEXT_GREEN = RGB(0, 255, 0);
const COLORREF COLOR_TEXT_BLUE = RGB(0, 191, 255);
const COLORREF COLOR_TEXT_PURPLE = RGB(128, 0, 128);


// 排行榜得分条目
struct ScoreEntry {
    string playerName;
    int score;
    int level;
    time_t date;     // 记录日期
    int playTime;    // 单局时长

    // 构造函数
    ScoreEntry(string name, int s, int l, int t) : 
        playerName(name), score(s), level(l), date(time(0)), playTime(t) {
    }
       
    // 排序规则重载：分数高的排前面，分数相同时用时短的排前面
    bool operator<(const ScoreEntry& other) const {
        if (score != other.score) return score > other.score;
        return playTime < other.playTime;
    }
};


///辅助结构体（数据管理与视觉特效）
///这部分定义了得分条目、粒子系统和云朵背景。

// 粒子系统：用于碰撞爆炸、跳跃尾迹、金币收集效果
struct Particle {
    float x, y;
    float vx, vy;    // 速度向量
    float life;      // 剩余生命时间
    float maxLife;   // 初始生命
    COLORREF color;
    int size;
    int type;        // 0:圆形, 1:星星, 2:线段

    Particle(float px, float py, COLORREF col, int t = 0) {
        x = px;
        y = py;
        vx = (rand() % 200 - 100) / 50.0f;
        vy = (rand() % 200 - 150) / 50.0f;
        life = 1.0f + (rand() % 100) / 100.0f;
        maxLife = life;
        color = col;
        size = 2 + rand() % 5;
        type = t;
    }

    void update(float deltaTime) {
        x += vx * deltaTime * 60;
        y += vy * deltaTime * 60;
        vy += 0.3f * deltaTime * 60; // 粒子受重力影响下落
        life -= deltaTime;           // 逐渐消亡
    }

    void update(float deltaTime) {
        x += vx * deltaTime * 60;
        y += vy * deltaTime * 60;
        vy += 0.3f * deltaTime * 60;
        life -= deltaTime;
    }

    void draw() const {
        if (life <= 0) return;
        // 根据生命比例计算透明度效果（颜色变暗）
        float alpha = life / maxLife;
        int r = (int)(GetRValue(color) * alpha);
        int g = (int)(GetGValue(color) * alpha);
        int b = (int)(GetBValue(color) * alpha);

        setfillcolor(RGB(r, g, b));
        setlinecolor(RGB(r, g, b));

        if (type == 0) {
            solidcircle((int)x, (int)y, size);
        }
        else if (type == 1) {
            drawStar((int)x, (int)y, size);
        }
        else if (type == 2) {
            setlinestyle(PS_SOLID, size);
            line((int)x, (int)y, (int)(x + vx * 2), (int)(y + vy * 2));
        }
    }

    void drawStar(int cx, int cy, int radius) const {
        POINT points[10];
        for (int i = 0; i < 10; i++) {
            float angle = 3.14159f * 2 * i / 10;
            float r = (i % 2 == 0) ? radius : radius / 2;
            points[i].x = cx + (int)(r * cos(angle));
            points[i].y = cy + (int)(r * sin(angle));
        }
        solidpolygon(points, 10);
        // ... 绘制代码 ...
    }
};

// 背景云朵：简单的视差滚动效果
struct Cloud {
    float x, y;
    float speed;
    int size;
    int alpha;

    Cloud() {
        x = rand() % SCREEN_WIDTH;
        y = rand() % 200;
        speed = (rand() % 50 + 20) / 100.0f;
        size = 20 + rand() % 40;
        alpha = 150 + rand() % 100;
    }

    void update(float deltaTime) {
        x -= speed * deltaTime * 60;
        if (x < -100) {
            x = SCREEN_WIDTH + 100;
            y = rand() % 200;
        }
    }

    void draw() const {
        setfillcolor(RGB(255, 255, 255));
        setlinecolor(RGB(255, 255, 255));

        solidcircle((int)x, (int)y, size);
        solidcircle((int)(x + size * 0.6), (int)(y - size * 0.3), (int)(size * 0.7));
        solidcircle((int)(x + size * 1.2), (int)y, (int)(size * 0.5));
        solidcircle((int)(x - size * 0.4), (int)(y + size * 0.3), (int)(size * 0.6));
    }
};


///3. 核心实体类（小鸟与障碍物）
///这是游戏逻辑的核心载体。
///3.1 Bird 类（玩家控制）

class Bird {
private:
    float x, y;
    float velocity;     // 当前纵向速度
    float gravity;      // 重力加速度
    float jumpForce;    // 跳跃瞬间向上的冲力
    float wingAngle;    // 翅膀煽动的动画角度
    int comboCount;     // 连击数（连续通过管子）
    float comboTime;    // 连击有效倒计时
    int radius;
    bool alive;
    float rotation;
    float wingSpeed;
    int scoreMultiplier;
    COLORREF color;

public:
    Bird() {
        reset();
    }
    void reset() {
        x = SCREEN_WIDTH / 4;
        y = SCREEN_HEIGHT / 2;
        velocity = 0;
        gravity = 0.5f;
        jumpForce = -8.5f;
        radius = 15;
        alive = true;
        rotation = 0;
        wingAngle = 0;
        wingSpeed = 0.3f;
        scoreMultiplier = 1;
        color = COLOR_BIRD_BODY;
        comboCount = 0;
        comboTime = 0;
    }

    void update(float deltaTime) {
        if (!alive) return;

        velocity += gravity; // 重力应用
        y += velocity;       // 位置更新
        wingAngle += wingSpeed;
        if (wingAngle > 6.28318f) wingAngle = 0;

        rotation = velocity * 3;
        if (rotation > 30) rotation = 30;
        if (rotation < -30) rotation = -30;

        // 连击逻辑：如果规定时间内没通过下一个管子，连击重置
        if (comboTime > 0) {
            comboTime -= deltaTime;
            if (comboTime <= 0) {
                comboCount = 0;
                scoreMultiplier = 1;
            }
        }

        // 边界限制
        if (y < radius) {
            y = radius;
            velocity = 0;
        }
        if (y > SCREEN_HEIGHT - GROUND_HEIGHT - radius) {
            y = SCREEN_HEIGHT - GROUND_HEIGHT - radius;
            velocity = 0;
        }
    }

    void jump() {
        if (alive) {
            velocity = jumpForce;
            wingAngle = 0;
        }    // 改变速度方向向上
    }
    void draw() const {
        if (!alive) return;

        setlinestyle(PS_SOLID, 2);

        setfillcolor(color);
        setlinecolor(RGB(GetRValue(color) * 0.8, GetGValue(color) * 0.8, GetBValue(color) * 0.8));
        solidcircle((int)x, (int)y, radius);

        float wingOffset = sin(wingAngle * 4) * 5;
        setfillcolor(COLOR_BIRD_WING);
        solidcircle((int)(x - radius * 0.7), (int)(y + wingOffset), (int)(radius * 0.8));

        setfillcolor(COLOR_BIRD_EYE);
        solidcircle((int)(x + radius * 0.5), (int)(y - radius * 0.3), (int)(radius * 0.4));
        setfillcolor(RGB(0, 0, 0));
        solidcircle((int)(x + radius * 0.7), (int)(y - radius * 0.3), (int)(radius * 0.2));

        setfillcolor(COLOR_BIRD_EYE);
        solidcircle((int)(x + radius * 0.65), (int)(y - radius * 0.35), (int)(radius * 0.08));

        setfillcolor(COLOR_BIRD_BEAK);
        POINT beak[3] = {
            {(int)(x + radius), (int)y},
            {(int)(x + radius + 20), (int)(y - 7)},
            {(int)(x + radius + 20), (int)(y + 7)}
        };
        solidpolygon(beak, 3);

        setfillcolor(RGB(255, 182, 193));
        setlinecolor(RGB(255, 182, 193));
        solidcircle((int)(x + radius * 0.2), (int)(y + radius * 0.4), (int)(radius * 0.3));

        POINT tail[4] = {
            {(int)(x - radius), (int)y},
            {(int)(x - radius - 15), (int)(y - 8)},
            {(int)(x - radius - 15), (int)(y + 8)},
            {(int)(x - radius), (int)y}
        };
        solidpolygon(tail, 4);

        if (comboTime > 0) {
            drawComboEffect();
        }
    }

    void drawComboEffect() const {
        char comboText[20];
        sprintf_s(comboText, "COMBO x%d", comboCount);

        wchar_t wcomboText[20];
        swprintf_s(wcomboText, 20, L"COMBO x%d", comboCount);
        int textWidth = textwidth(wcomboText);
        int textHeight = textheight(wcomboText);

        settextstyle(16, 0, _T("Arial"));
        settextcolor(RGB(255, 215, 0));
        setbkmode(TRANSPARENT);
        outtextxy((int)x - textWidth / 2, (int)y - radius - textHeight - 5, wcomboText);
    }

    // 获取碰撞矩形（稍微缩小一点，增加玩家容错率）
    RECT getCollisionRect() const {
        RECT rect;
        int margin = 3;
        rect.left = (int)x - radius + margin;
        rect.top = (int)y - radius + margin;
        rect.right = (int)x + radius - margin;
        rect.bottom = (int)y + radius - margin;
        return rect;
    }
    // ... 绘制逻辑：绘制身体、翅膀、眼睛、嘴巴、连击文字 ...
    void kill() {
        alive = false;
        color = RGB(128, 128, 128);
    }

    void addCombo() {
        comboCount++;
        comboTime = 2.0f;
        scoreMultiplier = 1 + comboCount / 3;
    }

    float getX() const { return x; }
    float getY() const { return y; }
    bool isAlive() const { return alive; }
    int getScoreMultiplier() const { return scoreMultiplier; }
    int getComboCount() const { return comboCount; }
    void setGravity(float g) { gravity = g; }
    void setJumpForce(float f) { jumpForce = f; }
    float getGravity() const { return gravity; }
    float getJumpForce() const { return jumpForce; }
};

//3.2 Pipe 类（障碍物）

class Pipe {
private:
    float x;
    float gapY;       // 缺口中心点的Y坐标
    int gapHeight;    // 缺口高度（小鸟通过的空间）
    bool hasCoin;     // 是否带金币
    bool coinCollected;
    int width;
    bool passed;
    COLORREF color;
    int id;
    float coinY;


public:
    Pipe(float startX, int pipeID) {
        x = startX;
        gapY = 150 + rand() % (SCREEN_HEIGHT - GROUND_HEIGHT - 250);// 随机缺口位置
        width = 70;
        gapHeight = 160;
        passed = false;
        id = pipeID;
        hasCoin = (rand() % 100) < 30;
        coinY = gapY;
        coinCollected = false;

        int colorType = rand() % 4;
        switch (colorType) {
        case 0: color = COLOR_PIPE_GREEN; break;
        case 1: color = COLOR_PIPE_BLUE; break;
        case 2: color = COLOR_PIPE_PURPLE; break;
        case 3: color = COLOR_PIPE_RED; break;
        }
    }

    void update(float speed) {
        x -= speed;
    }


    void draw() const {
        drawPipe(x, 0, gapY - gapHeight / 2, false);
        drawPipe(x, gapY + gapHeight / 2, SCREEN_HEIGHT - GROUND_HEIGHT, true);
        // 分别绘制上管子和下管子
        // 绘制管子的装饰纹理，增强视觉效果
        if (hasCoin && !coinCollected) {
            drawCoin();
        }
    }

    void drawPipe(float px, float top, float bottom, bool isBottom) const {
        setfillcolor(color);
        fillrectangle((int)px, (int)top, (int)(px + width), (int)bottom);

        COLORREF capColor = RGB(
            (int)(GetRValue(color) * 0.8),
            (int)(GetGValue(color) * 0.8),
            (int)(GetBValue(color) * 0.8)
        );
        setfillcolor(capColor);

        if (!isBottom) {
            fillrectangle((int)px - 10, (int)bottom - 20, (int)(px + width + 10), (int)bottom);
        }
        else {
            fillrectangle((int)px - 10, (int)top, (int)(px + width + 10), (int)top + 20);
        }

        setfillcolor(RGB(
            (int)(GetRValue(color) * 0.6),
            (int)(GetGValue(color) * 0.6),
            (int)(GetBValue(color) * 0.6)
        ));

        int textureSpacing = 25;
        if (!isBottom) {
            for (int y = (int)top + 10; y < (int)bottom - 25; y += textureSpacing) {
                fillrectangle((int)px + 10, y, (int)(px + width - 10), y + 10);
            }
        }
        else {
            for (int y = (int)top + 30; y < (int)bottom - 10; y += textureSpacing) {
                fillrectangle((int)px + 10, y, (int)(px + width - 10), y + 10);
            }
        }
    }

    void drawCoin() const {
        float coinX = x + width / 2;

        setfillcolor(RGB(255, 215, 0));
        setlinecolor(RGB(218, 165, 32));
        solidcircle((int)coinX, (int)coinY, 12);

        setlinecolor(RGB(255, 255, 0));
        circle((int)coinX, (int)coinY, 12);

        settextstyle(14, 0, _T("Arial"));
        settextcolor(RGB(255, 255, 255));
        setbkmode(TRANSPARENT);
        outtextxy((int)coinX - 4, (int)coinY - 7, L"$");
    }

    RECT getTopRect() const {
        RECT rect;
        rect.left = (int)x;
        rect.top = 0;
        rect.right = (int)(x + width);
        rect.bottom = (int)(gapY - gapHeight / 2);
        return rect;
    }

    RECT getBottomRect() const {
        RECT rect;
        rect.left = (int)x;
        rect.top = (int)(gapY + gapHeight / 2);
        rect.right = (int)(x + width);
        rect.bottom = SCREEN_HEIGHT - GROUND_HEIGHT;
        return rect;
    }

    RECT getCoinRect() const {
        RECT rect;
        rect.left = (int)(x + width / 2 - 12);
        rect.top = (int)coinY - 12;
        rect.right = (int)(x + width / 2 + 12);
        rect.bottom = (int)coinY + 12;
        return rect;
    }

    // 碰撞检测：检查小鸟是否进入了上管子或下管子的矩形区域
    bool checkCollision(RECT otherRect) const {
        RECT topPipe = getTopRect();
        RECT bottomPipe = getBottomRect();

        return (otherRect.right > topPipe.left && otherRect.left < topPipe.right &&
            otherRect.bottom > topPipe.top && otherRect.top < topPipe.bottom) ||
            (otherRect.right > bottomPipe.left && otherRect.left < bottomPipe.right &&
                otherRect.bottom > bottomPipe.top && otherRect.top < bottomPipe.bottom);
    }

//4. 游戏引擎类（Game）
//负责调度全局逻辑、渲染分发、输入处理和数据存档。
class Game {
private:
    GameState currentState; // 当前状态（菜单、运行等）
    vector<Pipe> pipes;     // 障碍物容器
    vector<Particle> particles; // 粒子容器
    Bird bird;
    vector<Cloud> clouds;
    vector<ScoreEntry> leaderboard;

    int score;
    int highScore;
    int coins;
    int level;
    float gameSpeed;
    float pipeTimer;
    float gameTime;
    int pipesPassed;
    int nextPipeID;

    string playerName;
    int selectedMenu;
    int selectedSetting;

    float animationTime;
    float shakeTime;
    float shakeIntensity;

    float birdGravity;
    float birdJumpForce;
    bool showFPS;
    bool showHitboxes;
    int difficulty;

    bool keys[256];
    bool keyPressed[256];
    // ... 属性：score, level, gameSpeed, shakeTime (屏幕震动) ...


public:
    Game() {
        init();
    }
    void init() {
        currentState = STATE_MENU;
        score = 0;
        highScore = 0;
        coins = 0;
        level = 1;
        gameSpeed = 3.0f;
        pipeTimer = 0;
        gameTime = 0;
        pipesPassed = 0;
        nextPipeID = 0;

        playerName = "Player";
        selectedMenu = 0;
        selectedSetting = 0;

        animationTime = 0;
        shakeTime = 0;
        shakeIntensity = 0;

        birdGravity = 0.5f;
        birdJumpForce = -8.5f;
        showFPS = true;
        showHitboxes = false;
        difficulty = 1;

        memset(keys, 0, sizeof(keys));
        memset(keyPressed, 0, sizeof(keyPressed));

        bird.reset();
        bird.setGravity(birdGravity);
        bird.setJumpForce(birdJumpForce);

        pipes.clear();
        particles.clear();
        clouds.clear();

        for (int i = 0; i < 8; i++) {
            clouds.push_back(Cloud());
        }

        loadLeaderboard();

        if (!leaderboard.empty()) {
            highScore = leaderboard[0].score;
        }
    }

    void loadLeaderboard() {
        leaderboard.clear();

        ifstream file("leaderboard.dat");
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string name;
                int score, level, playTime;
                time_t date;

                ss >> name >> score >> level >> playTime >> date;
                if (!name.empty() && !ss.fail()) {
                    ScoreEntry entry(name, score, level, playTime);
                    entry.date = date;
                    leaderboard.push_back(entry);
                }
            }
            file.close();

            sort(leaderboard.begin(), leaderboard.end());
        }
    }

    void saveLeaderboard() {
        ofstream file("leaderboard.dat");
        if (file.is_open()) {
            for (size_t i = 0; i < leaderboard.size() && i < 10; i++) {
                file << leaderboard[i].playerName << " "
                    << leaderboard[i].score << " "
                    << leaderboard[i].level << " "
                    << leaderboard[i].playTime << " "
                    << leaderboard[i].date << endl;
            }
            file.close();
        }
    }

    void addToLeaderboard() {
        ScoreEntry entry(playerName, score, level, (int)gameTime);
        leaderboard.push_back(entry);
        sort(leaderboard.begin(), leaderboard.end());

        if (leaderboard.size() > 10) {
            leaderboard.resize(10);
        }

        saveLeaderboard();

        if (score > highScore) {
            highScore = score;
        }
    }

    void updateInput() {
        for (int i = 0; i < 256; i++) {
            bool currentKeyState = GetAsyncKeyState(i) & 0x8000;
            keyPressed[i] = currentKeyState && !keys[i];
            keys[i] = currentKeyState;
        }

        handleInput();
    }

    void handleInput() {
        switch (currentState) {
        case STATE_MENU:
            handleMenuInput();
            break;
        case STATE_PLAYING:
            handleGameInput();
            break;
        case STATE_PAUSED:
            handlePauseInput();
            break;
        case STATE_GAME_OVER:
            handleGameOverInput();
            break;
        case STATE_LEADERBOARD:
            handleLeaderboardInput();
            break;
        case STATE_SETTINGS:
            handleSettingsInput();
            break;
        case STATE_HELP:
            handleHelpInput();
            break;
        case STATE_CREDITS:
            handleCreditsInput();
            break;
        }
    }

    void handleMenuInput() {
        if (keyPressed[VK_UP]) {
            selectedMenu = (selectedMenu - 1 + 7) % 7;
        }
        if (keyPressed[VK_DOWN]) {
            selectedMenu = (selectedMenu + 1) % 7;
        }
        if (keyPressed[VK_RETURN] || keyPressed[VK_SPACE]) {
            switch (selectedMenu) {
            case 0:
                startNewGame();
                break;
            case 1:
                if (score > 0) {
                    currentState = STATE_PLAYING;
                }
                break;
            case 2:
                currentState = STATE_LEADERBOARD;
                break;
            case 3:
                currentState = STATE_SETTINGS;
                selectedSetting = 0;
                break;
            case 4:
                currentState = STATE_HELP;
                break;
            case 5:
                currentState = STATE_CREDITS;
                break;
            case 6:
                closegraph();
                exit(0);
                break;
            }
        }
    }

    void handleGameInput() {
        if (keyPressed[VK_SPACE] || keyPressed[VK_UP]) {
            bird.jump();
            createParticles(bird.getX(), bird.getY(), 8, RGB(255, 255, 0), 1);
        }
        if (keyPressed[VK_ESCAPE]) {
            currentState = STATE_PAUSED;
        }
        if (keyPressed['R'] || keyPressed['r']) {
            startNewGame();
        }
    }

    void handlePauseInput() {
        if (keyPressed[VK_ESCAPE] || keyPressed[VK_SPACE]) {
            currentState = STATE_PLAYING;
        }
        if (keyPressed['M'] || keyPressed['m']) {
            currentState = STATE_MENU;
        }
        if (keyPressed['R'] || keyPressed['r']) {
            startNewGame();
        }
    }

    void handleGameOverInput() {
        if (keyPressed[VK_SPACE] || keyPressed[VK_RETURN]) {
            startNewGame();
        }
        if (keyPressed[VK_ESCAPE]) {
            currentState = STATE_MENU;
        }
    }

    void handleLeaderboardInput() {
        if (keyPressed[VK_ESCAPE] || keyPressed[VK_SPACE]) {
            currentState = STATE_MENU;
        }
    }

    void handleSettingsInput() {
        if (keyPressed[VK_ESCAPE]) {
            currentState = STATE_MENU;
            bird.setGravity(birdGravity);
            bird.setJumpForce(birdJumpForce);
        }
        if (keyPressed[VK_UP]) {
            selectedSetting = (selectedSetting - 1 + 5) % 5;
        }
        if (keyPressed[VK_DOWN]) {
            selectedSetting = (selectedSetting + 1) % 5;
        }
        if (keyPressed[VK_LEFT]) {
            adjustSetting(-1);
        }
        if (keyPressed[VK_RIGHT]) {
            adjustSetting(1);
        }
    }

    void adjustSetting(int direction) {
        switch (selectedSetting) {
        case 0:
            birdGravity += direction * 0.1f;
            if (birdGravity < 0.2f) birdGravity = 0.2f;
            if (birdGravity > 1.5f) birdGravity = 1.5f;
            break;
        case 1:
            birdJumpForce += direction * 0.5f;
            if (birdJumpForce > -5.0f) birdJumpForce = -5.0f;
            if (birdJumpForce < -12.0f) birdJumpForce = -12.0f;
            break;
        case 2:
            difficulty = (difficulty + direction + 3) % 3;
            applyDifficulty();
            break;
        case 3:
            showFPS = !showFPS;
            break;
        case 4:
            showHitboxes = !showHitboxes;
            break;
        }
    }

    void applyDifficulty() {
        switch (difficulty) {
        case 0:
            birdGravity = 0.4f;
            birdJumpForce = -7.5f;
            gameSpeed = 2.5f;
            break;
        case 1:
            birdGravity = 0.5f;
            birdJumpForce = -8.5f;
            gameSpeed = 3.0f;
            break;
        case 2:
            birdGravity = 0.6f;
            birdJumpForce = -9.5f;
            gameSpeed = 3.5f;
            break;
        }
        bird.setGravity(birdGravity);
        bird.setJumpForce(birdJumpForce);
    }

    void handleHelpInput() {
        if (keyPressed[VK_ESCAPE] || keyPressed[VK_SPACE]) {
            currentState = STATE_MENU;
        }
    }

    void handleCreditsInput() {
        if (keyPressed[VK_ESCAPE] || keyPressed[VK_SPACE]) {
            currentState = STATE_MENU;
        }
    }

    void update(float deltaTime) {
        animationTime += deltaTime;

        if (shakeTime > 0) {
            shakeTime -= deltaTime;
            shakeIntensity *= 0.9f;
        }

        for (auto& cloud : clouds) {
            cloud.update(deltaTime);
        }

        updateParticles(deltaTime);

        switch (currentState) {
        case STATE_PLAYING:
            updateGameplay(deltaTime);
            break;
        default:
            break;
        }
    }


    // 游戏主逻辑循环调用
    void updateGameplay(float deltaTime) {
        gameTime += deltaTime;

        bird.update(deltaTime);

        if (bird.getY() >= SCREEN_HEIGHT - GROUND_HEIGHT - 15) {
            gameOver();
            return;
        }

        for (auto& pipe : pipes) {
            pipe.update(gameSpeed);

            if (pipe.checkCollision(bird.getCollisionRect())) {
                gameOver();
                return;
            }

            if (pipe.checkCoinCollision(bird.getCollisionRect())) {
                pipe.collectCoin();
                coins += 10;
                score += 5;
                createParticles(bird.getX(), bird.getY(), 15, RGB(255, 215, 0), 1);
                shakeScreen(5.0f);
            }

            if (!pipe.isPassed() && pipe.getX() + 70 < bird.getX()) {
                pipe.markPassed();
                pipesPassed++;
                score += 1 * bird.getScoreMultiplier();

                bird.addCombo();

                createParticles(pipe.getX() + 35, pipe.getGapY(), 20, RGB(100, 255, 100), 0);

                if (pipesPassed % 5 == 0) {
                    level++;
                    gameSpeed += 0.2f;
                    shakeScreen(3.0f);
                }

                if (score > highScore) {
                    highScore = score;
                }
            }
        }


        auto it = pipes.begin();
        while (it != pipes.end()) {
            if (it->isOffScreen()) {
                it = pipes.erase(it);
            }
            else {
                ++it;
            }
        }

        pipeTimer += deltaTime;
        float spawnInterval = max(1.5f, 3.0f - level * 0.1f);
        if (pipeTimer > spawnInterval) {
            pipeTimer = 0;
            pipes.push_back(Pipe((float)SCREEN_WIDTH, nextPipeID++));
        }
    }

    void createParticles(float x, float y, int count, COLORREF color, int type) {
        for (int i = 0; i < count; i++) {
            particles.push_back(Particle(x, y, color, type));
        }
    }

    void updateParticles(float deltaTime) {
        for (auto& particle : particles) {
            particle.update(deltaTime);
        }

        auto it = particles.begin();
        while (it != particles.end()) {
            if (it->life <= 0) {
                it = particles.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void shakeScreen(float intensity) {
        shakeTime = 0.3f;
        shakeIntensity = intensity;
    }

    void startNewGame() {
        bird.reset();
        pipes.clear();
        particles.clear();

        score = 0;
        coins = 0;
        level = 1;
        gameSpeed = 3.0f;
        pipeTimer = 0;
        gameTime = 0;
        pipesPassed = 0;
        nextPipeID = 0;

        applyDifficulty();

        currentState = STATE_PLAYING;
    }

    void gameOver() {
        bird.kill();
        createParticles(bird.getX(), bird.getY(), 50, RGB(255, 50, 50), 2);
        shakeScreen(10.0f);
        addToLeaderboard();
        currentState = STATE_GAME_OVER;
    }

    void render() {
        BeginBatchDraw();

        int shakeX = 0, shakeY = 0;
        if (shakeTime > 0) {
            shakeX = (rand() % (int)(shakeIntensity * 2)) - (int)shakeIntensity;
            shakeY = (rand() % (int)(shakeIntensity * 2)) - (int)shakeIntensity;
        }

        cleardevice();

        drawSkyBackground();

        for (const auto& cloud : clouds) {
            cloud.draw();
        }

        drawGround();

        if (currentState == STATE_PLAYING ||
            currentState == STATE_PAUSED ||
            currentState == STATE_GAME_OVER) {

            for (const auto& pipe : pipes) {
                pipe.draw();
            }

            for (const auto& particle : particles) {
                particle.draw();
            }

            bird.draw();

            if (showHitboxes) {
                drawHitboxes();
            }

            drawGameUI();
        }

        switch (currentState) {
        case STATE_MENU:
            drawMenu();
            break;
        case STATE_PAUSED:
            drawPauseMenu();
            break;
        case STATE_GAME_OVER:
            drawGameOver();
            break;
        case STATE_LEADERBOARD:
            drawLeaderboard();
            break;
        case STATE_SETTINGS:
            drawSettings();
            break;
        case STATE_HELP:
            drawHelp();
            break;
        case STATE_CREDITS:
            drawCredits();
            break;
        }

        if (showFPS) {
            drawFPS();
        }

        if (shakeTime > 0) {
            drawShakeEffect(shakeX, shakeY);
        }

        FlushBatchDraw();
    }

    void drawSkyBackground() {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            float ratio = (float)y / SCREEN_HEIGHT;
            int r = (int)(GetRValue(COLOR_SKY_START) * (1 - ratio) + GetRValue(COLOR_SKY_END) * ratio);
            int g = (int)(GetGValue(COLOR_SKY_START) * (1 - ratio) + GetGValue(COLOR_SKY_END) * ratio);
            int b = (int)(GetBValue(COLOR_SKY_START) * (1 - ratio) + GetBValue(COLOR_SKY_END) * ratio);

            setlinecolor(RGB(r, g, b));
            line(0, y, SCREEN_WIDTH, y);
        }

        setfillcolor(RGB(255, 255, 0));
        setlinecolor(RGB(255, 200, 0));
        solidcircle(SCREEN_WIDTH - 80, 80, 40);

        setlinecolor(RGB(255, 255, 0));
        for (int i = 1; i <= 3; i++) {
            int radius = 40 + i * 10;
            circle(SCREEN_WIDTH - 80, 80, radius);
        }
    }

    void drawGround() {
        setfillcolor(COLOR_GROUND);
        fillrectangle(0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);

        setfillcolor(COLOR_GRASS);
        for (int x = 0; x < SCREEN_WIDTH; x += 20) {
            int height = 5 + rand() % 15;
            fillrectangle(x, SCREEN_HEIGHT - GROUND_HEIGHT - height,
                x + 15, SCREEN_HEIGHT - GROUND_HEIGHT);
        }

        setfillcolor(RGB(139, 69, 19));
        for (int x = 0; x < SCREEN_WIDTH; x += 40) {
            fillrectangle(x, SCREEN_HEIGHT - GROUND_HEIGHT,
                x + 20, SCREEN_HEIGHT - GROUND_HEIGHT + 10);
        }
    }

    void drawGameUI() {
        settextstyle(36, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_WHITE);
        setbkmode(TRANSPARENT);

        wchar_t wbuffer[100];
        swprintf_s(wbuffer, 100, L"%d", score);
        int scoreWidth = textwidth(wbuffer);
        outtextxy(SCREEN_WIDTH / 2 - scoreWidth / 2, 30, wbuffer);

        if (bird.getComboCount() > 0) {
            settextstyle(24, 0, _T("Arial"));
            settextcolor(RGB(255, 215, 0));
            swprintf_s(wbuffer, 100, L"COMBO x%d", bird.getComboCount());
            outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 75, wbuffer);
        }

        settextstyle(18, 0, _T("Arial"));
        settextcolor(RGB(200, 200, 255));

        swprintf_s(wbuffer, 100, L"Level: %d", level);
        outtextxy(20, 20, wbuffer);

        swprintf_s(wbuffer, 100, L"Coins: %d", coins);
        outtextxy(20, 50, wbuffer);

        swprintf_s(wbuffer, 100, L"Speed: %.1f", gameSpeed);
        outtextxy(20, 80, wbuffer);

        int minutes = (int)gameTime / 60;
        int seconds = (int)gameTime % 60;
        swprintf_s(wbuffer, 100, L"Time: %02d:%02d", minutes, seconds);
        outtextxy(20, 110, wbuffer);

        swprintf_s(wbuffer, 100, L"Best: %d", highScore);
        outtextxy(20, 140, wbuffer);

        settextcolor(RGB(255, 200, 255));
        wchar_t wname[100];
        size_t converted = 0;
        mbstowcs_s(&converted, wname, playerName.c_str(), 100);
        swprintf_s(wbuffer, 100, L"Player: %s", wname);
        outtextxy(SCREEN_WIDTH - textwidth(wbuffer) - 20, 20, wbuffer);

        if (currentState == STATE_PLAYING) {
            settextcolor(RGB(150, 150, 150));
            settextstyle(14, 0, _T("Arial"));
            outtextxy(20, SCREEN_HEIGHT - 40, L"SPACE: Jump  ESC: Pause  R: Restart");
        }
    }

    void drawHitboxes() {
        setlinecolor(RGB(255, 0, 0));
        setlinestyle(PS_DASH, 1);

        RECT birdRect = bird.getCollisionRect();
        rectangle(birdRect.left, birdRect.top, birdRect.right, birdRect.bottom);

        for (const auto& pipe : pipes) {
            RECT topRect = pipe.getTopRect();
            RECT bottomRect = pipe.getBottomRect();
            rectangle(topRect.left, topRect.top, topRect.right, topRect.bottom);
            rectangle(bottomRect.left, bottomRect.top, bottomRect.right, bottomRect.bottom);
        }

        setlinestyle(PS_SOLID, 1);
    }

    void drawFPS() {
        static DWORD lastTime = GetTickCount();
        static int frameCount = 0;
        static float fps = 0;

        frameCount++;
        DWORD currentTime = GetTickCount();
        if (currentTime - lastTime >= 1000) {
            fps = frameCount * 1000.0f / (currentTime - lastTime);
            frameCount = 0;
            lastTime = currentTime;
        }

        wchar_t wbuffer[20];
        swprintf_s(wbuffer, 20, L"FPS: %.1f", fps);

        settextstyle(12, 0, _T("Arial"));
        settextcolor(RGB(150, 150, 150));
        setbkmode(TRANSPARENT);
        outtextxy(SCREEN_WIDTH - 70, SCREEN_HEIGHT - 20, wbuffer);
    }

    void drawShakeEffect(int shakeX, int shakeY) {
        setlinecolor(RGB(255, 255, 255));
        for (int i = 0; i < 5; i++) {
            int offset = i * 2;
            rectangle(offset + shakeX, offset + shakeY,
                SCREEN_WIDTH - offset + shakeX, SCREEN_HEIGHT - offset + shakeY);
        }
    }

    void drawMenu() {
        setfillcolor(BLACK);
        fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        settextstyle(72, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_YELLOW);

        const wchar_t* title = L"FLAPPY BIRD";
        int titleWidth = textwidth(title);
        outtextxy(SCREEN_WIDTH / 2 - titleWidth / 2, 80, title);

        settextstyle(28, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_BLUE);
        outtextxy(SCREEN_WIDTH / 2 - 120, 160, L"ULTIMATE EDITION");

        const wchar_t* menuItems[] = {
            L"START NEW GAME",
            L"CONTINUE",
            L"LEADERBOARD",
            L"SETTINGS",
            L"HELP",
            L"CREDITS",
            L"EXIT"
        };

        settextstyle(28, 0, _T("Arial"));

        for (int i = 0; i < 7; i++) {
            int y = 220 + i * 45;

            if (i == selectedMenu) {
                settextcolor(COLOR_TEXT_RED);
                settextstyle(32, 0, _T("Arial"));
            }
            else {
                settextcolor(COLOR_TEXT_WHITE);
                settextstyle(28, 0, _T("Arial"));
            }

            int itemWidth = textwidth(menuItems[i]);
            outtextxy(SCREEN_WIDTH / 2 - itemWidth / 2, y, menuItems[i]);

            if (i == selectedMenu) {
                setfillcolor(COLOR_TEXT_RED);
                solidcircle(SCREEN_WIDTH / 2 - itemWidth / 2 - 25, y + 15, 8);
                solidcircle(SCREEN_WIDTH / 2 + itemWidth / 2 + 25, y + 15, 8);
            }
        }

        settextstyle(14, 0, _T("Arial"));
        settextcolor(RGB(150, 150, 150));
        outtextxy(10, SCREEN_HEIGHT - 20, L"Version 2.0 | Ultimate Edition");

        outtextxy(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT - 60,
            L"Use ARROW KEYS to navigate, ENTER to select");
    }

    void drawPauseMenu() {
        setfillcolor(BLACK);
        fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        settextstyle(64, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_YELLOW);
        outtextxy(SCREEN_WIDTH / 2 - 120, 150, L"PAUSED");

        settextstyle(24, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_WHITE);

        wchar_t wbuffer[100];

        swprintf_s(wbuffer, 100, L"Score: %d", score);
        outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 240, wbuffer);

        swprintf_s(wbuffer, 100, L"Level: %d", level);
        outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 280, wbuffer);

        settextstyle(20, 0, _T("Arial"));
        settextcolor(RGB(200, 200, 255));

        outtextxy(SCREEN_WIDTH / 2 - 150, 350, L"Press ESC or SPACE to continue");
        outtextxy(SCREEN_WIDTH / 2 - 100, 380, L"Press R to restart");
        outtextxy(SCREEN_WIDTH / 2 - 80, 410, L"Press M for menu");
    }

    void drawGameOver() {
        setfillcolor(BLACK);
        fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        settextstyle(64, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_RED);
        outtextxy(SCREEN_WIDTH / 2 - 180, 100, L"GAME OVER");

        settextstyle(36, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_WHITE);

        wchar_t wbuffer[100];

        swprintf_s(wbuffer, 100, L"Final Score: %d", score);
        outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 200, wbuffer);

        settextstyle(24, 0, _T("Arial"));

        swprintf_s(wbuffer, 100, L"Level Reached: %d", level);
        outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 250, wbuffer);

        swprintf_s(wbuffer, 100, L"Coins Collected: %d", coins);
        outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 280, wbuffer);

        int minutes = (int)gameTime / 60;
        int seconds = (int)gameTime % 60;
        swprintf_s(wbuffer, 100, L"Play Time: %02d:%02d", minutes, seconds);
        outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 310, wbuffer);

        if (score == highScore && score > 0) {
            settextstyle(32, 0, _T("Arial"));
            settextcolor(RGB(255, 215, 0));
            outtextxy(SCREEN_WIDTH / 2 - 100, 350, L"NEW HIGH SCORE!");

            setfillcolor(RGB(255, 215, 0));
            for (int i = 0; i < 5; i++) {
                float angle = animationTime * 2 + i * 1.256f;
                int starX = SCREEN_WIDTH / 2 - 150 + i * 75;
                int starY = 400 + (int)(sin(angle) * 10);
                int starSize = 10 + (int)(sin(animationTime * 3 + i) * 5);
                solidcircle(starX, starY, starSize);
            }
        }

        settextstyle(20, 0, _T("Arial"));
        settextcolor(RGB(200, 200, 255));
        outtextxy(SCREEN_WIDTH / 2 - 150, 450, L"Press SPACE to play again");
        outtextxy(SCREEN_WIDTH / 2 - 120, 480, L"Press ESC to return to menu");
    }

    void drawLeaderboard() {
        setfillcolor(RGB(20, 25, 40));
        fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        settextstyle(48, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_YELLOW);
        outtextxy(SCREEN_WIDTH / 2 - 100, 40, L"LEADERBOARD");

        settextstyle(20, 0, _T("Arial"));
        settextcolor(RGB(180, 180, 255));

        outtextxy(100, 110, L"Rank");
        outtextxy(180, 110, L"Player");
        outtextxy(350, 110, L"Score");
        outtextxy(450, 110, L"Level");
        outtextxy(550, 110, L"Time");
        outtextxy(650, 110, L"Date");

        setlinecolor(RGB(100, 100, 150));
        line(80, 140, 720, 140);

        settextstyle(18, 0, _T("Arial"));

        for (int i = 0; i < min(10, (int)leaderboard.size()); i++) {
            int y = 160 + i * 35;

            if (i % 2 == 0) {
                setfillcolor(RGB(40, 45, 70));
            }
            else {
                setfillcolor(RGB(30, 35, 60));
            }
            fillrectangle(80, y - 5, 720, y + 30);

            if (i == 0) settextcolor(RGB(255, 215, 0));
            else if (i == 1) settextcolor(RGB(192, 192, 192));
            else if (i == 2) settextcolor(RGB(205, 127, 50));
            else settextcolor(RGB(200, 200, 255));

            wchar_t wbuffer[50];
            swprintf_s(wbuffer, 50, L"%d.", i + 1);
            outtextxy(100, y, wbuffer);

            settextcolor(RGB(100, 255, 100));
            wchar_t wname[50];
            size_t converted = 0;
            mbstowcs_s(&converted, wname, leaderboard[i].playerName.c_str(), 50);
            outtextxy(180, y, wname);

            settextcolor(COLOR_TEXT_WHITE);
            swprintf_s(wbuffer, 50, L"%d", leaderboard[i].score);
            outtextxy(350, y, wbuffer);

            swprintf_s(wbuffer, 50, L"%d", leaderboard[i].level);
            outtextxy(450, y, wbuffer);

            int minutes = leaderboard[i].playTime / 60;
            int seconds = leaderboard[i].playTime % 60;
            swprintf_s(wbuffer, 50, L"%02d:%02d", minutes, seconds);
            outtextxy(550, y, wbuffer);

            tm timeinfo;
            localtime_s(&timeinfo, &leaderboard[i].date);
            swprintf_s(wbuffer, 50, L"%02d/%02d", timeinfo.tm_mon + 1, timeinfo.tm_mday);
            outtextxy(650, y, wbuffer);
        }

        settextstyle(18, 0, _T("Arial"));
        settextcolor(RGB(150, 150, 200));
        outtextxy(SCREEN_WIDTH / 2 - 100, 550, L"Press ESC to return to menu");
    }

    void drawSettings() {
        setfillcolor(RGB(30, 35, 50));
        fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        settextstyle(48, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_BLUE);
        outtextxy(SCREEN_WIDTH / 2 - 100, 40, L"SETTINGS");

        const wchar_t* settingNames[] = {
            L"GRAVITY",
            L"JUMP FORCE",
            L"DIFFICULTY",
            L"SHOW FPS",
            L"SHOW HITBOXES"
        };

        const wchar_t* difficultyNames[] = { L"EASY", L"NORMAL", L"HARD" };

        settextstyle(24, 0, _T("Arial"));

        for (int i = 0; i < 5; i++) {
            int y = 120 + i * 70;

            if (i == selectedSetting) {
                settextcolor(COLOR_TEXT_RED);
            }
            else {
                settextcolor(COLOR_TEXT_WHITE);
            }
            outtextxy(150, y, settingNames[i]);

            settextcolor(RGB(200, 200, 255));
            wchar_t wbuffer[50];

            switch (i) {
            case 0:
                swprintf_s(wbuffer, 50, L"%.1f", birdGravity);
                break;
            case 1:
                swprintf_s(wbuffer, 50, L"%.1f", fabs(birdJumpForce));
                break;
            case 2:
                swprintf_s(wbuffer, 50, L"%s", difficultyNames[difficulty]);
                break;
            case 3:
                swprintf_s(wbuffer, 50, L"%s", showFPS ? L"ON" : L"OFF");
                break;
            case 4:
                swprintf_s(wbuffer, 50, L"%s", showHitboxes ? L"ON" : L"OFF");
                break;
            }

            outtextxy(500, y, wbuffer);

            if (i < 3) {
                drawProgressBar(300, y + 10, 150, 20, i);
            }

            if (i == selectedSetting) {
                setfillcolor(COLOR_TEXT_RED);
                solidcircle(120, y + 15, 8);
            }
        }

        settextstyle(18, 0, _T("Arial"));
        settextcolor(RGB(150, 150, 200));
        outtextxy(SCREEN_WIDTH / 2 - 200, 500, L"Use ARROW KEYS to navigate and adjust values");
        outtextxy(SCREEN_WIDTH / 2 - 120, 530, L"Press ESC to save and return");
    }

    void drawProgressBar(int x, int y, int width, int height, int type) {
        setfillcolor(RGB(60, 60, 80));
        fillrectangle(x, y, x + width, y + height);

        float value = 0;
        COLORREF fillColor;

        switch (type) {
        case 0:
            value = (birdGravity - 0.2f) / 1.3f;
            fillColor = RGB(0, 200, 255);
            break;
        case 1:
            value = (fabs(birdJumpForce) - 5.0f) / 7.0f;
            fillColor = RGB(255, 100, 0);
            break;
        case 2:
            value = difficulty / 2.0f;
            fillColor = RGB(255, 50, 50);
            break;
        }

        setfillcolor(fillColor);
        fillrectangle(x, y, x + (int)(width * value), y + height);

        setlinecolor(RGB(100, 100, 120));
        rectangle(x, y, x + width, y + height);
    }

    void drawHelp() {
        setfillcolor(RGB(25, 40, 30));
        fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        settextstyle(48, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_GREEN);
        outtextxy(SCREEN_WIDTH / 2 - 80, 40, L"HELP");

        settextstyle(20, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_WHITE);

        const wchar_t* helpLines[] = {
            L"GAME OBJECTIVE:",
            L"  Guide the bird through the pipes without hitting them.",
            L"  Each pipe passed gives you 1 point.",
            L"  Collect coins for bonus points.",
            L"",
            L"CONTROLS:",
            L"  SPACE / UP ARROW - Make the bird jump",
            L"  ESC - Pause game / Return to menu",
            L"  R - Restart game",
            L"  M - Return to main menu (when paused)",
            L"",
            L"GAME FEATURES:",
            L"  Combo System: Pass pipes quickly for score multiplier",
            L"  Level System: Game speed increases every 5 pipes",
            L"  Coin Collection: Collect coins for extra points",
            L"  Difficulty Settings: Adjust game physics in settings",
            L"",
            L"TIPS:",
            L"  Tap lightly for small hops, hold for higher jumps",
            L"  Try to stay in the middle of pipe gaps",
            L"  Watch out for increasing speed at higher levels"
        };

        int numLines = sizeof(helpLines) / sizeof(helpLines[0]);
        for (int i = 0; i < numLines; i++) {
            outtextxy(100, 100 + i * 25, helpLines[i]);
        }

        settextstyle(18, 0, _T("Arial"));
        settextcolor(RGB(150, 200, 150));
        outtextxy(SCREEN_WIDTH / 2 - 120, 550, L"Press ESC to return to menu");
    }

    void drawCredits() {
        setfillcolor(RGB(40, 30, 50));
        fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        settextstyle(48, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_PURPLE);
        outtextxy(SCREEN_WIDTH / 2 - 120, 40, L"CREDITS");

        settextstyle(28, 0, _T("Arial"));
        settextcolor(RGB(255, 200, 255));
        outtextxy(SCREEN_WIDTH / 2 - 150, 120, L"FLAPPY BIRD ULTIMATE EDITION");

        settextstyle(22, 0, _T("Arial"));
        settextcolor(COLOR_TEXT_WHITE);

        const wchar_t* credits[] = {
            L"Game Design & Programming:",
            L" 羊能嘉，陈泓宇，侯美旭，郗晓祎",
            L"",
            L"Graphics & Animation:",
            L"  EasyX Graphics Library",
            L"  Custom Particle System",
            L"",
            L"Special Thanks To:",
            L"  Original Flappy Bird Creator",
            L"  All Beta Testers",
            L"  Open Source Community",
            L"",
            L"Version: 2.0 Ultimate",
            L"Release Date: 2024",
        };

        int numCredits = sizeof(credits) / sizeof(credits[0]);
        for (int i = 0; i < numCredits; i++) {
            int y = 180 + i * 30;
            outtextxy(SCREEN_WIDTH / 2 - textwidth(credits[i]) / 2, y, credits[i]);
        }

        float pulse = sin(animationTime * 2) * 0.5f + 0.5f;
        setfillcolor(RGB(255, 0, 0));
        solidcircle(SCREEN_WIDTH / 2, 500, 20 + (int)(pulse * 10));

        settextstyle(18, 0, _T("Arial"));
        settextcolor(RGB(200, 150, 200));
        outtextxy(SCREEN_WIDTH / 2 - 120, 550, L"Press ESC to return to menu");
    }

///5. 运行入口与高性能计时器
///run() 函数中的计时逻辑是游戏流畅运行的关键。
void run() {
    initgraph(SCREEN_WIDTH, SCREEN_HEIGHT);
    setbkcolor(BLACK);

    srand((unsigned)time(NULL));

    LARGE_INTEGER frequency;
    LARGE_INTEGER lastTime, currentTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&lastTime);

    const double frameInterval = 1.0 / FPS;
    double accumulator = 0.0;

    while (true) {
        QueryPerformanceCounter(&currentTime);

        double elapsedTime = (double)(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
        lastTime = currentTime;

        if (elapsedTime > 0.25) elapsedTime = 0.25;

        accumulator += elapsedTime;

        updateInput();

        while (accumulator >= frameInterval) {
            update((float)frameInterval);
            accumulator -= frameInterval;
        }

        render();

        double frameTime = (double)(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
        double sleepTime = frameInterval - frameTime;

        if (sleepTime > 0) {
            DWORD sleepMs = (DWORD)(sleepTime * 1000.0);
            if (sleepMs > 0) {
                Sleep(sleepMs);
            }
            else {
                LONGLONG endTime = currentTime.QuadPart + (LONGLONG)(sleepTime * frequency.QuadPart);
                LARGE_INTEGER now;
                do {
                    QueryPerformanceCounter(&now);
                } while (now.QuadPart < endTime);
            }
        }
    }

        closegraph();
    }
};

int main() {
    system("chcp 65001 > nul");

    cout << "=============================================" << endl;
    cout << "      FLAPPY BIRD ULTIMATE EDITION v2.0" << endl;
    cout << "=============================================" << endl;
    cout << "Features:" << endl;
    cout << "  • Enhanced Graphics & Animations" << endl;
    cout << "  • Particle Effects System" << endl;
    cout << "  • Combo Multiplier System" << endl;
    cout << "  • Coin Collection Mechanic" << endl;
    cout << "  • Multiple Difficulty Levels" << endl;
    cout << "  • Screen Shake Effects" << endl;
    cout << "  • Persistent Leaderboard" << endl;
    cout << "  • Customizable Settings" << endl;
    cout << "  • Dynamic Sky & Cloud System" << endl;
    cout << "=============================================" << endl;
    cout << "Starting game..." << endl;

    Game game;
    game.run();

    return 0;

}