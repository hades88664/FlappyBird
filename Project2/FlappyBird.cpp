// FlappyBird.cpp - 游戏主程序入口文件
// 编译此文件将生成可执行的Flappy Bird游戏

// 预处理器指令：禁用某些Visual Studio安全警告
#define _CRT_SECURE_NO_WARNINGS  // 允许使用一些较旧的C函数

// 包含必要的头文件
#include "game.h"  // 游戏主类头文件
#include <iostream> // 标准输入输出流（用于控制台输出）
#include <cstdlib>  // 标准库函数（system函数）

// main函数：程序入口点
int main() {
    // 设置控制台代码页为UTF-8，支持中文字符显示
    system("chcp 65001 > nul");
    // 说明：这条命令只在Windows命令提示符中有效
    // "chcp 65001" 将控制台代码页设置为UTF-8
    // "> nul" 将命令输出重定向到空设备，避免显示在屏幕上

    // 在控制台显示游戏标题和特性介绍
    std::cout << "=============================================" << std::endl;
    std::cout << "      FLAPPY BIRD ULTIMATE EDITION v2.0" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "  • Enhanced Graphics & Animations" << std::endl;
    std::cout << "  • Particle Effects System" << std::endl;
    std::cout << "  • Combo Multiplier System" << std::endl;
    std::cout << "  • Coin Collection Mechanic" << std::endl;
    std::cout << "  • Multiple Difficulty Levels" << std::endl;
    std::cout << "  • Screen Shake Effects" << std::endl;
    std::cout << "  • Persistent Leaderboard" << std::endl;
    std::cout << "  • Customizable Settings" << std::endl;
    std::cout << "  • Dynamic Sky & Cloud System" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "Starting game..." << std::endl;

    // 创建游戏对象
    // Game类是整个游戏的核心，包含所有游戏逻辑和状态
    Game game;

    // 运行游戏主循环
    // run()方法将启动图形窗口并进入游戏循环
    // 游戏循环将一直运行直到玩家退出游戏
    game.run();

    // 返回0表示程序正常退出
    // 注意：实际上game.run()通常不会返回，因为游戏循环是无限的
    // 直到玩家选择退出游戏时，才会执行到这里的return语句
    return 0;
}