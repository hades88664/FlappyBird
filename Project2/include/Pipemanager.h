// Pipemanager.h
#pragma once

#ifndef PIPEMANAGER_H
#define PIPEMANAGER_H

#include <graphics.h>
#include <vector>
#include "constants.h"

// 前向声明（避免循环包含）
class Bird;
class Game;

// 单个管道类
class Pipe {
private:
    float x;                    // 管道的X坐标
    float gapY;                 // 管道间隙的Y坐标（中心位置）
    int width;                  // 管道宽度（像素）
    int gapHeight;              // 管道间隙高度（像素）
    bool passed;                // 是否已被小鸟通过
    COLORREF color;             // 管道颜色
    int id;                     // 管道唯一标识符
    bool hasCoin;               // 管道是否有硬币
    float coinY;                // 硬币的Y坐标
    bool coinCollected;         // 硬币是否已被收集

public:
    // 构造函数：在指定位置创建管道
    Pipe(float startX, int pipeID);

    // 更新方法：根据速度移动管道
    void update(float speed);

    // 绘制方法：绘制管道和硬币（如果有）
    void draw() const;

    // 绘制单个管道部分（顶部或底部）
    void drawPipe(float px, float top, float bottom, bool isBottom) const;

    // 绘制硬币方法
    void drawCoin() const;

    // 获取碰撞矩形（用于碰撞检测）
    RECT getTopRect() const;     // 顶部管道的矩形
    RECT getBottomRect() const;  // 底部管道的矩形
    RECT getCoinRect() const;    // 硬币的矩形

    // 碰撞检测方法
    bool checkCollision(RECT otherRect) const;        // 检查与管道的碰撞
    bool checkCoinCollision(RECT otherRect) const;    // 检查与硬币的碰撞

    // 检查管道是否移出屏幕
    bool isOffScreen() const;

    // 获取和设置属性的方法
    float getX() const { return x; }                 // 获取X坐标
    float getGapY() const { return gapY; }           // 获取间隙Y坐标
    bool isPassed() const { return passed; }         // 检查是否已通过
    void markPassed() { passed = true; }             // 标记为已通过
    bool hasCoinAvailable() const { return hasCoin && !coinCollected; } // 是否有可收集的硬币
    void collectCoin() { coinCollected = true; }     // 收集硬币
    int getID() const { return id; }                 // 获取管道ID
};

// 管道管理器类：管理所有管道
class PipeManager {
private:
    std::vector<Pipe> pipes;    // 存储所有管道的向量

public:
    // 构造函数
    PipeManager();

    // 更新所有管道
    void update(float gameSpeed, Bird* bird, int& score, int& level,
        float& currentSpeed, float& shakeTime, float& shakeIntensity,
        Game& game);

    // 绘制所有管道
    void draw() const;

    // 绘制所有碰撞框（用于调试）
    void drawHitboxes() const;

    // 检查与任何管道的碰撞
    bool checkCollision(RECT birdRect) const;

    // 检查与任何硬币的碰撞
    bool checkCoinCollision(RECT birdRect) const;

    // 检查小鸟是否通过任何管道
    bool checkPipePassed(float birdX) const;

    // 添加新管道
    void addPipe(float startX, int pipeID);

    // 清空所有管道
    void clearPipes() { pipes.clear(); }

    // 获取管道数量
    size_t getPipeCount() const { return pipes.size(); }
};

#endif // PIPEMANAGER_H