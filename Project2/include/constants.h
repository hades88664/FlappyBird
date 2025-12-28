#pragma once
// constants.h
#pragma once

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <graphics.h>

// 屏幕尺寸
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GROUND_HEIGHT 60

// 游戏状态
enum GameState {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_LEADERBOARD,
    STATE_SETTINGS,
    STATE_HELP,
    STATE_CREDITS
};

// 颜色常量
#define COLOR_SKY_START RGB(135, 206, 235)     // 浅蓝色（天空顶部）
#define COLOR_SKY_END RGB(70, 130, 180)        // 深蓝色（天空底部）
#define COLOR_GROUND RGB(222, 184, 135)        // 土黄色（地面）
#define COLOR_GRASS RGB(124, 252, 0)           // 亮绿色（草）
#define COLOR_BIRD_BODY RGB(255, 165, 0)       // 橙色（小鸟身体）
#define COLOR_BIRD_WING RGB(255, 140, 0)       // 深橙色（小鸟翅膀）
#define COLOR_BIRD_EYE RGB(255, 255, 255)      // 白色（小鸟眼睛）
#define COLOR_BIRD_BEAK RGB(255, 69, 0)        // 红色（小鸟喙）
#define COLOR_PIPE_GREEN RGB(0, 180, 0)        // 绿色管道
#define COLOR_PIPE_BLUE RGB(0, 120, 255)       // 蓝色管道
#define COLOR_PIPE_PURPLE RGB(160, 32, 240)    // 紫色管道
#define COLOR_PIPE_RED RGB(220, 20, 60)        // 红色管道
#define COLOR_TEXT_WHITE RGB(255, 255, 255)    // 白色文字
#define COLOR_TEXT_RED RGB(255, 0, 0)          // 红色文字
#define COLOR_TEXT_BLUE RGB(0, 120, 255)       // 蓝色文字
#define COLOR_TEXT_YELLOW RGB(255, 255, 0)     // 黄色文字
#define COLOR_TEXT_GREEN RGB(0, 255, 0)        // 绿色文字
#define COLOR_TEXT_PURPLE RGB(160, 32, 240)    // 紫色文字

// 游戏常量
#define FPS 60.0

#endif // CONSTANTS_H