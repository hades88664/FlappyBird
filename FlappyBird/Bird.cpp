#include "Bird.h"
#include <graphics.h>  // EasyX图形库

// ============ Rect结构的方法实现 ============
bool Rect::CheckCollision(const Rect& other) const {
    return !(x + width < other.x ||
        x > other.x + other.width ||
        y + height < other.y ||
        y > other.y + other.height);
}

// ============ 构造函数 ============
Bird::Bird()
    : posX(100.0f)       // 初始X位置
    , posY(300.0f)       // 初始Y位置
    , velocityY(0.0f)    // 初始Y速度为0
    , gravity(980.0f)    // 重力加速度：980像素/秒²
    , jumpForce(-375.0f) // 跳跃力：向上375像素/秒
    , state(BirdState::IDLE)  // 初始状态为空闲
    , isAlive(true)      // 初始为活着
    , color(YELLOW)      // 颜色为黄色
    , radius(20.0f) {    // 半径为20像素

    // 初始化碰撞矩形
    collisionBox.width = radius * 1.5f;   // 碰撞盒宽度为半径的1.5倍
    collisionBox.height = radius * 1.5f;  // 碰撞盒高度为半径的1.5倍
    UpdateCollisionBox();  // 更新碰撞盒位置
}

// ============ 初始化小鸟 ============
void Bird::Initialize(float startX, float startY) {
    posX = startX;      // 设置初始X位置
    posY = startY;      // 设置初始Y位置
    velocityY = 0.0f;   // 重置速度
    state = BirdState::FLYING;  // 状态设为飞行
    isAlive = true;     // 设为活着
    color = YELLOW;     // 重置颜色为黄色
    UpdateCollisionBox();  // 更新碰撞盒
}

// ============ 更新小鸟状态 ============
void Bird::Update(float deltaTime) {
    if (!isAlive) return;  // 如果死亡则不更新

    // 1. 应用重力
    velocityY += gravity * deltaTime;

    // 2. 更新位置
    posY += velocityY * deltaTime;

    // 3. 更新碰撞盒
    UpdateCollisionBox();

    // 4. 根据速度更新状态
    if (velocityY < 0) {
        state = BirdState::FLYING;   // 速度为负：向上飞
    }
    else {
        state = BirdState::FALLING;  // 速度为正：向下落
    }
}

// ============ 小鸟跳跃 ============
void Bird::Jump() {
    if (!isAlive) return;  // 如果死亡则不能跳跃

    velocityY = jumpForce;       // 施加跳跃力
    state = BirdState::FLYING;   // 状态设为飞行
}

// ============ 绘制小鸟 ============
void Bird::Render() const {
    // 1. 绘制小鸟身体（圆形）
    setfillcolor(color);
    solidcircle(static_cast<int>(posX),
        static_cast<int>(posY),
        static_cast<int>(radius));

    // 2. 绘制白色眼睛
    setfillcolor(WHITE);
    solidcircle(static_cast<int>(posX + radius * 0.3),
        static_cast<int>(posY - radius * 0.2),
        static_cast<int>(radius * 0.3));

    // 3. 绘制黑色瞳孔
    setfillcolor(BLACK);
    solidcircle(static_cast<int>(posX + radius * 0.4),
        static_cast<int>(posY - radius * 0.2),
        static_cast<int>(radius * 0.15));

    // 4. 绘制橙色鸟喙（三角形）
    setfillcolor(RGB(255, 165, 0));  // 橙色

    // 定义三角形的三个顶点
    POINT beak[3] = {
        {static_cast<int>(posX + radius), static_cast<int>(posY)},  // 顶点1：鸟嘴根部
        {static_cast<int>(posX + radius * 1.5), static_cast<int>(posY - radius * 0.3)},  // 顶点2：上嘴尖
        {static_cast<int>(posX + radius * 1.5), static_cast<int>(posY + radius * 0.3)}   // 顶点3：下嘴尖
    };

    solidpolygon(beak, 3);  // 绘制实心三角形
}

// ============ 检测碰撞 ============
bool Bird::CheckCollision(const Rect& other) const {
    // 使用Rect的CheckCollision方法
    return collisionBox.CheckCollision(other);
}

// ============ 更新碰撞盒位置 ============
void Bird::UpdateCollisionBox() {
    // 碰撞盒中心与小鸟中心对齐
    collisionBox.x = posX - collisionBox.width / 2.0f;
    collisionBox.y = posY - collisionBox.height / 2.0f;
}

// ============ 重置小鸟 ============
void Bird::Reset() {
    posX = 100.0f;      // 重置X位置
    posY = 300.0f;      // 重置Y位置
    velocityY = 0.0f;   // 重置速度
    state = BirdState::FLYING;  // 设为飞行状态
    isAlive = true;     // 设为活着
    color = YELLOW;     // 重置颜色
    UpdateCollisionBox();  // 更新碰撞盒
}

// ============ 小鸟死亡 ============
void Bird::Kill() {
    isAlive = false;                 // 设为死亡
    state = BirdState::DEAD;         // 状态设为死亡
    color = RGB(128, 128, 128);     // 颜色变为灰色
}