#include "../include/bird.h"
#include <cmath>
#include <string>

// 小鸟构造函数：初始化所有成员变量
Bird::Bird() {
    reset();  // 调用重置方法进行初始化
}

// 重置方法：将小鸟恢复到游戏开始时的状态
void Bird::reset() {
    x = SCREEN_WIDTH / 4;             // X坐标：屏幕宽度的1/4处
    y = SCREEN_HEIGHT / 2;            // Y坐标：屏幕高度的中间
    velocity = 0;                     // 初始速度为0
    gravity = 0.5f;                   // 默认重力值
    jumpForce = -8.5f;                // 默认跳跃力量（负值表示向上）
    radius = 15;                      // 小鸟半径15像素
    alive = true;                     // 初始状态为存活
    rotation = 0;                     // 初始旋转角度为0
    wingAngle = 0;                    // 翅膀角度从0开始
    wingSpeed = 0.3f;                 // 翅膀扇动速度
    scoreMultiplier = 1;              // 初始分数倍数为1
    color = COLOR_BIRD_BODY;          // 小鸟身体颜色（橙色）
    comboCount = 0;                   // 初始连击次数为0
    comboTime = 0;                    // 连击时间为0
}

// 更新方法：根据时间更新小鸟的物理状态和动画
void Bird::update(float deltaTime) {
    if (!alive) return;  // 如果小鸟死亡，不进行更新

    // 物理模拟：应用重力
    velocity += gravity;  // 速度增加重力加速度
    y += velocity;        // 根据速度更新Y坐标

    // 翅膀动画：随时间改变翅膀角度
    wingAngle += wingSpeed;  // 增加翅膀角度
    if (wingAngle > 6.28318f) wingAngle = 0;  // 如果超过2π，重置为0（完整周期）

    // 旋转效果：根据速度计算小鸟的旋转角度
    rotation = velocity * 3;  // 速度乘以3作为旋转角度（比例系数）
    if (rotation > 30) rotation = 30;      // 限制最大旋转角度为30度
    if (rotation < -30) rotation = -30;    // 限制最小旋转角度为-30度

    // 连击系统：更新连击时间
    if (comboTime > 0) {
        comboTime -= deltaTime;  // 减少连击剩余时间
        if (comboTime <= 0) {    // 如果连击时间结束
            comboCount = 0;      // 重置连击次数
            scoreMultiplier = 1; // 重置分数倍数为1
        }
    }

    // 边界检查：防止小鸟飞出屏幕上下边界
    if (y < radius) {            // 如果碰到上边界
        y = radius;              // 固定在小鸟半径位置
        velocity = 0;            // 重置速度
    }
    // 如果碰到地面（屏幕高度减去地面高度减去小鸟半径）
    if (y > SCREEN_HEIGHT - GROUND_HEIGHT - radius) {
        y = SCREEN_HEIGHT - GROUND_HEIGHT - radius;  // 固定在地面位置
        velocity = 0;                                // 重置速度
    }
}

// 跳跃方法：给小鸟一个向上的速度
void Bird::jump() {
    if (alive) {                  // 只有存活的小鸟才能跳跃
        velocity = jumpForce;     // 设置速度为跳跃力量（向上）
        wingAngle = 0;            // 重置翅膀角度，开始新的扇动周期
    }
}

// 绘制方法：绘制小鸟的所有部分
void Bird::draw() const {
    if (!alive) return;  // 如果小鸟死亡，不进行绘制

    setlinestyle(PS_SOLID, 2);  // 设置线条样式为实线，宽度2像素

    // 绘制小鸟身体（主圆形）
    setfillcolor(color);  // 设置填充颜色为小鸟身体颜色
    // 设置边框颜色为身体颜色的80%（稍暗）
    setlinecolor(RGB(GetRValue(color) * 0.8,
        GetGValue(color) * 0.8,
        GetBValue(color) * 0.8));
    solidcircle((int)x, (int)y, radius);  // 绘制实心圆作为身体

    // 绘制小鸟翅膀（动态扇动效果）
    float wingOffset = sin(wingAngle * 4) * 5;  // 使用正弦函数计算翅膀偏移
    setfillcolor(COLOR_BIRD_WING);              // 设置翅膀颜色
    // 绘制翅膀圆形（位置在身体左下方，随wingOffset动态移动）
    solidcircle((int)(x - radius * 0.7),
        (int)(y + wingOffset),
        (int)(radius * 0.8));

    // 绘制小鸟眼睛（白色眼白和黑色瞳孔）
    setfillcolor(COLOR_BIRD_EYE);  // 白色眼白
    solidcircle((int)(x + radius * 0.5),  // 眼睛在身体右上方
        (int)(y - radius * 0.3),
        (int)(radius * 0.4));

    setfillcolor(RGB(0, 0, 0));  // 黑色瞳孔
    solidcircle((int)(x + radius * 0.7),  // 瞳孔在眼白右上方
        (int)(y - radius * 0.3),
        (int)(radius * 0.2));

    // 绘制眼睛高光（小白色圆形，增加立体感）
    setfillcolor(COLOR_BIRD_EYE);  // 白色高光
    solidcircle((int)(x + radius * 0.65),
        (int)(y - radius * 0.35),
        (int)(radius * 0.08));

    // 绘制小鸟喙（橙色三角形）
    setfillcolor(COLOR_BIRD_BEAK);  // 喙的颜色
    // 定义喙的三个顶点坐标（三角形）
    POINT beak[3] = {
        {(int)(x + radius), (int)y},          // 顶点1：身体右侧中间
        {(int)(x + radius + 20), (int)(y - 7)}, // 顶点2：向右上延伸
        {(int)(x + radius + 20), (int)(y + 7)}  // 顶点3：向右下延伸
    };
    solidpolygon(beak, 3);  // 绘制实心三角形

    // 绘制小鸟脸颊（粉色圆形，增加可爱感）
    setfillcolor(RGB(255, 182, 193));  // 浅粉色
    setlinecolor(RGB(255, 182, 193));  // 边框同样颜色
    solidcircle((int)(x + radius * 0.2),  // 脸颊在身体右下方
        (int)(y + radius * 0.4),
        (int)(radius * 0.3));

    // 绘制小鸟尾巴（三角形）
    POINT tail[4] = {
        {(int)(x - radius), (int)y},          // 起点：身体左侧中间
        {(int)(x - radius - 15), (int)(y - 8)}, // 向左上延伸
        {(int)(x - radius - 15), (int)(y + 8)}, // 向左下延伸
        {(int)(x - radius), (int)y}           // 回到起点（形成闭合）
    };
    solidpolygon(tail, 4);  // 绘制实心四边形（三角形加一个点）

    // 如果有连击效果，绘制连击显示
    if (comboTime > 0) {
        drawComboEffect();
    }
}

// 绘制连击效果：在小鸟上方显示连击信息
void Bird::drawComboEffect() const {
    wchar_t wcomboText[20];  // 宽字符数组（Windows图形库需要宽字符）
    // 将多字节字符串转换为宽字符字符串
    swprintf_s(wcomboText, 20, L"COMBO x%d", comboCount);

    // 获取文本的宽度和高度（用于居中显示）
    int textWidth = textwidth(wcomboText);
    int textHeight = textheight(wcomboText);

    // 设置连击文本的样式
    settextstyle(16, 0, _T("Arial"));  // 16号Arial字体
    settextcolor(RGB(255, 215, 0));    // 金色文字
    setbkmode(TRANSPARENT);            // 透明背景

    // 在小鸟上方显示连击文本（居中）
    outtextxy((int)x - textWidth / 2,          // X坐标：小鸟X坐标减去一半文字宽度
        (int)y - radius - textHeight - 5, // Y坐标：小鸟上方（减去半径和文字高度）
        wcomboText);                      // 要显示的文本
}

// 获取碰撞矩形：返回小鸟用于碰撞检测的区域
RECT Bird::getCollisionRect() const {
    RECT rect;          // Windows矩形结构体
    int margin = 3;     // 边距（让碰撞框比实际小鸟稍小一点，提高游戏体验）

    // 设置矩形的四个边界
    rect.left = (int)x - radius + margin;     // 左边界：X坐标减去半径加边距
    rect.top = (int)y - radius + margin;      // 上边界：Y坐标减去半径加边距
    rect.right = (int)x + radius - margin;    // 右边界：X坐标加上半径减边距
    rect.bottom = (int)y + radius - margin;   // 下边界：Y坐标加上半径减边距

    return rect;  // 返回矩形
}

// 死亡方法：标记小鸟为死亡状态
void Bird::kill() {
    alive = false;                     // 设置存活状态为false
    color = RGB(128, 128, 128);        // 将小鸟颜色改为灰色（表示死亡）
}

// 增加连击：当小鸟成功通过管道时调用
void Bird::addCombo() {
    comboCount++;          // 增加连击次数
    comboTime = 2.0f;      // 重置连击时间（2秒内继续通过管道可以保持连击）
    scoreMultiplier = 1 + comboCount / 3;  // 计算新的分数倍数（每3次连击增加1倍）
}