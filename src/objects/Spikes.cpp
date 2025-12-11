#include "Spikes.hpp"
#include <iostream>


Spikes::Spikes() {}

void Spikes::LoadTextures() {
    auto& rm = ResourceManager::Get();
    spikeTex_ = &rm.GetTexture("sprites/spikes.png");
    
    srcRectActive_ = {28, 126, 28, 28};
    srcRectInactive_ = { 28, 0, 28, 28};
}

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
    if (!spikeTex_) return;

    for (const auto &s : spikes) {

        Rectangle src = s.active ? srcRectActive_ : srcRectInactive_;

        Rectangle dest {
            ox + s.x * tileSize,
            oy + s.y * tileSize,
            (float)tileSize,
            (float)tileSize
        };

        DrawTexturePro(*spikeTex_, src, dest, {0,0}, 0.0f, WHITE);
    }
}


bool Spikes::isActiveAt(int gridX, int gridY) const {
    for (const auto &s : spikes) {
        if (s.x == gridX && s.y == gridY)
            return s.active;
    }
    return false;
}
