#include "../include/InputHandler.h"

void InputHandler::update() {
    // 备份上一帧状态
    previousKeys = currentKeys;

    // 更新当前帧状态（VK_SPACE 和 VK_ESCAPE）
    // 注意：这里的 keys 数组要在头文件的类定义里声明
    int keysToMonitor[] = { VK_SPACE, VK_ESCAPE };
    for (int k : keysToMonitor) {
        currentKeys[k] = (GetAsyncKeyState(k) & 0x8000) != 0;
    }
}

// 刚刚按下的判断（用于跳跃）
bool InputHandler::isKeyPressed(int vKey) const {
    if (currentKeys.count(vKey) && previousKeys.count(vKey)) {
        return currentKeys.at(vKey) && !previousKeys.at(vKey);
    }
    return false;
}

// 【补全这里】解决 VCR001 警告：判断按键是否正被按住
bool InputHandler::isKeyDown(int vKey) const {
    if (currentKeys.count(vKey)) {
        return currentKeys.at(vKey);
    }
    return false;
}