#pragma once
#include "raylib.h"
#include <vector>

class Spikes {
private:
    struct Spike {
        int x, y;
        bool active;
    };

    std::vector<Spike> spikes;
    float timer = 0.0f;
    float interval = 3.0f; // segundos para cambiar de estado
    int tileSize;

public:
    Spikes(int tileSize = 32);

    void addSpike(int gridX, int gridY);
    void update(float deltaTime);
    void render() const;
    bool isActiveAt(int gridX, int gridY) const;
};
