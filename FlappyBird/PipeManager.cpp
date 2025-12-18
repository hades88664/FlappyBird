// PipeManager.cpp
#include "PipeManager.h"
#include "Bird.h"
#include <graphics.h>
#include <cstdlib>
#include <ctime>

Rect Pipe::GetTopPipeRect() const {
    Rect rect;
    rect.x = posX;
    rect.y = 0;
    rect.width = 80.0f;
    rect.height = gapY - gapHeight / 2.0f;
    return rect;
}

Rect Pipe::GetBottomPipeRect() const {
    Rect rect;
    rect.x = posX;
    rect.y = gapY + gapHeight / 2.0f;
    rect.width = 80.0f;
    rect.height = 1000.0f;
    return rect;
}

PipeManager::PipeManager()
    : pipeWidth(80.0f), pipeSpeed(200.0f), gapHeight(180.0f),
    minGapY(150.0f), maxGapY(450.0f), spawnInterval(2.0f),
    spawnTimer(0.0f), lastSpawnX(0.0f) {
    srand(static_cast<unsigned>(time(nullptr)));
}

void PipeManager::Initialize(float windowWidth) {
    pipes.clear();
    spawnTimer = 0.0f;
    lastSpawnX = windowWidth;

    for (int i = 0; i < 3; ++i) {
        SpawnPipe();
        pipes.back().posX += i * 300.0f;
    }
}

void PipeManager::Update(float deltaTime) {
    for (auto& pipe : pipes) {
        pipe.posX -= pipeSpeed * deltaTime;
    }

    spawnTimer += deltaTime;
    if (spawnTimer >= spawnInterval) {
        SpawnPipe();
        spawnTimer = 0.0f;
    }

    CleanupPipes(-pipeWidth);
}

void PipeManager::Render() const {
    for (const auto& pipe : pipes) {
        Rect topPipe = pipe.GetTopPipeRect();
        Rect bottomPipe = pipe.GetBottomPipeRect();

        setfillcolor(GREEN);
        solidrectangle(static_cast<int>(topPipe.x),
            static_cast<int>(topPipe.y),
            static_cast<int>(topPipe.x + topPipe.width),
            static_cast<int>(topPipe.y + topPipe.height));

        solidrectangle(static_cast<int>(bottomPipe.x),
            static_cast<int>(bottomPipe.y),
            static_cast<int>(bottomPipe.x + bottomPipe.width),
            static_cast<int>(bottomPipe.y + bottomPipe.height));

        setfillcolor(RGB(0, 180, 0));
        solidrectangle(static_cast<int>(topPipe.x - 5),
            static_cast<int>(topPipe.y + topPipe.height - 20),
            static_cast<int>(topPipe.x + topPipe.width + 5),
            static_cast<int>(topPipe.y + topPipe.height));

        solidrectangle(static_cast<int>(bottomPipe.x - 5),
            static_cast<int>(bottomPipe.y),
            static_cast<int>(bottomPipe.x + bottomPipe.width + 5),
            static_cast<int>(bottomPipe.y + 20));
    }
}

void PipeManager::SpawnPipe() {
    Pipe newPipe;
    float randomGapY = minGapY + static_cast<float>(rand()) /
        (static_cast<float>(RAND_MAX / (maxGapY - minGapY)));

    newPipe.posX = lastSpawnX + 400.0f;
    newPipe.gapY = randomGapY;
    newPipe.gapHeight = gapHeight;
    newPipe.passed = false;
    newPipe.color = GREEN;

    pipes.push_back(newPipe);
    lastSpawnX = newPipe.posX;
}

bool PipeManager::CheckCollision(const Rect& birdBox) const {
    for (const auto& pipe : pipes) {
        if (birdBox.CheckCollision(pipe.GetTopPipeRect()) ||
            birdBox.CheckCollision(pipe.GetBottomPipeRect())) {
            return true;
        }
    }
    return false;
}

// 新增函数：检查并标记通过的管道
int PipeManager::CheckAndMarkPassed(float birdX) {
    int passedCount = 0;
    for (auto& pipe : pipes) {
        if (birdX > pipe.posX && !pipe.passed) {
            pipe.passed = true;
            passedCount++;
        }
    }
    return passedCount;
}

void PipeManager::CleanupPipes(float leftBound) {
    for (auto it = pipes.begin(); it != pipes.end(); ) {
        if (it->posX + pipeWidth < leftBound) {
            it = pipes.erase(it);
        }
        else {
            ++it;
        }
    }
}

void PipeManager::Reset() {
    pipes.clear();
    spawnTimer = 0.0f;
    lastSpawnX = 800.0f;

    for (int i = 0; i < 3; ++i) {
        SpawnPipe();
        pipes.back().posX += i * 300.0f;
    }
}