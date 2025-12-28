#include "../include/AudioManager.h" 
#include <algorithm>
#include <iostream>
#include "../include/constants.h"
#include <SFML/Audio.hpp>

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
        sf::SoundBuffer buffer;
        if (buffer.loadFromFile(path)) {
            soundBuffers[id] = buffer;
        }
        else {
            std::cerr << "Failed to load SFX: " << path << std::endl;
        }
    }
}

// 播放短音效 (SFX)
void AudioManager::playSFX(const std::string& id) {
    cleanFinishedSounds(); // 先清理旧的

    if (soundBuffers.count(id)) {
        // SFML 3.0 修改点：必须在构造时传入 buffer
        // emplace_back 会调用 sf::Sound(const sf::SoundBuffer&)
        activeSounds.emplace_back(soundBuffers[id]);

        sf::Sound& sound = activeSounds.back();
        float finalVol = sfxVolume * (masterVolume / 100.0f);
        sound.setVolume(finalVol);
        sound.play();
    }
}

// 播放背景音乐 (BGM)
void AudioManager::playBGM(const std::string& id) {
    if (library.count(id) && library[id].isBGM) {
        backgroundMusic.stop(); // 停止上一首
        if (backgroundMusic.openFromFile(library[id].path)) {
            backgroundMusic.setLooping(true);
            float finalVol = musicVolume * (masterVolume / 100.0f);
            backgroundMusic.setVolume(finalVol);
            backgroundMusic.play();
        }
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