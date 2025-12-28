// 因为 .cpp 在 src 文件夹，.h 在 include 文件夹
// 所以需要 ../ 先跳出 src，再进入 include
#include "../include/AudioManager.h" 
#include <algorithm>
#include <iostream>
#include "../include/constants.h"
#include <SFML/Audio.hpp>

AudioManager::AudioManager()
    : masterVolume(100.0f), soundsVolume(100.0f), musicVolume(100.0f) {
}

AudioManager::~AudioManager() {
    cleanup();
}

bool AudioManager::loadSoundEffect(const std::string& name, const std::string& filepath) {
    sf::SoundBuffer buffer;
    if (buffer.loadFromFile(filepath)) {
        soundBuffers[name] = buffer;
        return true;
    }
    return false;
}

void AudioManager::playSound(const std::string& name, float volume, float pitch) {
    cleanFinishedSounds();
    auto it = soundBuffers.find(name);
    if (it != soundBuffers.end()) {
        // 直接在 emplace_back 的括号里传入 Buffer
        // 直接调用 sf::Sound(const SoundBuffer&) 构造函数
        activeSounds.emplace_back(it->second);

        sf::Sound& sound = activeSounds.back();

        // 注意：把原来的 sound.setBuffer(...) 那一行删掉，因为上面已经设置过了

        float finalVolume = (masterVolume / 100.0f) * (soundsVolume / 100.0f) * volume;
        sound.setVolume(finalVolume);
        sound.setPitch(pitch);
        sound.play();
    }
}

bool AudioManager::playMusic(const std::string& filepath, bool loop) {
    if (backgroundMusic.openFromFile(filepath)) {
        backgroundMusic.setLooping(loop);
        backgroundMusic.setVolume(masterVolume * (musicVolume / 100.0f));
        backgroundMusic.play();
        return true;
    }
    return false;
}

void AudioManager::stopMusic() { backgroundMusic.stop(); }
void AudioManager::pauseMusic() { backgroundMusic.pause(); }
void AudioManager::resumeMusic() { backgroundMusic.play(); }

void AudioManager::setMasterVolume(float volume) { masterVolume = volume; }
void AudioManager::setSoundsVolume(float volume) { soundsVolume = volume; }
void AudioManager::setMusicVolume(float volume) { musicVolume = volume; }

float AudioManager::getMasterVolume() const { return masterVolume; }
float AudioManager::getSoundsVolume() const { return soundsVolume; }
float AudioManager::getMusicVolume() const { return musicVolume; }

bool AudioManager::isMusicPlaying() const {
    return backgroundMusic.getStatus() == ::sf::SoundSource::Status::Playing;
}

void AudioManager::cleanFinishedSounds() {
    activeSounds.erase(std::remove_if(activeSounds.begin(), activeSounds.end(),
        [](const sf::Sound& s) { 
            return s.getStatus() == ::sf::SoundSource::Status::Stopped;
        }), activeSounds.end());
}

void AudioManager::cleanup() {
    backgroundMusic.stop();
    activeSounds.clear();
    soundBuffers.clear();
}