#pragma once
#include "raylib.h"
#include <vector>
#include "Config.hpp"

class Spikes {
private:
    struct Spike {
        int x, y;
        bool active;
    };

    std::vector<Spike> spikes;
    float timer = 0.0f;
    float interval = 3.0f; // segundos para cambiar de estado
    int tileSize = TILE_SIZE;

public:
    Spikes();

    void addSpike(int gridX, int gridY);
    void update(float deltaTime);
    void render(int ox, int oy) const;
    bool isActiveAt(int gridX, int gridY) const;
};
