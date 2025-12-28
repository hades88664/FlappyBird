#pragma once
#include <SFML/Audio.hpp>
#include <map>
#include <string>
#include <vector>

class AudioManager {
public:
    // 获取单例
    static AudioManager& getInstance() {
        static AudioManager instance;
        return instance;
    }

    // 禁用拷贝
    AudioManager(const AudioManager&) = delete;
    void operator=(const AudioManager&) = delete;

    // --- 音效方法 ---
    bool loadSoundEffect(const std::string& name, const std::string& filepath);
    void playSound(const std::string& name, float volume = 100.0f, float pitch = 1.0f);

    // --- 音乐方法 ---
    bool playMusic(const std::string& filepath, bool loop = true);
    void stopMusic();
    void pauseMusic();   // 确保这一行存在
    void resumeMusic();  // 确保这一行存在

    // --- 音量控制 ---
    void setMasterVolume(float volume);
    void setSoundsVolume(float volume);
    void setMusicVolume(float volume);

    float getMasterVolume() const;
    float getSoundsVolume() const;
    float getMusicVolume() const;
    bool isMusicPlaying() const;

    void cleanup();

private:
    AudioManager();
    ~AudioManager();
    void cleanFinishedSounds();

    std::map<std::string, sf::SoundBuffer> soundBuffers;
    std::vector<sf::Sound> activeSounds;
    sf::Music backgroundMusic;

    float masterVolume;
    float soundsVolume;
    float musicVolume;

}; // <--- 【核心】这里必须有这个分号，否则报 E0135 和 E1670