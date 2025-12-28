#pragma once
#include "GameState.hpp"
#include "PlayerSpriteCatalog.hpp"
#include <vector>

class SelectPlayerState : public GameState {
public:
    SelectPlayerState() = default;
    ~SelectPlayerState() override = default;

    void init() override;
    void handleInput() override;
    void update(float deltaTime) override;
    void render() override;
    void pause() override {}
    void resume() override {}

private:
    // Cache local de los sets disponibles para render y navegación.
    std::vector<PlayerSpriteSet> sets_;
    // Índice actual dentro de sets_.
    int selectedIndex_ = 0;
    // Id resuelto como default (p.ej. "Archer") si está disponible.
    std::string defaultId_;
    // Top inicial para listar opciones en pantalla.
    int listStartY_ = 140;
};
