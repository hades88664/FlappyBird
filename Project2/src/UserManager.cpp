#include "../include/game.h"
#include <fstream>
#include <iostream>

// 构造函数
UserManager::UserManager() : currentUser(nullptr) {
    loadUsers();
}

// 加载用户信息
void UserManager::loadUsers() {
    users.clear();
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        UserData user;
        std::string unlockedStr;

        // 格式：用户名 总分 最高分 解锁项(逗号隔开)
        ss >> user.name >> user.totalScore >> user.highScore >> unlockedStr;

        // 解析解锁项
        std::stringstream ssItems(unlockedStr);
        std::string item;
        while (std::getline(ssItems, item, ',')) {
            if (!item.empty()) user.unlockedItems.insert(item);
        }
        users.push_back(user);
    }
    file.close();
}

// 保存用户信息
void UserManager::saveUsers() {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    for (const auto& user : users) {
        file << user.name << " "
            << user.totalScore << " "
            << user.highScore << " ";

        // 保存解锁项，逗号隔开
        bool first = true;
        for (const auto& item : user.unlockedItems) {
            if (!first) file << ",";
            file << item;
            first = false;
        }
        if (user.unlockedItems.empty()) file << "none"; // 防止字段为空
        file << std::endl;
    }
    file.close();
}

// 登录/创建账户
// 返回值：0-登录成功，1-新创建成功，2-同名且已在列表中（冲突提示）
int UserManager::login(const std::string& name) {
    for (auto& user : users) {
        if (user.name == name) {
            currentUser = &user;
            return 0; // 找到旧用户，登录成功
        }
    }

    // 没找到，创建新用户
    UserData newUser;
    newUser.name = name;
    newUser.totalScore = 0;
    newUser.highScore = 0;
    newUser.unlockedItems.insert("menu_bgm"); // 赠送初始BGM
    newUser.unlockedItems.insert("jump_sfx"); // 赠送初始音效

    users.push_back(newUser);
    currentUser = &users.back(); // 指向新创建的用户
    saveUsers(); // 立即存盘一次
    return 1;
}

// 检查物品是否解锁
bool UserManager::isItemUnlocked(const std::string& itemID) {
    if (!currentUser) return false;
    return currentUser->unlockedItems.count(itemID) > 0;
}

// 解锁物品
bool UserManager::unlockItem(const std::string& itemID, int cost) {
    if (!currentUser) return false;

    // 如果已经解锁，直接返回true
    if (isItemUnlocked(itemID)) return true;

    // 检查分数是否足够
    if (currentUser->totalScore >= cost) {
        currentUser->totalScore -= cost;
        currentUser->unlockedItems.insert(itemID);
        saveUsers();
        return true;
    }

    return false; // 分数不足
}