#include "../include/AudioManager.h" 
#include <algorithm>
#include <iostream>
#include "../include/constants.h"
#include <SFML/Audio.hpp>
#include <filesystem>

AudioManager* AudioManager::instance = nullptr;

AudioManager::AudioManager()
    : masterVolume(100.0f), musicVolume(100.0f), sfxVolume(100.0f) {
}

AudioManager::~AudioManager() {
    cleanup();
}

AudioManager& AudioManager::getInstance() {
    if (instance == nullptr) {
        instance = new AudioManager();
    }
    return *instance;
}

// 注册并加载资源
void AudioManager::registerAudio(std::string id, std::string path, std::wstring name, int cost, bool isBGM) {
    // 1. 存入图书馆元数据
    library[id] = { id, path, name, cost, isBGM };

    // 2. 如果是短音效，预加载到内存缓存 buffer 中
    if (!isBGM) {
        namespace fs = std::filesystem;
        std::error_code ec;
        bool exists = fs::exists(path, ec);
        if (!exists) {
            if (ec) {
                std::cerr << "registerAudio: exists check error for '" << path << "': " << ec.message() << std::endl;
            }
            else {
                std::cerr << "registerAudio: SFX file not found: " << path << std::endl;
            }
        }

        sf::SoundBuffer buffer;
        if (buffer.loadFromFile(path)) {
            // 使用 move/ emplacement，尽量避免不必要的拷贝
            soundBuffers.emplace(id, std::move(buffer));
            std::cerr << "registerAudio: Loaded SFX '" << id << "' from " << path << std::endl;
        }
        else {
            std::cerr << "registerAudio: Failed to load SFX: " << path << " (loadFromFile returned false)" << std::endl;
        }
    }
    else {
        std::cerr << "registerAudio: Registered BGM '" << id << "' -> " << path << std::endl;
    }
}

// 播放短音效 (SFX)
void AudioManager::playSFX(const std::string& id) {
    cleanFinishedSounds(); // 先清理旧的

    auto it = soundBuffers.find(id);
    if (it != soundBuffers.end()) {
        // SFML 3.0 修改点：必须在构造时传入 buffer
        activeSounds.emplace_back(it->second);

        sf::Sound& sound = activeSounds.back();
        // 确保音量在 0-100 范围
        float finalVol = sfxVolume * (masterVolume / 100.0f);
        if (finalVol < 0.0f) finalVol = 0.0f;
        if (finalVol > 100.0f) finalVol = 100.0f;
        sound.setVolume(finalVol);
        sound.play();

        std::cerr << "playSFX: Playing '" << id << "' volume=" << finalVol
            << " activeSounds=" << activeSounds.size() << std::endl;
    }
    else {
        std::cerr << "playSFX: SFX id not found: '" << id << "'. Available SFX keys:";
        for (const auto& p : soundBuffers) {
            std::cerr << " " << p.first;
        }
        std::cerr << std::endl;
    }
}

// 播放背景音乐 (BGM)
void AudioManager::playBGM(const std::string& id) {
    if (library.count(id) && library[id].isBGM) {
        backgroundMusic.stop(); // 停止上一首
        if (backgroundMusic.openFromFile(library[id].path)) {
            backgroundMusic.setLooping(true);
            float finalVol = musicVolume * (masterVolume / 100.0f);
            if (finalVol < 0.0f) finalVol = 0.0f;
            if (finalVol > 100.0f) finalVol = 100.0f;
            backgroundMusic.setVolume(finalVol);
            backgroundMusic.play();
            std::cerr << "playBGM: Playing BGM '" << id << "' from " << library[id].path << " volume=" << finalVol << std::endl;
        }
        else {
            std::cerr << "playBGM: Failed to open BGM file: " << library[id].path << std::endl;
        }
    }
    else {
        std::cerr << "playBGM: BGM id not found or not BGM: '" << id << "'" << std::endl;
    }
}

void AudioManager::stopBGM() {
    backgroundMusic.stop();
}

// 清理已停止的音效内存（解决 std::remove_if 报错）
void AudioManager::cleanFinishedSounds() {
    activeSounds.remove_if([](const sf::Sound& s) {
        return s.getStatus() == sf::SoundSource::Status::Stopped;
        });
}

// 音量设置更新
void AudioManager::setMasterVolume(float vol) {
    masterVolume = vol;
    // 更新 BGM 音量
    backgroundMusic.setVolume(musicVolume * (masterVolume / 100.0f));
}

void AudioManager::setMusicVolume(float vol) {
    musicVolume = vol;
    backgroundMusic.setVolume(musicVolume * (masterVolume / 100.0f));
}

void AudioManager::setSoundsVolume(float vol) {
    sfxVolume = vol;
}

void AudioManager::cleanup() {
    backgroundMusic.stop();
    activeSounds.clear();
    soundBuffers.clear();
}