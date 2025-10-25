#include "Spikes.hpp"
#include <iostream>


Spikes::Spikes(int tileSize) : tileSize(tileSize) {}

void Spikes::addSpike(int gridX, int gridY) {
    spikes.push_back({gridX, gridY, true});
}

void Spikes::update(float deltaTime) {
    timer += deltaTime;
    if (timer >= interval) {
        timer = 0.0f;
        for (auto &s : spikes)
            s.active = !s.active;
    }
}

void Spikes::render(int ox, int oy) const {
    for (const auto &s : spikes) {
        //si esta activo dibuja ^^ sino nada
        if (s.active) {
            DrawText("^^", 
                     (s.x * tileSize + tileSize / 4) + ox,  
                     (s.y * tileSize + tileSize / 8) + oy, 
                     tileSize, 
                     BLACK);
        }
    }
}

bool Spikes::isActiveAt(int gridX, int gridY) const {
    for (const auto &s : spikes) {
        if (s.x == gridX && s.y == gridY)
            return s.active;
    }
    return false;
}
