#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <SFML/Audio.hpp>
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <map>
#include <vector>
#include <list> 

#pragma comment(lib, "Winmm.lib")

// 1. 新增：音频资产结构，用于商店展示
struct AudioAsset {
    std::string id;          // 唯一ID，如 "bgm_jazz"
    std::string path;        // 文件路径
    std::wstring displayName; // 商店显示的名字
    int cost;                // 兑换所需分数
    bool isBGM;              // 是背景音乐还是短音效
};

class AudioManager {
private:
    static AudioManager* instance;

    // 资源存储
    std::map<std::string, AudioAsset> library;           // 资产元数据
    std::map<std::string, sf::SoundBuffer> soundBuffers; // SFX 缓存

    // SFML 3.0 建议使用 list 存储播放中的声音，防止内存重分配导致声音崩溃
    std::list<sf::Sound> activeSounds;

    sf::Music backgroundMusic;                          // 背景音乐流

    // 音量设置 (0.0f - 100.0f)
    float masterVolume;
    float musicVolume;
    float sfxVolume;

    AudioManager(); // 构造函数

public:
    static AudioManager& getInstance();
    ~AudioManager();

    // --- 核心功能：注册与播放 ---
    // 游戏初始化时调用，一次性加载
    void registerAudio(std::string id, std::string path, std::wstring name, int cost, bool isBGM);

    void playSFX(const std::string& id);
    void playBGM(const std::string& id);
    void stopBGM();

    // --- 音量控制 ---
    void setMasterVolume(float vol);
    void setMusicVolume(float vol);
    void setSoundsVolume(float vol);

    float getMasterVolume() const { return masterVolume; }
    float getMusicVolume() const { return musicVolume; }
    float getSoundsVolume() const { return sfxVolume; }

    // --- 系统管理 ---
    void cleanFinishedSounds(); // 每帧清理已播放完的音效
    void cleanup();             // 退出时释放资源
    const std::map<std::string, AudioAsset>& getLibrary() const { return library; }
};
#endif