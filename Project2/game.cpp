#include "game.h"          // 包含游戏主头文件，定义游戏类和常量
#include "bird.h"          // 包含小鸟类头文件
#include "pipemanager.h"   // 包含管道管理器头文件
#include <cmath>

// ============================================================
// Particle类方法的实现
// ============================================================

// 粒子构造函数：初始化粒子的所有属性
Particle::Particle(float px, float py, COLORREF col, int t) {
    x = px;                     // 设置粒子的X坐标
    y = py;                     // 设置粒子的Y坐标
    // 生成随机的水平速度（-2到2之间）
    vx = (rand() % 200 - 100) / 50.0f;
    // 生成随机的垂直速度（-3到-1之间）
    vy = (rand() % 200 - 150) / 50.0f;
    // 生成随机的生命周期（1到2秒之间）
    life = 1.0f + (rand() % 100) / 100.0f;
    maxLife = life;             // 记录最大生命周期
    color = col;                // 设置粒子颜色
    size = 2 + rand() % 5;      // 设置粒子大小（2到6像素）
    type = t;                   // 设置粒子类型（0:圆形，1:星星，2:线条）
}

// 粒子更新方法：根据时间更新粒子状态
void Particle::update(float deltaTime) {
    x += vx * deltaTime * 60;   // 更新X位置（乘以60以匹配游戏速度）
    y += vy * deltaTime * 60;   // 更新Y位置
    vy += 0.3f * deltaTime * 60; // 应用重力效果（垂直速度增加）
    life -= deltaTime;          // 减少生命周期
}

// 粒子绘制方法：根据粒子类型绘制粒子
void Particle::draw() const {
    if (getLife() <= 0) return;      // 使用公有方法访问

    // 根据生命周期计算透明度（alpha值）
    float alpha = getLife() / getMaxLife();  // 使用公有方法
    // 根据透明度计算实际颜色值
    int r = (int)(GetRValue(color) * alpha);
    int g = (int)(GetGValue(color) * alpha);
    int b = (int)(GetBValue(color) * alpha);

    // 设置粒子的填充颜色和边框颜色
    setfillcolor(RGB(r, g, b));
    setlinecolor(RGB(r, g, b));

    // 根据粒子类型选择绘制方式
    if (type == 0) {
        // 类型0：绘制实心圆
        solidcircle((int)x, (int)y, size);
    }
    else if (type == 1) {
        // 类型1：绘制星星形状
        drawStar((int)x, (int)y, size);
    }
    else if (type == 2) {
        // 类型2：绘制线条（轨迹效果）
        setlinestyle(PS_SOLID, size); // 设置线条样式
        // 绘制从当前位置到速度方向的线条
        line((int)x, (int)y, (int)(x + vx * 2), (int)(y + vy * 2));
    }
}

// 绘制星星形状的辅助方法
void Particle::drawStar(int cx, int cy, int radius) const {
    POINT points[10];  // 星星有10个顶点（5个外角，5个内角）

    // 计算星星的所有顶点坐标
    for (int i = 0; i < 10; i++) {
        float angle = 3.14159f * 2 * i / 10;  // 计算当前顶点的角度
        // 交替使用外半径和内半径（外半径为radius，内半径为radius/2）
        float r = (i % 2 == 0) ? radius : radius / 2;
        // 计算顶点坐标
        points[i].x = cx + (int)(r * cos(angle));  // X坐标 = 中心X + 半径*cos(角度)
        points[i].y = cy + (int)(r * sin(angle));  // Y坐标 = 中心Y + 半径*sin(角度)
    }

    // 使用多边形填充函数绘制星星
    solidpolygon(points, 10);
}

// ============================================================
// Cloud类方法的实现
// ============================================================

// Cloud构造函数：随机初始化云朵的属性
Cloud::Cloud() {
    x = rand() % SCREEN_WIDTH;          // 随机X坐标（0到屏幕宽度）
    y = rand() % 200;                   // 随机Y坐标（0到200，天空区域）
    // 随机速度（0.2到0.7像素/帧）
    speed = (rand() % 50 + 20) / 100.0f;
    size = 20 + rand() % 40;            // 随机大小（20到59像素）
    alpha = 150 + rand() % 100;         // 随机透明度（150到249）
}

// 云朵更新方法：根据时间移动云朵
void Cloud::update(float deltaTime) {
    x -= speed * deltaTime * 60;        // 向左移动云朵（乘以60匹配游戏速度）

    // 如果云朵完全移出屏幕左侧
    if (x < -100) {
        x = SCREEN_WIDTH + 100;         // 重置到屏幕右侧
        y = rand() % 200;               // 随机新的Y坐标
    }
}

// 云朵绘制方法：绘制云朵（多个圆形组合）
void Cloud::draw() const {
    setfillcolor(RGB(255, 255, 255));  // 设置填充颜色为白色
    setlinecolor(RGB(255, 255, 255));  // 设置边框颜色为白色

    // 绘制云朵的主体（4个重叠的圆形）
    solidcircle((int)x, (int)y, size);                         // 主圆形
    solidcircle((int)(x + size * 0.6), (int)(y - size * 0.3),  // 右上圆形
        (int)(size * 0.7));
    solidcircle((int)(x + size * 1.2), (int)y,                // 右圆形
        (int)(size * 0.5));
    solidcircle((int)(x - size * 0.4), (int)(y + size * 0.3), // 左下圆形
        (int)(size * 0.6));
}

// ============================================================
// Game类方法的实现
// ============================================================

// Game构造函数：初始化游戏对象指针为nullptr
Game::Game() : bird(nullptr), pipeManager(nullptr) {
    init();  // 调用初始化方法
}

// Game析构函数：释放动态分配的内存
Game::~Game() {
    delete bird;        // 释放小鸟对象内存
    delete pipeManager; // 释放管道管理器内存
}

// 游戏初始化方法：设置所有游戏变量和对象的初始状态
void Game::init() {
    currentState = STATE_MENU;  // 初始状态为主菜单

    // 初始化游戏数值变量
    score = 0;          // 当前分数
    highScore = 0;      // 最高分
    coins = 0;          // 硬币数量
    level = 1;          // 当前等级
    gameSpeed = 3.0f;   // 游戏基础速度
    pipeTimer = 0;      // 管道生成计时器
    gameTime = 0;       // 游戏总时间
    pipesPassed = 0;    // 通过的管道数量
    nextPipeID = 0;     // 下一个管道的ID

    // 初始化UI相关变量
    playerName = "Player";  // 默认玩家名称
    selectedMenu = 0;       // 菜单选择索引
    selectedSetting = 0;    // 设置选择索引

    // 初始化动画效果变量
    animationTime = 0;      // 动画时间累计
    shakeTime = 0;          // 屏幕震动剩余时间
    shakeIntensity = 0;     // 屏幕震动强度

    // 初始化游戏设置
    birdGravity = 0.5f;      // 小鸟重力系数
    birdJumpForce = -8.5f;   // 小鸟跳跃力量
    showFPS = true;          // 显示FPS
    showHitboxes = false;    // 显示碰撞框
    difficulty = 1;          // 难度等级（0:简单，1:普通，2:困难）

    // 初始化按键状态数组（全部设为false）
    memset(keys, 0, sizeof(keys));        // 当前帧按键状态
    memset(keyPressed, 0, sizeof(keyPressed)); // 按键按下瞬间状态

    // 创建游戏对象
    if (bird) delete bird;              // 如果已存在则先删除
    bird = new Bird();                  // 创建新的小鸟对象
    bird->setGravity(birdGravity);      // 设置小鸟重力
    bird->setJumpForce(birdJumpForce);  // 设置小鸟跳跃力量

    if (pipeManager) delete pipeManager; // 如果已存在则先删除
    pipeManager = new PipeManager();    // 创建新的管道管理器

    // 清空粒子效果和云朵
    particles.clear();  // 清空粒子数组
    clouds.clear();     // 清空云朵数组

    // 创建初始云朵（8朵）
    for (int i = 0; i < 8; i++) {
        clouds.push_back(Cloud());  // 添加新的云朵对象
    }

    // 加载排行榜数据
    loadLeaderboard();

    // 如果有排行榜数据，设置最高分
    if (!leaderboard.empty()) {
        highScore = leaderboard[0].score;  // 取最高分记录
    }
}

// 加载排行榜方法：从文件读取排行榜数据
void Game::loadLeaderboard() {
    leaderboard.clear();  // 清空当前排行榜

    std::ifstream file("leaderboard.dat");  // 打开排行榜文件
    if (file.is_open()) {  // 如果文件成功打开
        std::string line;  // 用于存储每行数据

        // 逐行读取文件
        while (std::getline(file, line)) {
            std::stringstream ss(line);  // 创建字符串流用于解析
            std::string name;            // 玩家名称
            int score, level, playTime; // 分数、等级、游戏时间
            time_t date;            // 日期时间戳

            // 从行中解析数据
            ss >> name >> score >> level >> playTime >> date;

            // 如果解析成功且名称不为空
            if (!name.empty() && !ss.fail()) {
                ScoreEntry entry(name, score, level, playTime);  // 创建分数记录
                entry.date = date;  // 设置日期
                leaderboard.push_back(entry);  // 添加到排行榜数组
            }
        }
        file.close();  // 关闭文件

        // 对排行榜进行排序（按分数降序，分数相同时按游戏时间升序）
        std::sort(leaderboard.begin(), leaderboard.end());
    }
}

// 保存排行榜方法：将排行榜数据写入文件
void Game::saveLeaderboard() {
    std::ofstream file("leaderboard.dat");  // 打开排行榜文件（输出模式）
    if (file.is_open()) {  // 如果文件成功打开
        // 保存前10名记录
        for (size_t i = 0; i < leaderboard.size() && i < 10; i++) {
            // 将记录写入文件（空格分隔）
            file << leaderboard[i].playerName << " "
                << leaderboard[i].score << " "
                << leaderboard[i].level << " "
                << leaderboard[i].playTime << " "
                << leaderboard[i].date << std::endl;  // 换行
        }
        file.close();  // 关闭文件
    }
}

// 添加到排行榜方法：将当前游戏记录添加到排行榜
void Game::addToLeaderboard() {
    // 创建当前游戏的分数记录
    ScoreEntry entry(playerName, score, level, (int)gameTime);
    leaderboard.push_back(entry);  // 添加到排行榜数组

    std::sort(leaderboard.begin(), leaderboard.end());  // 重新排序

    // 如果超过10条记录，只保留前10名
    if (leaderboard.size() > 10) {
        leaderboard.resize(10);  // 调整数组大小为10
    }

    saveLeaderboard();  // 保存到文件

    // 更新最高分
    if (score > highScore) {
        highScore = score;
    }
}

// 更新输入方法：检测键盘按键状态
void Game::updateInput() {
    // 遍历256个可能的按键
    for (int i = 0; i < 256; i++) {
        // 获取当前按键状态（GetAsyncKeyState返回按键状态）
        bool currentKeyState = GetAsyncKeyState(i) & 0x8000;
        // 按键按下瞬间：当前帧按下且上一帧未按下
        keyPressed[i] = currentKeyState && !keys[i];
        // 更新当前帧按键状态
        keys[i] = currentKeyState;
    }

    handleInput();  // 调用输入处理函数
}

// 处理输入方法：根据当前游戏状态调用对应的输入处理函数
void Game::handleInput() {
    switch (currentState) {  // 根据游戏状态选择处理函数
    case STATE_MENU:
        handleMenuInput();           // 主菜单输入处理
        break;
    case STATE_PLAYING:
        handleGameInput();           // 游戏中输入处理
        break;
    case STATE_PAUSED:
        handlePauseInput();          // 暂停状态输入处理
        break;
    case STATE_GAME_OVER:
        handleGameOverInput();       // 游戏结束输入处理
        break;
    case STATE_LEADERBOARD:
        handleLeaderboardInput();    // 排行榜输入处理
        break;
    case STATE_SETTINGS:
        handleSettingsInput();       // 设置界面输入处理
        break;
    case STATE_HELP:
        handleHelpInput();           // 帮助界面输入处理
        break;
    case STATE_CREDITS:
        handleCreditsInput();        // 制作人员界面输入处理
        break;
    }
}

// 主菜单输入处理方法：处理菜单导航和选择
void Game::handleMenuInput() {
    // 上方向键：菜单项向上移动
    if (keyPressed[VK_UP]) {
        selectedMenu = (selectedMenu - 1 + 7) % 7;  // 循环选择（7个菜单项）
    }
    // 下方向键：菜单项向下移动
    if (keyPressed[VK_DOWN]) {
        selectedMenu = (selectedMenu + 1) % 7;  // 循环选择
    }
    // 回车键或空格键：确认选择
    if (keyPressed[VK_RETURN] || keyPressed[VK_SPACE]) {
        switch (selectedMenu) {  // 根据当前选中的菜单项执行操作
        case 0:
            startNewGame();      // 开始新游戏
            break;
        case 1:
            // 继续游戏（需要有游戏进度）
            if (score > 0) {
                currentState = STATE_PLAYING;  // 切换到游戏状态
            }
            break;
        case 2:
            currentState = STATE_LEADERBOARD;  // 切换到排行榜界面
            break;
        case 3:
            currentState = STATE_SETTINGS;     // 切换到设置界面
            selectedSetting = 0;               // 重置设置选择索引
            break;
        case 4:
            currentState = STATE_HELP;         // 切换到帮助界面
            break;
        case 5:
            currentState = STATE_CREDITS;      // 切换到制作人员界面
            break;
        case 6:
            closegraph();   // 关闭图形窗口
            exit(0);        // 退出程序
            break;
        }
    }
}

// 游戏输入处理方法：处理游戏中的按键
void Game::handleGameInput() {
    // 空格键或上方向键：小鸟跳跃
    if (keyPressed[VK_SPACE] || keyPressed[VK_UP]) {
        bird->jump();  // 调用小鸟跳跃方法
        // 在跳跃位置创建粒子效果
        createParticles(bird->getX(), bird->getY(), 8, RGB(255, 255, 0), 1);
    }
    // ESC键：暂停游戏
    if (keyPressed[VK_ESCAPE]) {
        currentState = STATE_PAUSED;  // 切换到暂停状态
    }
    // R键：重新开始游戏
    if (keyPressed['R'] || keyPressed['r']) {
        startNewGame();  // 开始新游戏
    }
}

// 暂停状态输入处理方法
void Game::handlePauseInput() {
    // ESC键或空格键：继续游戏
    if (keyPressed[VK_ESCAPE] || keyPressed[VK_SPACE]) {
        currentState = STATE_PLAYING;  // 切换回游戏状态
    }
    // M键：返回主菜单
    if (keyPressed['M'] || keyPressed['m']) {
        currentState = STATE_MENU;  // 切换到主菜单
    }
    // R键：重新开始游戏
    if (keyPressed['R'] || keyPressed['r']) {
        startNewGame();  // 开始新游戏
    }
}

// 游戏结束状态输入处理方法
void Game::handleGameOverInput() {
    // 空格键或回车键：重新开始游戏
    if (keyPressed[VK_SPACE] || keyPressed[VK_RETURN]) {
        startNewGame();  // 开始新游戏
    }
    // ESC键：返回主菜单
    if (keyPressed[VK_ESCAPE]) {
        currentState = STATE_MENU;  // 切换到主菜单
    }
}

// 排行榜界面输入处理方法
void Game::handleLeaderboardInput() {
    // ESC键或空格键：返回主菜单
    if (keyPressed[VK_ESCAPE] || keyPressed[VK_SPACE]) {
        currentState = STATE_MENU;  // 切换到主菜单
    }
}

// 设置界面输入处理方法
void Game::handleSettingsInput() {
    // ESC键：保存设置并返回主菜单
    if (keyPressed[VK_ESCAPE]) {
        currentState = STATE_MENU;  // 切换到主菜单
        // 应用当前设置到小鸟
        bird->setGravity(birdGravity);
        bird->setJumpForce(birdJumpForce);
    }
    // 上方向键：设置项向上移动
    if (keyPressed[VK_UP]) {
        selectedSetting = (selectedSetting - 1 + 5) % 5;  // 循环选择（5个设置项）
    }
    // 下方向键：设置项向下移动
    if (keyPressed[VK_DOWN]) {
        selectedSetting = (selectedSetting + 1) % 5;  // 循环选择
    }
    // 左方向键：减少当前设置项的值
    if (keyPressed[VK_LEFT]) {
        adjustSetting(-1);  // 方向为-1表示减少
    }
    // 右方向键：增加当前设置项的值
    if (keyPressed[VK_RIGHT]) {
        adjustSetting(1);   // 方向为1表示增加
    }
}

// 调整设置项方法：根据方向调整当前选中的设置
void Game::adjustSetting(int direction) {
    switch (selectedSetting) {  // 根据当前选中的设置项
    case 0:  // 重力设置
        birdGravity += direction * 0.1f;  // 按0.1的步长调整
        // 限制重力范围（0.2到1.5之间）
        if (birdGravity < 0.2f) birdGravity = 0.2f;
        if (birdGravity > 1.5f) birdGravity = 1.5f;
        break;
    case 1:  // 跳跃力量设置
        birdJumpForce += direction * 0.5f;  // 按0.5的步长调整
        // 限制跳跃力量范围（-12到-5之间，负数表示向上）
        if (birdJumpForce > -5.0f) birdJumpForce = -5.0f;
        if (birdJumpForce < -12.0f) birdJumpForce = -12.0f;
        break;
    case 2:  // 难度设置
        difficulty = (difficulty + direction + 3) % 3;  // 在0,1,2之间循环
        applyDifficulty();  // 应用难度设置
        break;
    case 3:  // 显示FPS设置
        showFPS = !showFPS;  // 切换显示/隐藏FPS
        break;
    case 4:  // 显示碰撞框设置
        showHitboxes = !showHitboxes;  // 切换显示/隐藏碰撞框
        break;
    }
}

// 应用难度设置方法：根据难度等级调整游戏参数
void Game::applyDifficulty() {
    switch (difficulty) {  // 根据难度等级
    case 0:  // 简单模式
        birdGravity = 0.4f;      // 较小的重力
        birdJumpForce = -7.5f;   // 较小的跳跃力量
        gameSpeed = 2.5f;        // 较慢的游戏速度
        break;
    case 1:  // 普通模式（默认）
        birdGravity = 0.5f;      // 标准重力
        birdJumpForce = -8.5f;   // 标准跳跃力量
        gameSpeed = 3.0f;        // 标准游戏速度
        break;
    case 2:  // 困难模式
        birdGravity = 0.6f;      // 较大的重力
        birdJumpForce = -9.5f;   // 较大的跳跃力量
        gameSpeed = 3.5f;        // 较快的游戏速度
        break;
    }
    // 将设置应用到小鸟对象
    bird->setGravity(birdGravity);
    bird->setJumpForce(birdJumpForce);
}

// 帮助界面输入处理方法
void Game::handleHelpInput() {
    // ESC键或空格键：返回主菜单
    if (keyPressed[VK_ESCAPE] || keyPressed[VK_SPACE]) {
        currentState = STATE_MENU;  // 切换到主菜单
    }
}

// 制作人员界面输入处理方法
void Game::handleCreditsInput() {
    // ESC键或空格键：返回主菜单
    if (keyPressed[VK_ESCAPE] || keyPressed[VK_SPACE]) {
        currentState = STATE_MENU;  // 切换到主菜单
    }
}

// 游戏更新方法：根据时间更新游戏状态
void Game::update(float deltaTime) {
    animationTime += deltaTime;  // 累计动画时间

    // 处理屏幕震动效果
    if (shakeTime > 0) {
        shakeTime -= deltaTime;          // 减少震动剩余时间
        shakeIntensity *= 0.9f;          // 逐渐减弱震动强度
    }

    // 更新所有云朵的位置
    for (auto& cloud : clouds) {
        cloud.update(deltaTime);
    }

    // 更新所有粒子效果
    updateParticles(deltaTime);

    // 根据游戏状态执行不同的更新逻辑
    switch (currentState) {
    case STATE_PLAYING:
        updateGameplay(deltaTime);  // 更新游戏玩法逻辑
        break;
    default:
        // 其他状态不需要更新游戏玩法
        break;
    }
}

// 更新游戏玩法逻辑
void Game::updateGameplay(float deltaTime) {
    gameTime += deltaTime;  // 累计游戏时间

    bird->update(deltaTime);  // 更新小鸟状态

    // 检查小鸟是否碰到地面
    if (bird->getY() >= SCREEN_HEIGHT - GROUND_HEIGHT - 15) {
        gameOver();  // 游戏结束
        return;      // 提前返回，不再执行后面的代码
    }

    // 更新管道管理器（移动管道，检查碰撞等）
    pipeManager->update(gameSpeed, bird, score, level,
        gameSpeed, shakeTime, shakeIntensity, *this);

    // 检查小鸟与管道的碰撞
    if (pipeManager->checkCollision(bird->getCollisionRect())) {
        gameOver();  // 游戏结束
        return;      // 提前返回
    }

    // 检查小鸟与硬币的碰撞
    if (pipeManager->checkCoinCollision(bird->getCollisionRect())) {
        coins += 10;   // 增加硬币数量
        score += 5;    // 增加分数
        // 创建硬币收集粒子效果
        createParticles(bird->getX(), bird->getY(), 15, RGB(255, 215, 0), 1);
        shakeScreen(5.0f);  // 屏幕震动效果
    }

    // 检查小鸟是否通过管道
    if (pipeManager->checkPipePassed(bird->getX())) {
        pipesPassed++;  // 增加通过的管道数量
        // 计算得分：基础分1分乘以连击倍数
        score += 1 * bird->getScoreMultiplier();

        bird->addCombo();  // 增加连击计数

        // 每通过5个管道，提升等级和游戏速度
        if (pipesPassed % 5 == 0) {
            level++;              // 等级提升
            gameSpeed += 0.2f;    // 游戏速度增加
            shakeScreen(3.0f);    // 屏幕震动效果
        }

        // 更新最高分
        if (score > highScore) {
            highScore = score;
        }
    }

    // 更新管道生成计时器
    pipeTimer += deltaTime;
    // 计算管道生成间隔：随着等级提高，间隔变短（最低1.5秒）
    float spawnInterval = (3.0f - level * 0.1f) > 1.5f ? (3.0f - level * 0.1f) : 1.5f;

    // 如果达到生成间隔时间，生成新管道
    if (pipeTimer > spawnInterval) {
        pipeTimer = 0;  // 重置计时器
        // 在屏幕右侧生成新管道
        pipeManager->addPipe((float)SCREEN_WIDTH, nextPipeID++);
    }
}

// 创建粒子效果方法
void Game::createParticles(float x, float y, int count,
    COLORREF color, int type) {
    // 创建指定数量的粒子
    for (int i = 0; i < count; i++) {
        // 在指定位置创建粒子并添加到粒子数组
        particles.push_back(Particle(x, y, color, type));
    }
}

// 更新所有粒子效果
void Game::updateParticles(float deltaTime) {
    // 遍历所有粒子并更新它们
    for (auto& particle : particles) {
        particle.update(deltaTime);
    }

    // 删除生命周期结束的粒子
    auto it = particles.begin();  // 获取粒子数组的起始迭代器
    while (it != particles.end()) {
        if (it->shouldRemove()) {           // 使用公有方法
            it = particles.erase(it);  // 删除粒子，并获取下一个迭代器
        }
        else {
            ++it;  // 移动到下一个粒子
        }
    }
}

// 屏幕震动效果方法
void Game::shakeScreen(float intensity) {
    shakeTime = 0.3f;          // 设置震动持续时间（0.3秒）
    shakeIntensity = intensity; // 设置震动强度
}

// 开始新游戏方法：重置所有游戏状态
void Game::startNewGame() {
    bird->reset();          // 重置小鸟状态
    pipeManager->clearPipes();  // 清空所有管道
    particles.clear();      // 清空所有粒子效果

    // 重置游戏数值
    score = 0;          // 分数归零
    coins = 0;          // 硬币数量归零
    level = 1;          // 等级重置为1
    gameSpeed = 3.0f;   // 游戏速度重置
    pipeTimer = 0;      // 管道生成计时器重置
    gameTime = 0;       // 游戏时间重置
    pipesPassed = 0;    // 通过的管道数量重置
    nextPipeID = 0;     // 管道ID重置

    applyDifficulty();  // 应用当前难度设置

    currentState = STATE_PLAYING;  // 切换到游戏状态
}

// 游戏结束方法：处理游戏结束逻辑
void Game::gameOver() {
    bird->kill();  // 设置小鸟为死亡状态

    // 创建游戏结束粒子效果（红色轨迹效果）
    createParticles(bird->getX(), bird->getY(), 50, RGB(255, 50, 50), 2);

    shakeScreen(10.0f);  // 强烈的屏幕震动效果

    addToLeaderboard();  // 将分数添加到排行榜

    currentState = STATE_GAME_OVER;  // 切换到游戏结束状态
}

// 游戏渲染方法：绘制游戏画面
void Game::render() {
    BeginBatchDraw();  // 开始批量绘制（提高绘制效率）

    // 计算屏幕震动偏移
    int shakeX = 0, shakeY = 0;
    if (shakeTime > 0) {
        // 随机生成X和Y方向的震动偏移
        shakeX = (rand() % (int)(shakeIntensity * 2)) - (int)shakeIntensity;
        shakeY = (rand() % (int)(shakeIntensity * 2)) - (int)shakeIntensity;
    }

    cleardevice();  // 清空屏幕（用背景色填充）

    drawSkyBackground();  // 绘制天空背景

    // 绘制所有云朵
    for (const auto& cloud : clouds) {
        cloud.draw();
    }

    drawGround();  // 绘制地面

    // 只在游戏相关状态绘制游戏元素
    if (currentState == STATE_PLAYING ||
        currentState == STATE_PAUSED ||
        currentState == STATE_GAME_OVER) {

        pipeManager->draw();  // 绘制所有管道

        // 绘制所有粒子效果
        for (const auto& particle : particles) {
            particle.draw();
        }

        bird->draw();  // 绘制小鸟

        // 如果开启了碰撞框显示，绘制碰撞框
        if (showHitboxes) {
            drawHitboxes();
        }

        drawGameUI();  // 绘制游戏UI
    }

    // 根据当前游戏状态绘制对应的界面
    switch (currentState) {
    case STATE_MENU:
        drawMenu();        // 绘制主菜单
        break;
    case STATE_PAUSED:
        drawPauseMenu();   // 绘制暂停菜单
        break;
    case STATE_GAME_OVER:
        drawGameOver();    // 绘制游戏结束界面
        break;
    case STATE_LEADERBOARD:
        drawLeaderboard(); // 绘制排行榜
        break;
    case STATE_SETTINGS:
        drawSettings();    // 绘制设置界面
        break;
    case STATE_HELP:
        drawHelp();        // 绘制帮助界面
        break;
    case STATE_CREDITS:
        drawCredits();     // 绘制制作人员界面
        break;
    }

    // 如果开启了FPS显示，绘制FPS
    if (showFPS) {
        drawFPS();
    }

    // 如果正在震动效果中，绘制震动边框
    if (shakeTime > 0) {
        drawShakeEffect(shakeX, shakeY);
    }

    FlushBatchDraw();  // 结束批量绘制，实际显示到屏幕
}

// 绘制天空背景：创建渐变天空效果
void Game::drawSkyBackground() {
    // 从上到下绘制渐变线，创建天空效果
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        // 计算当前位置的渐变比例（0到1之间）
        float ratio = (float)y / SCREEN_HEIGHT;

        // 计算当前行的颜色（从浅蓝色渐变到深蓝色）
        int r = (int)(GetRValue(COLOR_SKY_START) * (1 - ratio) +
            GetRValue(COLOR_SKY_END) * ratio);
        int g = (int)(GetGValue(COLOR_SKY_START) * (1 - ratio) +
            GetGValue(COLOR_SKY_END) * ratio);
        int b = (int)(GetBValue(COLOR_SKY_START) * (1 - ratio) +
            GetBValue(COLOR_SKY_END) * ratio);

        setlinecolor(RGB(r, g, b));  // 设置线条颜色
        line(0, y, SCREEN_WIDTH, y); // 绘制一条横线
    }

    // 绘制太阳
    setfillcolor(RGB(255, 255, 0));  // 黄色填充
    setlinecolor(RGB(255, 200, 0));  // 橙黄色边框
    solidcircle(SCREEN_WIDTH - 80, 80, 40);  // 绘制实心圆作为太阳

    // 绘制太阳光晕（多个同心圆）
    setlinecolor(RGB(255, 255, 0));  // 黄色边框
    for (int i = 1; i <= 3; i++) {
        int radius = 40 + i * 10;  // 计算每个光晕圈的半径
        circle(SCREEN_WIDTH - 80, 80, radius);  // 绘制圆形
    }
}

// 绘制地面：包括地面、草和装饰
void Game::drawGround() {
    // 绘制地面主体
    setfillcolor(COLOR_GROUND);  // 设置地面颜色（土黄色）
    fillrectangle(0, SCREEN_HEIGHT - GROUND_HEIGHT,
        SCREEN_WIDTH, SCREEN_HEIGHT);  // 填充矩形

    // 绘制草地（随机高度的草叶）
    setfillcolor(COLOR_GRASS);  // 设置草的颜色（亮绿色）
    for (int x = 0; x < SCREEN_WIDTH; x += 20) {
        int height = 5 + rand() % 15;  // 随机草叶高度（5-19像素）
        // 绘制草叶（细长的矩形）
        fillrectangle(x, SCREEN_HEIGHT - GROUND_HEIGHT - height,
            x + 15, SCREEN_HEIGHT - GROUND_HEIGHT);
    }

    // 绘制地面装饰（小土块）
    setfillcolor(RGB(139, 69, 19));  // 设置土块颜色（棕色）
    for (int x = 0; x < SCREEN_WIDTH; x += 40) {
        // 绘制小土块
        fillrectangle(x, SCREEN_HEIGHT - GROUND_HEIGHT,
            x + 20, SCREEN_HEIGHT - GROUND_HEIGHT + 10);
    }
}

// 绘制游戏UI：显示分数、等级、硬币等信息
void Game::drawGameUI() {
    // 设置分数显示的文字样式
    settextstyle(36, 0, _T("Arial"));  // 36号Arial字体
    settextcolor(COLOR_TEXT_WHITE);    // 白色文字
    setbkmode(TRANSPARENT);            // 透明背景

    wchar_t wbuffer[100];  // 格式化字符串缓冲区

    // 格式化并显示当前分数
    swprintf_s(wbuffer, 100, L"%d", score);
    int scoreWidth = textwidth(wbuffer);  // 获取文字宽度
    // 在屏幕顶部中央显示分数
    outtextxy(SCREEN_WIDTH / 2 - scoreWidth / 2, 30, wbuffer);

    // 如果有连击，显示连击数
    if (bird->getComboCount() > 0) {
        settextstyle(24, 0, _T("Arial"));  // 稍小号字体
        settextcolor(RGB(255, 215, 0));    // 金色文字
        // 格式化连击文本
        swprintf_s(wbuffer, 100, L"COMBO x%d", bird->getComboCount());
        // 在分数下方显示连击
        outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 75, wbuffer);
    }

    // 设置游戏信息显示的文字样式
    settextstyle(18, 0, _T("Arial"));   // 小号字体
    settextcolor(RGB(200, 200, 255));   // 浅蓝色文字

    // 显示等级
    swprintf_s(wbuffer, 100, L"Level: %d", level);
    outtextxy(20, 20, wbuffer);  // 左上角显示

    // 显示硬币数量
    swprintf_s(wbuffer, 100, L"Coins: %d", coins);
    outtextxy(20, 50, wbuffer);  // 等级下方显示

    // 显示游戏速度
    swprintf_s(wbuffer, 100, L"Speed: %.1f", gameSpeed);
    outtextxy(20, 80, wbuffer);  // 硬币下方显示

    // 显示游戏时间（分:秒格式）
    int minutes = (int)gameTime / 60;  // 计算分钟
    int seconds = (int)gameTime % 60;  // 计算秒数
    swprintf_s(wbuffer, 100, L"Time: %02d:%02d", minutes, seconds);
    outtextxy(20, 110, wbuffer);  // 速度下方显示

    // 显示最高分
    swprintf_s(wbuffer, 100, L"Best: %d", highScore);
    outtextxy(20, 140, wbuffer);  // 时间下方显示

    // 显示玩家名称（屏幕右上角）
    settextcolor(RGB(255, 200, 255));  // 浅粉色文字
    wchar_t wname[100];
    size_t converted = 0;
    // 将玩家名称从多字节转换为宽字符
    mbstowcs_s(&converted, wname, playerName.c_str(), 100);
    swprintf_s(wbuffer, 100, L"Player: %s", wname);
    // 计算文字宽度，靠右显示
    outtextxy(SCREEN_WIDTH - textwidth(wbuffer) - 20, 20, wbuffer);

    // 如果正在游戏中，显示操作提示
    if (currentState == STATE_PLAYING) {
        settextcolor(RGB(150, 150, 150));  // 灰色文字
        settextstyle(14, 0, _T("Arial"));  // 更小号字体
        // 在屏幕左下角显示操作提示
        outtextxy(20, SCREEN_HEIGHT - 40,
            L"SPACE: Jump  ESC: Pause  R: Restart");
    }
}

// 绘制碰撞框方法：用于调试显示碰撞检测区域
void Game::drawHitboxes() {
    setlinecolor(RGB(255, 0, 0));  // 设置碰撞框颜色为红色
    setlinestyle(PS_DASH, 1);       // 设置虚线样式，宽度为1像素

    // 获取小鸟的碰撞矩形并绘制
    RECT birdRect = bird->getCollisionRect();
    rectangle(birdRect.left, birdRect.top,
        birdRect.right, birdRect.bottom);  // 绘制矩形框

    // 绘制所有管道的碰撞框
    pipeManager->drawHitboxes();

    setlinestyle(PS_SOLID, 1);  // 恢复实线样式，避免影响其他绘制
}

// 绘制FPS（每秒帧数）显示
void Game::drawFPS() {
    // 静态变量用于计算FPS
    static DWORD lastTime = GetTickCount();  // 上次计算时间
    static int frameCount = 0;               // 帧数计数器
    static float fps = 0;                    // 当前FPS值

    frameCount++;  // 每调用一次增加一帧
    DWORD currentTime = GetTickCount();  // 获取当前时间（毫秒）

    // 如果超过1秒（1000毫秒），重新计算FPS
    if (currentTime - lastTime >= 1000) {
        // 计算FPS：帧数 ÷ 经过的时间（秒）
        fps = frameCount * 1000.0f / (currentTime - lastTime);
        frameCount = 0;          // 重置帧数计数器
        lastTime = currentTime;  // 更新上次计算时间
    }

    wchar_t wbuffer[20];  // 格式化字符串缓冲区
    swprintf_s(wbuffer, 20, L"FPS: %.1f", fps);  // 格式化FPS字符串

    // 设置FPS显示的文字样式
    settextstyle(12, 0, _T("Arial"));        // 12号小字体
    settextcolor(RGB(150, 150, 150));        // 灰色文字
    setbkmode(TRANSPARENT);                  // 透明背景
    // 在屏幕右下角显示FPS
    outtextxy(SCREEN_WIDTH - 70, SCREEN_HEIGHT - 20, wbuffer);
}

// 绘制屏幕震动效果：多层白色边框
void Game::drawShakeEffect(int shakeX, int shakeY) {
    setlinecolor(RGB(255, 255, 255));  // 白色边框

    // 绘制5层逐渐增大的白色边框
    for (int i = 0; i < 5; i++) {
        int offset = i * 2;  // 每层边框偏移2像素

        // 绘制矩形边框
        rectangle(offset + shakeX, offset + shakeY,              // 左上角
            SCREEN_WIDTH - offset + shakeX,                // 右下角X
            SCREEN_HEIGHT - offset + shakeY);              // 右下角Y
    }
}

// 绘制主菜单界面
void Game::drawMenu() {
    // 黑色背景
    setfillcolor(BLACK);  // 设置填充颜色为黑色
    fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);  // 填充整个屏幕

    // 绘制游戏标题
    settextstyle(72, 0, _T("Arial"));      // 72号大字体
    settextcolor(COLOR_TEXT_YELLOW);       // 黄色标题
    const wchar_t* title = L"FLAPPY BIRD";  // 游戏标题
    int titleWidth = textwidth(title);      // 获取标题宽度
    // 在屏幕中央显示标题
    outtextxy(SCREEN_WIDTH / 2 - titleWidth / 2, 80, title);

    // 绘制副标题
    settextstyle(28, 0, _T("Arial"));       // 28号字体
    settextcolor(COLOR_TEXT_BLUE);          // 蓝色文字
    // 显示"ULTIMATE EDITION"
    outtextxy(SCREEN_WIDTH / 2 - 120, 160, L"ULTIMATE EDITION");

    // 菜单项数组
    const wchar_t* menuItems[] = {
        L"START NEW GAME",    // 菜单项0：开始新游戏
        L"CONTINUE",          // 菜单项1：继续游戏
        L"LEADERBOARD",       // 菜单项2：排行榜
        L"SETTINGS",          // 菜单项3：设置
        L"HELP",              // 菜单项4：帮助
        L"CREDITS",           // 菜单项5：制作人员
        L"EXIT"               // 菜单项6：退出
    };

    settextstyle(28, 0, _T("Arial"));  // 设置菜单项字体

    // 遍历所有菜单项并绘制
    for (int i = 0; i < 7; i++) {
        int y = 220 + i * 45;  // 计算每个菜单项的Y坐标（垂直间距45像素）

        // 根据是否选中设置不同的样式
        if (i == selectedMenu) {
            // 当前选中的菜单项：红色、稍大的字体
            settextcolor(COLOR_TEXT_RED);
            settextstyle(32, 0, _T("Arial"));
        }
        else {
            // 未选中的菜单项：白色、正常字体
            settextcolor(COLOR_TEXT_WHITE);
            settextstyle(28, 0, _T("Arial"));
        }

        int itemWidth = textwidth(menuItems[i]);  // 获取菜单项宽度
        // 在屏幕中央显示菜单项
        outtextxy(SCREEN_WIDTH / 2 - itemWidth / 2, y, menuItems[i]);

        // 如果当前项被选中，在两侧绘制红色小圆点
        if (i == selectedMenu) {
            setfillcolor(COLOR_TEXT_RED);  // 红色填充
            // 左侧圆点
            solidcircle(SCREEN_WIDTH / 2 - itemWidth / 2 - 25, y + 15, 8);
            // 右侧圆点
            solidcircle(SCREEN_WIDTH / 2 + itemWidth / 2 + 25, y + 15, 8);
        }
    }

    // 绘制版本信息和操作提示
    settextstyle(14, 0, _T("Arial"));          // 小号字体
    settextcolor(RGB(150, 150, 150));          // 灰色文字
    outtextxy(10, SCREEN_HEIGHT - 20,          // 左下角显示版本信息
        L"Version 2.0 | Ultimate Edition");
    // 在屏幕下方中央显示操作提示
    outtextxy(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT - 60,
        L"Use ARROW KEYS to navigate, ENTER to select");
}

// 绘制暂停菜单
void Game::drawPauseMenu() {
    // 黑色半透明背景
    setfillcolor(BLACK);
    fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // 绘制"PAUSED"标题
    settextstyle(64, 0, _T("Arial"));      // 64号大字体
    settextcolor(COLOR_TEXT_YELLOW);       // 黄色文字
    outtextxy(SCREEN_WIDTH / 2 - 120, 150, L"PAUSED");  // 屏幕中央

    // 绘制游戏信息
    settextstyle(24, 0, _T("Arial"));      // 24号字体
    settextcolor(COLOR_TEXT_WHITE);        // 白色文字

    wchar_t wbuffer[100];  // 格式化字符串缓冲区

    // 显示当前分数
    swprintf_s(wbuffer, 100, L"Score: %d", score);
    outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 240, wbuffer);

    // 显示当前等级
    swprintf_s(wbuffer, 100, L"Level: %d", level);
    outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 280, wbuffer);

    // 绘制操作提示
    settextstyle(20, 0, _T("Arial"));              // 20号字体
    settextcolor(RGB(200, 200, 255));              // 浅蓝色文字

    // 继续游戏提示
    outtextxy(SCREEN_WIDTH / 2 - 150, 350,
        L"Press ESC or SPACE to continue");
    // 重新开始游戏提示
    outtextxy(SCREEN_WIDTH / 2 - 100, 380,
        L"Press R to restart");
    // 返回主菜单提示
    outtextxy(SCREEN_WIDTH / 2 - 80, 410,
        L"Press M for menu");
}

// 绘制游戏结束界面
void Game::drawGameOver() {
    // 黑色背景
    setfillcolor(BLACK);
    fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // 绘制"GAME OVER"标题
    settextstyle(64, 0, _T("Arial"));      // 64号大字体
    settextcolor(COLOR_TEXT_RED);          // 红色文字
    outtextxy(SCREEN_WIDTH / 2 - 180, 100, L"GAME OVER");

    // 绘制游戏结果信息
    settextstyle(36, 0, _T("Arial"));      // 36号字体
    settextcolor(COLOR_TEXT_WHITE);        // 白色文字

    wchar_t wbuffer[100];

    // 显示最终分数
    swprintf_s(wbuffer, 100, L"Final Score: %d", score);
    outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 200, wbuffer);

    // 显示游戏统计数据
    settextstyle(24, 0, _T("Arial"));      // 24号字体

    // 达到的最高等级
    swprintf_s(wbuffer, 100, L"Level Reached: %d", level);
    outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 250, wbuffer);

    // 收集的硬币数量
    swprintf_s(wbuffer, 100, L"Coins Collected: %d", coins);
    outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 280, wbuffer);

    // 游戏时间（分钟:秒格式）
    int minutes = (int)gameTime / 60;      // 分钟
    int seconds = (int)gameTime % 60;      // 秒
    swprintf_s(wbuffer, 100, L"Play Time: %02d:%02d", minutes, seconds);
    outtextxy(SCREEN_WIDTH / 2 - textwidth(wbuffer) / 2, 310, wbuffer);

    // 如果获得了新高分，显示特殊效果
    if (score == highScore && score > 0) {
        settextstyle(32, 0, _T("Arial"));          // 32号字体
        settextcolor(RGB(255, 215, 0));            // 金色文字
        outtextxy(SCREEN_WIDTH / 2 - 100, 350, L"NEW HIGH SCORE!");

        // 绘制动态星星效果
        setfillcolor(RGB(255, 215, 0));            // 金色填充
        for (int i = 0; i < 5; i++) {
            // 计算星星的动画角度（基于时间和索引）
            float angle = animationTime * 2 + i * 1.256f;
            // 计算星星X坐标（等间距分布）
            int starX = SCREEN_WIDTH / 2 - 150 + i * 75;
            // 计算星星Y坐标（使用正弦函数产生上下浮动）
            int starY = 400 + (int)(sin(angle) * 10);
            // 计算星星大小（使用正弦函数产生大小变化）
            int starSize = 10 + (int)(sin(animationTime * 3 + i) * 5);
            // 绘制星星（实心圆）
            solidcircle(starX, starY, starSize);
        }
    }

    // 绘制操作提示
    settextstyle(20, 0, _T("Arial"));              // 20号字体
    settextcolor(RGB(200, 200, 255));              // 浅蓝色文字
    // 重新开始游戏提示
    outtextxy(SCREEN_WIDTH / 2 - 150, 450,
        L"Press SPACE to play again");
    // 返回主菜单提示
    outtextxy(SCREEN_WIDTH / 2 - 120, 480,
        L"Press ESC to return to menu");
}

// 绘制排行榜界面
void Game::drawLeaderboard() {
    // 深蓝色背景
    setfillcolor(RGB(20, 25, 40));  // RGB(20,25,40)深蓝色
    fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // 绘制标题"LEADERBOARD"
    settextstyle(48, 0, _T("Arial"));       // 48号字体
    settextcolor(COLOR_TEXT_YELLOW);        // 黄色文字
    outtextxy(SCREEN_WIDTH / 2 - 100, 40, L"LEADERBOARD");

    // 绘制列标题
    settextstyle(20, 0, _T("Arial"));       // 20号字体
    settextcolor(RGB(180, 180, 255));       // 浅紫色文字

    // 显示各列标题
    outtextxy(100, 110, L"Rank");    // 排名
    outtextxy(180, 110, L"Player");  // 玩家
    outtextxy(350, 110, L"Score");   // 分数
    outtextxy(450, 110, L"Level");   // 等级
    outtextxy(550, 110, L"Time");    // 时间
    outtextxy(650, 110, L"Date");    // 日期

    // 绘制标题下方的分隔线
    setlinecolor(RGB(100, 100, 150));       // 深蓝色线条
    line(80, 140, 720, 140);               // 从(80,140)到(720,140)

    // 绘制排行榜数据
    settextstyle(18, 0, _T("Arial"));       // 18号字体

    // 显示最多10条记录
    int displayCount = (int)leaderboard.size() < 10 ? (int)leaderboard.size() : 10;
    for (int i = 0; i < displayCount; i++) {
        int y = 160 + i * 35;  // 计算当前行的Y坐标（每行高35像素）

        // 交替行使用不同背景色，提高可读性
        if (i % 2 == 0) {
            setfillcolor(RGB(40, 45, 70));  // 较浅的深蓝色
        }
        else {
            setfillcolor(RGB(30, 35, 60));  // 较深的深蓝色
        }
        // 填充行背景
        fillrectangle(80, y - 5, 720, y + 30);

        // 根据排名设置文字颜色
        if (i == 0) settextcolor(RGB(255, 215, 0));      // 第一名：金色
        else if (i == 1) settextcolor(RGB(192, 192, 192)); // 第二名：银色
        else if (i == 2) settextcolor(RGB(205, 127, 50));  // 第三名：铜色
        else settextcolor(RGB(200, 200, 255));           // 其他名次：浅蓝色

        wchar_t wbuffer[50];  // 格式化字符串缓冲区

        // 显示排名（第几名）
        swprintf_s(wbuffer, 50, L"%d.", i + 1);
        outtextxy(100, y, wbuffer);

        // 显示玩家名称（绿色文字）
        settextcolor(RGB(100, 255, 100));  // 亮绿色
        wchar_t wname[50];
        size_t converted = 0;
        // 转换玩家名称从多字节到宽字符
        mbstowcs_s(&converted, wname, leaderboard[i].playerName.c_str(), 50);
        outtextxy(180, y, wname);

        // 显示分数（白色文字）
        settextcolor(COLOR_TEXT_WHITE);
        swprintf_s(wbuffer, 50, L"%d", leaderboard[i].score);
        outtextxy(350, y, wbuffer);

        // 显示等级（白色文字）
        swprintf_s(wbuffer, 50, L"%d", leaderboard[i].level);
        outtextxy(450, y, wbuffer);

        // 显示游戏时间（分钟:秒格式）
        int minutes = leaderboard[i].playTime / 60;  // 分钟
        int seconds = leaderboard[i].playTime % 60;  // 秒
        swprintf_s(wbuffer, 50, L"%02d:%02d", minutes, seconds);
        outtextxy(550, y, wbuffer);

        // 显示日期（月/日格式）
        tm timeinfo;  // 时间结构体
        localtime_s(&timeinfo, &leaderboard[i].date);  // 转换时间为本地时间
        swprintf_s(wbuffer, 50, L"%02d/%02d",
            timeinfo.tm_mon + 1,  // 月份（从0开始，所以+1）
            timeinfo.tm_mday);    // 日
        outtextxy(650, y, wbuffer);
    }

    // 绘制返回提示
    settextstyle(18, 0, _T("Arial"));       // 18号字体
    settextcolor(RGB(150, 150, 200));       // 浅紫色文字
    outtextxy(SCREEN_WIDTH / 2 - 100, 550,
        L"Press ESC to return to menu");
}

// 绘制设置界面
void Game::drawSettings() {
    // 深蓝色背景
    setfillcolor(RGB(30, 35, 50));  // RGB(30,35,50)
    fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // 绘制标题"SETTINGS"
    settextstyle(48, 0, _T("Arial"));       // 48号字体
    settextcolor(COLOR_TEXT_BLUE);          // 蓝色文字
    outtextxy(SCREEN_WIDTH / 2 - 100, 40, L"SETTINGS");

    // 设置项名称数组
    const wchar_t* settingNames[] = {
        L"GRAVITY",       // 设置项0：重力
        L"JUMP FORCE",    // 设置项1：跳跃力量
        L"DIFFICULTY",    // 设置项2：难度
        L"SHOW FPS",      // 设置项3：显示FPS
        L"SHOW HITBOXES"  // 设置项4：显示碰撞框
    };

    // 难度等级名称数组
    const wchar_t* difficultyNames[] = { L"EASY", L"NORMAL", L"HARD" };

    // 绘制所有设置项
    settextstyle(24, 0, _T("Arial"));       // 24号字体

    for (int i = 0; i < 5; i++) {
        int y = 120 + i * 70;  // 计算每个设置项的Y坐标（间距70像素）

        // 根据是否选中设置不同的文字颜色
        if (i == selectedSetting) {
            settextcolor(COLOR_TEXT_RED);  // 选中项：红色
        }
        else {
            settextcolor(COLOR_TEXT_WHITE); // 未选中：白色
        }
        // 显示设置项名称
        outtextxy(150, y, settingNames[i]);

        // 显示设置项的当前值（浅蓝色文字）
        settextcolor(RGB(200, 200, 255));
        wchar_t wbuffer[50];

        // 根据设置项索引显示对应的值
        switch (i) {
        case 0:  // 重力值
            swprintf_s(wbuffer, 50, L"%.1f", birdGravity);  // 显示一位小数
            break;
        case 1:  // 跳跃力量（显示绝对值，因为是负数）
            swprintf_s(wbuffer, 50, L"%.1f", fabs(birdJumpForce));
            break;
        case 2:  // 难度等级
            swprintf_s(wbuffer, 50, L"%s", difficultyNames[difficulty]);
            break;
        case 3:  // 显示FPS开关
            swprintf_s(wbuffer, 50, L"%s", showFPS ? L"ON" : L"OFF");
            break;
        case 4:  // 显示碰撞框开关
            swprintf_s(wbuffer, 50, L"%s", showHitboxes ? L"ON" : L"OFF");
            break;
        }

        // 显示设置项的值
        outtextxy(500, y, wbuffer);

        // 前3个设置项有进度条
        if (i < 3) {
            drawProgressBar(300, y + 10, 150, 20, i);  // 绘制进度条
        }

        // 如果当前设置项被选中，在左侧绘制红色小圆点
        if (i == selectedSetting) {
            setfillcolor(COLOR_TEXT_RED);
            solidcircle(120, y + 15, 8);  // 绘制小圆点
        }
    }

    // 绘制操作提示
    settextstyle(18, 0, _T("Arial"));       // 18号字体
    settextcolor(RGB(150, 150, 200));       // 浅紫色文字
    // 导航和调整提示
    outtextxy(SCREEN_WIDTH / 2 - 200, 500,
        L"Use ARROW KEYS to navigate and adjust values");
    // 保存返回提示
    outtextxy(SCREEN_WIDTH / 2 - 120, 530,
        L"Press ESC to save and return");
}

// 绘制进度条方法
void Game::drawProgressBar(int x, int y, int width, int height, int type) {
    // 绘制进度条背景（深灰色）
    setfillcolor(RGB(60, 60, 80));
    fillrectangle(x, y, x + width, y + height);

    float value = 0;        // 进度值（0到1之间）
    COLORREF fillColor;     // 进度条填充颜色

    // 根据进度条类型计算值和颜色
    switch (type) {
    case 0:  // 重力进度条
        // 计算重力值在范围内的比例（0.2-1.5映射到0-1）
        value = (birdGravity - 0.2f) / 1.3f;
        fillColor = RGB(0, 200, 255);  // 蓝色
        break;
    case 1:  // 跳跃力量进度条
        // 计算跳跃力量在范围内的比例（5-12映射到0-1）
        value = (fabs(birdJumpForce) - 5.0f) / 7.0f;
        fillColor = RGB(255, 100, 0);  // 橙色
        break;
    case 2:  // 难度进度条
        // 难度等级映射到0-1之间（0,1,2映射到0,0.5,1）
        value = difficulty / 2.0f;
        fillColor = RGB(255, 50, 50);  // 红色
        break;
    }

    // 绘制进度条填充部分
    setfillcolor(fillColor);
    // 根据value计算填充宽度
    fillrectangle(x, y, x + (int)(width * value), y + height);

    // 绘制进度条边框
    setlinecolor(RGB(100, 100, 120));  // 深灰色边框
    rectangle(x, y, x + width, y + height);
}

// 绘制帮助界面
void Game::drawHelp() {
    // 深绿色背景
    setfillcolor(RGB(25, 40, 30));  // RGB(25,40,30)深绿色
    fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // 绘制标题"HELP"
    settextstyle(48, 0, _T("Arial"));       // 48号字体
    settextcolor(COLOR_TEXT_GREEN);         // 绿色文字
    outtextxy(SCREEN_WIDTH / 2 - 80, 40, L"HELP");

    // 设置帮助文本样式
    settextstyle(20, 0, _T("Arial"));       // 20号字体
    settextcolor(COLOR_TEXT_WHITE);         // 白色文字

    // 帮助文本数组（多行）
    const wchar_t* helpLines[] = {
        L"GAME OBJECTIVE:",                          // 游戏目标标题
        L"  Guide the bird through the pipes without hitting them.", // 目标描述
        L"  Each pipe passed gives you 1 point.",    // 分数说明
        L"  Collect coins for bonus points.",        // 硬币说明
        L"",                                         // 空行
        L"CONTROLS:",                                // 控制标题
        L"  SPACE / UP ARROW - Make the bird jump", // 跳跃控制
        L"  ESC - Pause game / Return to menu",      // ESC控制
        L"  R - Restart game",                       // 重新开始
        L"  M - Return to main menu (when paused)",  // 返回菜单
        L"",                                         // 空行
        L"GAME FEATURES:",                           // 游戏特性标题
        L"  Combo System: Pass pipes quickly for score multiplier", // 连击系统
        L"  Level System: Game speed increases every 5 pipes",      // 等级系统
        L"  Coin Collection: Collect coins for extra points",       // 硬币收集
        L"  Difficulty Settings: Adjust game physics in settings",  // 难度设置
        L"",                                         // 空行
        L"TIPS:",                                    // 技巧标题
        L"  Tap lightly for small hops, hold for higher jumps",     // 跳跃技巧
        L"  Try to stay in the middle of pipe gaps",                // 位置技巧
        L"  Watch out for increasing speed at higher levels"        // 速度提示
    };

    int numLines = sizeof(helpLines) / sizeof(helpLines[0]);  // 计算总行数

    // 绘制所有帮助文本行
    for (int i = 0; i < numLines; i++) {
        // 每行垂直间距25像素
        outtextxy(100, 100 + i * 25, helpLines[i]);
    }

    // 绘制返回提示
    settextstyle(18, 0, _T("Arial"));       // 18号字体
    settextcolor(RGB(150, 200, 150));       // 浅绿色文字
    outtextxy(SCREEN_WIDTH / 2 - 120, 550,
        L"Press ESC to return to menu");
}

// 绘制制作人员界面
void Game::drawCredits() {
    // 深紫色背景
    setfillcolor(RGB(40, 30, 50));  // RGB(40,30,50)深紫色
    fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // 绘制标题"CREDITS"
    settextstyle(48, 0, _T("Arial"));       // 48号字体
    settextcolor(COLOR_TEXT_PURPLE);        // 紫色文字
    outtextxy(SCREEN_WIDTH / 2 - 120, 40, L"CREDITS");

    // 绘制副标题
    settextstyle(28, 0, _T("Arial"));       // 28号字体
    settextcolor(RGB(255, 200, 255));       // 浅粉色文字
    outtextxy(SCREEN_WIDTH / 2 - 150, 120, L"FLAPPY BIRD ULTIMATE EDITION");

    // 设置制作人员信息样式
    settextstyle(22, 0, _T("Arial"));       // 22号字体
    settextcolor(COLOR_TEXT_WHITE);         // 白色文字

    // 制作人员信息数组
    const wchar_t* credits[] = {
        L"Game Design & Programming:",      // 游戏设计和编程
        L"  开发者姓名",                    // 开发者姓名
        L"",                                // 空行
        L"Graphics & Animation:",           // 图形和动画
        L"  EasyX Graphics Library",        // 图形库
        L"  Custom Particle System",        // 粒子系统
        L"",                                // 空行
        L"Special Thanks To:",              // 特别感谢
        L"  Original Flappy Bird Creator",  // 原版Flappy Bird作者
        L"  All Beta Testers",              // 测试人员
        L"  Open Source Community",         // 开源社区
        L"",                                // 空行
        L"Version: 2.0 Ultimate",           // 版本信息
        L"Release Date: 2024",              // 发布日期
    };

    int numCredits = sizeof(credits) / sizeof(credits[0]);  // 计算总行数

    // 绘制所有制作人员信息行（居中显示）
    for (int i = 0; i < numCredits; i++) {
        int y = 180 + i * 30;  // 每行垂直间距30像素
        // 居中显示每一行
        outtextxy(SCREEN_WIDTH / 2 - textwidth(credits[i]) / 2, y, credits[i]);
    }

    // 绘制动态的心形效果（使用圆形替代）
    float pulse = sin(animationTime * 2) * 0.5f + 0.5f;  // 计算脉冲值（0-1之间）
    setfillcolor(RGB(255, 0, 0));  // 红色填充
    // 绘制大小动态变化的圆形（20-30像素）
    solidcircle(SCREEN_WIDTH / 2, 500, 20 + (int)(pulse * 10));

    // 绘制返回提示
    settextstyle(18, 0, _T("Arial"));       // 18号字体
    settextcolor(RGB(200, 150, 200));       // 浅紫色文字
    outtextxy(SCREEN_WIDTH / 2 - 120, 550,
        L"Press ESC to return to menu");
}

// 游戏主循环方法
void Game::run() {
    // 初始化图形窗口
    initgraph(SCREEN_WIDTH, SCREEN_HEIGHT);  // 创建指定大小的窗口
    setbkcolor(BLACK);                        // 设置背景颜色为黑色

    // 初始化随机数种子
    srand((unsigned)time(NULL));  // 使用当前时间作为随机种子

    // 高精度计时器相关变量
    LARGE_INTEGER frequency;      // 计时器频率
    LARGE_INTEGER lastTime, currentTime;  // 上次和当前时间
    QueryPerformanceFrequency(&frequency);  // 获取计时器频率（每秒计数次数）
    QueryPerformanceCounter(&lastTime);     // 获取初始时间

    const double frameInterval = 1.0 / FPS;  // 每帧的理想时间（秒）
    double accumulator = 0.0;                // 时间累积器

    // 游戏主循环
    while (true) {
        QueryPerformanceCounter(&currentTime);  // 获取当前时间

        // 计算自上次循环以来经过的时间（秒）
        double elapsedTime = (double)(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
        lastTime = currentTime;  // 更新上次时间

        if (elapsedTime > 0.25) elapsedTime = 0.25;  // 防止时间过长（防卡顿）

        accumulator += elapsedTime;  // 累积经过的时间

        // 处理输入
        updateInput();

        // 固定时间步长更新（维持稳定的游戏更新频率）
        while (accumulator >= frameInterval) {
            update((float)frameInterval);  // 用固定的时间步长更新游戏
            accumulator -= frameInterval;   // 减去已消耗的时间
        }

        // 渲染当前帧
        render();

        // 计算并控制帧率
        double frameTime = (double)(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
        double sleepTime = frameInterval - frameTime;  // 需要休眠的时间

        // 如果还有剩余时间，进行休眠
        if (sleepTime > 0) {
            DWORD sleepMs = (DWORD)(sleepTime * 1000.0);  // 转换为毫秒

            if (sleepMs > 0) {
                Sleep(sleepMs);  // 使用Sleep函数休眠
            }
            else {
                // 如果睡眠时间太短，使用忙等待
                LONGLONG endTime = currentTime.QuadPart +
                    (LONGLONG)(sleepTime * frequency.QuadPart);
                LARGE_INTEGER now;
                do {
                    QueryPerformanceCounter(&now);  // 获取当前时间
                } while (now.QuadPart < endTime);   // 等待到目标时间
            }
        }
    }

    closegraph();  // 关闭图形窗口（实际上不会执行到这里，因为循环是无限的）
}