#include "pipemanager.h"
#include "game.h"
#include <cmath>

// Pipe类的构造函数
Pipe::Pipe(float startX, int pipeID) {
    x = startX;  // 设置初始X坐标（屏幕右侧）

    // 随机生成管道间隙的Y坐标
    // 150到（屏幕高度 - 地面高度 - 250）之间，确保管道不会太靠近上下边缘
    gapY = 150 + rand() % (SCREEN_HEIGHT - GROUND_HEIGHT - 250);

    width = 70;           // 管道宽度70像素
    gapHeight = 160;      // 管道间隙高度160像素（小鸟可以通过的空间）
    passed = false;       // 初始状态为未通过
    id = pipeID;          // 设置管道ID

    // 30%的几率生成带硬币的管道
    hasCoin = (rand() % 100) < 30;
    coinY = gapY;         // 硬币在管道间隙的中间
    coinCollected = false; // 硬币初始状态为未收集

    // 随机选择管道颜色（4种颜色之一）
    int colorType = rand() % 4;  // 生成0-3的随机数
    switch (colorType) {
    case 0: color = COLOR_PIPE_GREEN; break;   // 绿色管道
    case 1: color = COLOR_PIPE_BLUE; break;    // 蓝色管道
    case 2: color = COLOR_PIPE_PURPLE; break;  // 紫色管道
    case 3: color = COLOR_PIPE_RED; break;     // 红色管道
    }
}

// 更新方法：向左移动管道
void Pipe::update(float speed) {
    x -= speed;  // 根据游戏速度向左移动
}

// 绘制方法：绘制整个管道（包括顶部、底部和硬币）
void Pipe::draw() const {
    // 绘制顶部管道（从屏幕顶部到间隙顶部）
    drawPipe(x, 0, gapY - gapHeight / 2, false);

    // 绘制底部管道（从间隙底部到地面）
    drawPipe(x, gapY + gapHeight / 2, SCREEN_HEIGHT - GROUND_HEIGHT, true);

    // 如果管道有硬币且未被收集，绘制硬币
    if (hasCoin && !coinCollected) {
        drawCoin();
    }
}

// 绘制单个管道部分（顶部或底部）
void Pipe::drawPipe(float px, float top, float bottom, bool isBottom) const {
    // 绘制管道主体（矩形）
    setfillcolor(color);  // 设置管道颜色
    // 填充管道矩形（从px到px+width，从top到bottom）
    fillrectangle((int)px, (int)top, (int)(px + width), (int)bottom);

    // 绘制管道顶部/底部的盖子（稍暗的颜色，增加立体感）
    COLORREF capColor = RGB(
        (int)(GetRValue(color) * 0.8),  // R通道乘以0.8（变暗）
        (int)(GetGValue(color) * 0.8),  // G通道乘以0.8
        (int)(GetBValue(color) * 0.8)   // B通道乘以0.8
    );
    setfillcolor(capColor);  // 设置盖子颜色

    // 根据是顶部管道还是底部管道，绘制不同位置的盖子
    if (!isBottom) {
        // 顶部管道：在底部绘制盖子（向下突出）
        fillrectangle((int)px - 10, (int)bottom - 20,
            (int)(px + width + 10), (int)bottom);
    }
    else {
        // 底部管道：在顶部绘制盖子（向上突出）
        fillrectangle((int)px - 10, (int)top,
            (int)(px + width + 10), (int)top + 20);
    }

    // 绘制管道纹理（水平条纹，增加细节）
    setfillcolor(RGB(
        (int)(GetRValue(color) * 0.6),  // 更暗的颜色
        (int)(GetGValue(color) * 0.6),
        (int)(GetBValue(color) * 0.6)
    ));

    int textureSpacing = 25;  // 纹理间距25像素

    if (!isBottom) {
        // 顶部管道：从顶部开始绘制水平条纹
        for (int y = (int)top + 10; y < (int)bottom - 25; y += textureSpacing) {
            fillrectangle((int)px + 10, y,
                (int)(px + width - 10), y + 10);  // 绘制一个条纹
        }
    }
    else {
        // 底部管道：从顶部+30开始绘制水平条纹
        for (int y = (int)top + 30; y < (int)bottom - 10; y += textureSpacing) {
            fillrectangle((int)px + 10, y,
                (int)(px + width - 10), y + 10);  // 绘制一个条纹
        }
    }
}

// 绘制硬币方法
void Pipe::drawCoin() const {
    float coinX = x + width / 2;  // 硬币在管道中间的X坐标

    // 绘制硬币主体（金色圆形）
    setfillcolor(RGB(255, 215, 0));  // 金色填充
    setlinecolor(RGB(218, 165, 32)); // 深金色边框
    solidcircle((int)coinX, (int)coinY, 12);  // 绘制12像素半径的圆形

    // 绘制硬币外圈（亮金色边框）
    setlinecolor(RGB(255, 255, 0));  // 亮黄色边框
    circle((int)coinX, (int)coinY, 12);  // 绘制圆形边框

    // 在硬币上绘制"$"符号
    settextstyle(14, 0, _T("Arial"));  // 14号字体
    settextcolor(RGB(255, 255, 255));  // 白色文字
    setbkmode(TRANSPARENT);            // 透明背景
    outtextxy((int)coinX - 4, (int)coinY - 7, L"$");  // 居中显示$符号
}

// 获取顶部管道的碰撞矩形
RECT Pipe::getTopRect() const {
    RECT rect;
    rect.left = (int)x;                     // 左边界：管道X坐标
    rect.top = 0;                           // 上边界：屏幕顶部
    rect.right = (int)(x + width);          // 右边界：管道X坐标加宽度
    rect.bottom = (int)(gapY - gapHeight / 2); // 下边界：间隙顶部
    return rect;
}

// 获取底部管道的碰撞矩形
RECT Pipe::getBottomRect() const {
    RECT rect;
    rect.left = (int)x;                             // 左边界：管道X坐标
    rect.top = (int)(gapY + gapHeight / 2);         // 上边界：间隙底部
    rect.right = (int)(x + width);                  // 右边界：管道X坐标加宽度
    rect.bottom = SCREEN_HEIGHT - GROUND_HEIGHT;    // 下边界：地面顶部
    return rect;
}

// 获取硬币的碰撞矩形
RECT Pipe::getCoinRect() const {
    RECT rect;
    // 硬币矩形：以硬币为中心，12像素为半径的正方形
    rect.left = (int)(x + width / 2 - 12);   // 左边界：硬币X坐标减12
    rect.top = (int)coinY - 12;              // 上边界：硬币Y坐标减12
    rect.right = (int)(x + width / 2 + 12);  // 右边界：硬币X坐标加12
    rect.bottom = (int)coinY + 12;           // 下边界：硬币Y坐标加12
    return rect;
}

// 检查与指定矩形的碰撞（顶部或底部管道）
bool Pipe::checkCollision(RECT otherRect) const {
    RECT topPipe = getTopRect();     // 获取顶部管道矩形
    RECT bottomPipe = getBottomRect(); // 获取底部管道矩形

    // 检查与顶部管道的碰撞
    bool hitTop = (otherRect.right > topPipe.left &&   // 小鸟右边界 > 管道左边界
        otherRect.left < topPipe.right &&   // 小鸟左边界 < 管道右边界
        otherRect.bottom > topPipe.top &&   // 小鸟下边界 > 管道上边界
        otherRect.top < topPipe.bottom);    // 小鸟上边界 < 管道下边界

    // 检查与底部管道的碰撞
    bool hitBottom = (otherRect.right > bottomPipe.left &&
        otherRect.left < bottomPipe.right &&
        otherRect.bottom > bottomPipe.top &&
        otherRect.top < bottomPipe.bottom);

    // 如果与顶部或底部管道碰撞，返回true
    return hitTop || hitBottom;
}

// 检查与硬币的碰撞
bool Pipe::checkCoinCollision(RECT otherRect) const {
    // 如果没有硬币或硬币已被收集，返回false
    if (!hasCoin || coinCollected) return false;

    RECT coinRect = getCoinRect();  // 获取硬币矩形

    // 检查矩形是否相交
    return (otherRect.right > coinRect.left &&    // 小鸟右边界 > 硬币左边界
        otherRect.left < coinRect.right &&    // 小鸟左边界 < 硬币右边界
        otherRect.bottom > coinRect.top &&    // 小鸟下边界 > 硬币上边界
        otherRect.top < coinRect.bottom);     // 小鸟上边界 < 硬币下边界
}

// 检查管道是否完全移出屏幕
bool Pipe::isOffScreen() const {
    return x + width < 0;  // 管道右边界 < 屏幕左边界（0）
}

// PipeManager类的构造函数
PipeManager::PipeManager() {
    pipes.clear();  // 初始化时清空管道向量
}

// 更新所有管道
void PipeManager::update(float gameSpeed, Bird* bird, int& score, int& level,
    float& currentSpeed, float& shakeTime, float& shakeIntensity,
    Game& game) {
    // 遍历所有管道
    for (auto& pipe : pipes) {
        pipe.update(gameSpeed);  // 移动管道
    }

    // 移除移出屏幕的管道
    auto it = pipes.begin();
    while (it != pipes.end()) {
        if (it->isOffScreen()) {
            it = pipes.erase(it);  // 删除管道，返回下一个迭代器
        }
        else {
            ++it;  // 移动到下一个管道
        }
    }
}

// 绘制所有管道
void PipeManager::draw() const {
    // 遍历所有管道并绘制
    for (const auto& pipe : pipes) {
        pipe.draw();
    }
}

// 绘制所有管道的碰撞框（用于调试）
void PipeManager::drawHitboxes() const {
    // 遍历所有管道
    for (const auto& pipe : pipes) {
        // 获取顶部和底部管道的碰撞矩形
        RECT topRect = pipe.getTopRect();
        RECT bottomRect = pipe.getBottomRect();

        // 绘制顶部管道碰撞框
        rectangle(topRect.left, topRect.top, topRect.right, topRect.bottom);

        // 绘制底部管道碰撞框
        rectangle(bottomRect.left, bottomRect.top, bottomRect.right, bottomRect.bottom);
    }
}

// 检查与任何管道的碰撞
bool PipeManager::checkCollision(RECT birdRect) const {
    // 遍历所有管道
    for (const auto& pipe : pipes) {
        // 如果与当前管道碰撞，返回true
        if (pipe.checkCollision(birdRect)) {
            return true;
        }
    }
    return false;  // 没有与任何管道碰撞
}

// 检查与任何硬币的碰撞
bool PipeManager::checkCoinCollision(RECT birdRect) const {
    // 遍历所有管道
    for (auto& pipe : const_cast<std::vector<Pipe>&>(pipes)) {
        // 如果与当前管道的硬币碰撞
        if (pipe.checkCoinCollision(birdRect)) {
            pipe.collectCoin();  // 收集硬币
            return true;         // 返回true表示发生碰撞
        }
    }
    return false;  // 没有与任何硬币碰撞
}

// 检查小鸟是否通过任何管道
bool PipeManager::checkPipePassed(float birdX) const {
    bool passedAny = false;  // 标记是否通过了任何管道

    // 遍历所有管道
    for (const auto& pipe : pipes) {
        // 如果管道未被标记为已通过，且小鸟的X坐标大于管道的右边界
        if (!pipe.isPassed() && pipe.getX() + 70 < birdX) {
            const_cast<Pipe&>(pipe).markPassed();  // 标记为已通过（需要const_cast）
            passedAny = true;  // 设置标记为true
        }
    }

    return passedAny;  // 返回是否通过了任何管道
}

// 添加新管道
void PipeManager::addPipe(float startX, int pipeID) {
    pipes.push_back(Pipe(startX, pipeID));  // 在向量末尾添加新管道
}