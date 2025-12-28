#pragma once
#include <Windows.h>
#include <map>

class InputHandler {
public:
    // 单例模式，方便全局访问
    static InputHandler& getInstance() {
        static InputHandler instance;
        return instance;
    }

    // 每帧调用一次，更新按键状态
    void update();

    // 判断按键是否“刚刚按下”（仅在按下的那一帧返回 true）
    bool isKeyPressed(int vKey) const;

    // 判断按键是否“正被按住”
    bool isKeyDown(int vKey) const;

private:
    InputHandler() = default;

    // 存储当前帧和上一帧的按键状态
    std::map<int, bool> currentKeys;
    std::map<int, bool> previousKeys;

    // 需要监听的按键列表
    const int keysToMonitor[2] = { VK_SPACE, VK_ESCAPE };
};