#pragma once
#include "raylib.h"
#include <vector>
#include "core/Config.hpp"
#include "../core/ResourceManager.hpp"

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
    
    const Texture2D* spikeTex_ = nullptr; 
    Rectangle srcRectActive_;
    Rectangle srcRectInactive_;


public:
    Spikes();

    void addSpike(int gridX, int gridY);
    void update(float deltaTime);
    void render(int ox, int oy) const;
    bool isActiveAt(int gridX, int gridY) const;
    void LoadTextures();
};
