// Bird.h
#pragma once

#ifndef BIRD_H
#define BIRD_H

#include <graphics.h>
#include <vector>
#include <string>
#include "constants.h"

// 小鸟类：负责管理小鸟的所有行为和状态
class Bird {
private:
    float x, y;              // 小鸟的X和Y坐标（浮点数确保平滑移动）
    float velocity;          // 小鸟的垂直速度（正值向下，负值向上）
    float gravity;           // 重力加速度（影响小鸟下落的速度）
    float jumpForce;         // 跳跃力量（负值，表示向上的力量）
    int radius;              // 小鸟的半径（用于绘制和碰撞检测）
    bool alive;              // 小鸟是否存活（true=存活，false=死亡）
    float rotation;          // 小鸟的旋转角度（根据速度动态变化）
    float wingAngle;         // 翅膀的角度（用于翅膀扇动动画）
    float wingSpeed;         // 翅膀扇动的速度
    int scoreMultiplier;     // 分数倍数（连击系统）
    COLORREF color;          // 小鸟的身体颜色
    int comboCount;          // 当前连击次数
    float comboTime;         // 连击剩余时间（秒）

public:
    // 构造函数：创建小鸟对象
    Bird();

    // 重置方法：将小鸟恢复到初始状态
    void reset();

    // 更新方法：根据时间更新小鸟的状态
    void update(float deltaTime);

    // 跳跃方法：让小鸟向上跳跃
    void jump();

    // 绘制方法：绘制小鸟（包括身体、翅膀、眼睛、喙等）
    void draw() const;

    // 绘制连击效果：显示连击信息
    void drawComboEffect() const;

    // 获取碰撞矩形：返回小鸟的碰撞检测区域
    RECT getCollisionRect() const;

    // 死亡方法：标记小鸟为死亡状态
    void kill();

    // 增加连击：当小鸟通过管道时调用
    void addCombo();

    // 获取和设置属性的方法（访问器和修改器）
    float getX() const { return x; }                 // 获取X坐标
    float getY() const { return y; }                 // 获取Y坐标
    bool isAlive() const { return alive; }           // 检查是否存活
    bool isDead() const { return !alive; }           // 检查是否死亡
    int getScoreMultiplier() const { return scoreMultiplier; } // 获取分数倍数
    int getComboCount() const { return comboCount; } // 获取连击次数

    // 设置重力：调整小鸟下落的速度
    void setGravity(float g) { gravity = g; }

    // 设置跳跃力量：调整小鸟跳跃的高度
    void setJumpForce(float f) { jumpForce = f; }

    // 获取当前重力值
    float getGravity() const { return gravity; }

    // 获取当前跳跃力量值
    float getJumpForce() const { return jumpForce; }
};

#endif // BIRD_H