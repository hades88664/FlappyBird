#include <graphics.h>  // EasyX图形库头文件
#include <conio.h>     // 控制台输入输出头文件
#include <stdio.h>     // 添加这个头文件以使用printf
#include "Game.h"      // 游戏主控类

int main() {
    Game game;

    if (!game.Initialize()) {
        printf("游戏初始化失败！\n");
        return -1;
    }

    game.Run();

    // 关闭批量绘制
    EndBatchDraw();

    closegraph();
    return 0;
}