#include "Spikes.hpp"
#include <iostream>


Spikes::Spikes() {}

void Spikes::LoadTextures() {
    auto& rm = ResourceManager::Get();
    _spikeTex = &rm.GetTexture("sprites/_spikes.png");
    
    _srcRectActive = {28, 126, 22, 22};
    _srcRectInactive = { 28, 0, 22, 22};
}

void Spikes::addSpike(int gridX, int gridY) {
    _spikes.push_back({gridX, gridY, true});
}

void Spikes::update(float deltaTime) {
    _timer += deltaTime;
    if (_timer >= _interval) {
        _timer = 0.0f;
        for (auto &s : _spikes)
            s.active = !s.active;
    }
}

void Spikes::render(int ox, int oy) const {
    if (!_spikeTex) return;

    for (const auto &s : _spikes) {

        Rectangle src = s.active ? _srcRectActive : _srcRectInactive;

        Rectangle dest {
            (float)ox + s.x * _tileSize,
            (float)oy + s.y * _tileSize,
            (float)_tileSize,
            (float)_tileSize
        };

        DrawTexturePro(*_spikeTex, src, dest, {0,0}, 0.0f, WHITE);
    }
}


bool Spikes::isActiveAt(int gridX, int gridY) const {
    for (const auto &s : _spikes) {
        if (s.x == gridX && s.y == gridY)
            return s.active;
    }
    return false;
}
