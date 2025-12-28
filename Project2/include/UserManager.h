#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <set>

// 用户数据结构
struct UserData {
    std::string name;
    int totalScore;         // 总得分（累计，用于兑换）
    int highScore;         // 历史单局最高分
    std::set<std::string> unlockedItems; // 已解锁的资源ID（音乐、音效）

    UserData() : name(""), totalScore(0), highScore(0) {
        unlockedItems.insert("default_bgm"); // 默认资源
        unlockedItems.insert("default_sfx");
    }
};

class UserManager {
private:
    std::vector<UserData> users;
    UserData* currentUser;
    const std::string filename = "users.dat";

public:
    UserManager() : currentUser(nullptr) { loadUsers(); }
    ~UserManager() { saveUsers(); }

    void loadUsers();
    void saveUsers();

    // 登录或创建：返回0-登录成功，1-新用户创建，2-重名提示
    int login(const std::string& name);

    UserData* getCurrentUser() { return currentUser; }
    std::vector<UserData>& getAllUsers() { return users; }

    bool isItemUnlocked(const std::string& itemID) {
        return currentUser && currentUser->unlockedItems.count(itemID);
    }

    bool unlockItem(const std::string& itemID, int cost) {
        if (currentUser && currentUser->totalScore >= cost) {
            currentUser->totalScore -= cost;
            currentUser->unlockedItems.insert(itemID);
            saveUsers();
            return true;
        }
        return false;
    }
};