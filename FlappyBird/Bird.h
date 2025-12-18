#pragma once// Bird.h
#pragma once

// 小鸟的状态
enum class BirdState {
    IDLE,      // 空闲（在菜单中）
    FLYING,    // 向上飞
    FALLING,   // 向下落
    DEAD       // 死亡
};

// 矩形结构，用于碰撞检测（移到这里定义）
struct Rect {
    float x;        // 左上角X坐标
    float y;        // 左上角Y坐标
    float width;    // 宽度
    float height;   // 高度

    // 检查与另一个矩形的碰撞
    bool CheckCollision(const Rect& other) const;
};

class Bird {
private:
    // ============ 物理属性 ============
    float posX;         // X坐标（像素）
    float posY;         // Y坐标（像素）
    float velocityY;    // Y方向速度（像素/秒）
    float gravity;      // 重力加速度（像素/秒?）
    float jumpForce;    // 跳跃力量（像素/秒）

    // ============ 状态 ============
    BirdState state;    // 当前状态
    bool isAlive;       // 是否活着

    // ============ 外观 ============
    int color;          // 颜色（EasyX的颜色值）
    float radius;       // 小鸟半径（像素）

    // ============ 碰撞检测 ============
    Rect collisionBox;  // 碰撞矩形

public:
    // ============ 构造函数和初始化 ============
    Bird();  // 构造函数
    void Initialize(float startX, float startY);  // 初始化小鸟

    // ============ 更新和控制 ============
    void Update(float deltaTime);  // 更新小鸟状态
    void Jump();                   // 让小鸟跳跃

    // ============ 绘制 ============
    void Render() const;  // 绘制小鸟

    // ============ 碰撞检测 ============
    bool CheckCollision(const Rect& other) const;  // 检查与其他矩形的碰撞
    Rect GetCollisionBox() const { return collisionBox; }  // 获取碰撞矩形

    // ============ 状态控制 ============
    void Reset();                     // 重置小鸟状态
    void Kill();                      // 小鸟死亡
    bool IsAlive() const { return isAlive; }        // 是否活着
    BirdState GetState() const { return state; }    // 获取状态

    // ============ 获取位置信息 ============
    float GetX() const { return posX; }  // 获取X坐标
    float GetY() const { return posY; }  // 获取Y坐标

private:
    // 更新碰撞矩形的位置（跟随小鸟位置）
    void UpdateCollisionBox();
};
