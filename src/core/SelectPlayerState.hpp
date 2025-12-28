#pragma once
#include "GameState.hpp"
#include "PlayerSpriteCatalog.hpp"
#include <entt/entt.hpp>
#include <vector>
extern "C" {
    #include <raylib.h>
}

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
    // Preview animado con ECS (entidad local al estado).
    entt::registry previewRegistry_;
    entt::entity previewEntity_ = entt::null;
    std::string previewId_;
    float previewTimer_ = 0.0f;
    float previewTileSize_ = 32.0f;
    Vector2 previewPos_{0.0f, 0.0f};
    float previewIdleHold_ = 0.0f;
    bool previewHasFocus_ = false;

    void UpdatePreviewForSet(const PlayerSpriteSet& set);
};
