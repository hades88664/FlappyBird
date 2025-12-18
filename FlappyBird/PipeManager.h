// PipeManager.h
#pragma once
#include <vector>

struct Rect;

struct Pipe {
    float posX;
    float gapY;
    float gapHeight;
    bool passed;
    int color;

    Rect GetTopPipeRect() const;
    Rect GetBottomPipeRect() const;
};

class PipeManager {
private:
    std::vector<Pipe> pipes;
    float pipeWidth;
    float pipeSpeed;
    float gapHeight;
    float minGapY;
    float maxGapY;
    float spawnInterval;
    float spawnTimer;
    float lastSpawnX;

public:
    PipeManager();
    void Initialize(float windowWidth);
    void Update(float deltaTime);
    void Render() const;
    void SpawnPipe();
    bool CheckCollision(const Rect& birdBox) const;
    int CheckAndMarkPassed(float birdX);  // 新增：检查通过并返回数量
    void CleanupPipes(float leftBound);
    void Reset();
    const std::vector<Pipe>& GetPipes() const { return pipes; }
};