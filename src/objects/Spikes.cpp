#include "Spikes.hpp"
#include <iostream>


Spikes::Spikes() {
    auto& rm = ResourceManager::Get();
    spikeTex_ = &rm.GetTexture("sprites/spikes.png");
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

    int frameH = spikeTex_->height / 8;    // 5 filas → ajusta si son más/menos
    int frameW = spikeTex_->width / 4;         // ancho completo

    int activeRow   = 4;   // última fila (pinchos arriba)
    int inactiveRow = 0;   // primera fila (pinchos abajo)

    for (const auto &s : spikes) {

        int row = s.active ? activeRow : inactiveRow;

        Rectangle src {
            (float) (1 * frameW), // segunda columna (frame estático)
            (float)(row * frameH),
            (float)frameW,
            (float)frameH
        };

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
