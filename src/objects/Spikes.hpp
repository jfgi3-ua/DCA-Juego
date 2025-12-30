#pragma once
#include "raylib.h"
#include <vector>
#include "core/Config.hpp"
#include "../core/ResourceManager.hpp"

class Spikes {
public:
    Spikes();

    void addSpike(int gridX, int gridY);
    void update(float deltaTime);
    void render(int ox, int oy) const;
    bool isActiveAt(int gridX, int gridY) const;
    void LoadTextures();

private:
    struct Spike {
        int x, y;
        bool active;
    };

    std::vector<Spike> _spikes;
    float _timer = 0.0f;
    float _interval = 3.0f; // segundos para cambiar de estado
    int _tileSize = TILE_SIZE;
    
    const Texture2D* _spikeTex = nullptr; 
    Rectangle _srcRectActive;
    Rectangle _srcRectInactive;
};
