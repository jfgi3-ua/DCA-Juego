#include "SelectPlayerState.hpp"
#include "MainGameState.hpp"
#include "PlayerSelection.hpp"
#include "ResourceManager.hpp"
#include "StateMachine.hpp"
#include "Config.hpp"
#include "ecs/components/World/AnimationComponent.hpp"
#include "ecs/components/World/GridClipComponent.hpp"
#include "ecs/components/World/MovementComponent.hpp"
#include "ecs/components/World/SpriteComponent.hpp"
#include "ecs/components/World/TransformComponent.hpp"
#include "ecs/systems/RenderSystems.hpp"
#include "ecs/systems/WorldSystems.hpp"
#include <algorithm>
#include <cmath>

extern "C" {
    #include <raylib.h>
}

static int ComputeFramesForTexture(const Texture2D& tex) {
    if (tex.height <= 0) return 1;
    int frames = tex.width / tex.height;
    return frames > 0 ? frames : 1;
}

void SelectPlayerState::init() {
    // Descubre los sets disponibles en assets/sprites/player.
    sets_ = DiscoverPlayerSpriteSets();
    // Resuelve el set por defecto (preferimos "Archer" si existe y está completo).
    defaultId_ = ResolveDefaultPlayerSpriteSetId(sets_);
    previewTileSize_ = (float)TILE_SIZE;
    previewPos_ = { WINDOW_WIDTH - 260.0f, 360.0f };

    if (!defaultId_.empty()) {
        auto it = std::find_if(sets_.begin(), sets_.end(),
                               [&](const PlayerSpriteSet& set) { return set.id == defaultId_; });
        if (it != sets_.end()) {
            selectedIndex_ = (int)std::distance(sets_.begin(), it);
            // Guardamos la selección en memoria para que el siguiente estado la consuma.
            PlayerSelection::SetSelectedSpriteId(defaultId_);
            UpdatePreviewForSet(*it);
            return;
        }
    }

    if (!sets_.empty()) {
        selectedIndex_ = 0;
        // Fallback: primer set válido si no hay default.
        PlayerSelection::SetSelectedSpriteId(sets_[0].id);
        UpdatePreviewForSet(sets_[0]);
    }
}

void SelectPlayerState::handleInput() {
    if (sets_.empty()) {
        if (IsKeyPressed(KEY_ENTER)) {
            if (!this->state_machine) return;
            this->state_machine->add_state(std::make_unique<MainGameState>(), true);
        }
        return;
    }

    // Navegacion basica con teclado.
    if (IsKeyPressed(KEY_UP)) {
        selectedIndex_ = (selectedIndex_ - 1 + (int)sets_.size()) % (int)sets_.size();
        PlayerSelection::SetSelectedSpriteId(sets_[selectedIndex_].id);
        UpdatePreviewForSet(sets_[selectedIndex_]);
    } else if (IsKeyPressed(KEY_DOWN)) {
        selectedIndex_ = (selectedIndex_ + 1) % (int)sets_.size();
        PlayerSelection::SetSelectedSpriteId(sets_[selectedIndex_].id);
        UpdatePreviewForSet(sets_[selectedIndex_]);
    }

    // Confirmar seleccion y continuar.
    if (IsKeyPressed(KEY_ENTER)) {
        if (!this->state_machine) return;
        this->state_machine->add_state(std::make_unique<MainGameState>(), true);
        return;
    }
}

void SelectPlayerState::update(float deltaTime) {
    previewTimer_ += deltaTime;
    bool wantsWalk = std::fmod(previewTimer_, 2.0f) >= 1.0f;

    if (previewEntity_ != entt::null && previewRegistry_.valid(previewEntity_) &&
        previewRegistry_.all_of<MovementComponent>(previewEntity_)) {
        auto& move = previewRegistry_.get<MovementComponent>(previewEntity_);
        move.wantsMove = wantsWalk;
    }

    AnimationSystem(previewRegistry_, deltaTime);
}

void SelectPlayerState::render() {
    ClearBackground(RAYWHITE);

    const char* title = "Seleccion de personaje (placeholder)";
    int titleWidth = MeasureText(title, 24);
    DrawText(title, (WINDOW_WIDTH - titleWidth) / 2, 60, 24, DARKGRAY);

    if (sets_.empty()) {
        const char* emptyText = "Sin sets disponibles";
        int emptyWidth = MeasureText(emptyText, 20);
        DrawText(emptyText, (WINDOW_WIDTH - emptyWidth) / 2, 110, 20, GRAY);
    } else {
        // Lista navegable simple.
        int lineHeight = 24;
        for (int i = 0; i < (int)sets_.size(); ++i) {
            int y = listStartY_ + (i * lineHeight);
            const char* name = sets_[i].id.c_str();
            Color color = (i == selectedIndex_) ? BLACK : DARKGRAY;
            DrawText(name, 120, y, 20, color);
        }
    }

    // Panel de preview a la derecha.
    Rectangle previewPanel{ WINDOW_WIDTH - 360.0f, 140.0f, 300.0f, 360.0f };
    DrawRectangleRounded(previewPanel, 0.2f, 6, Fade(LIGHTGRAY, 0.4f));
    DrawRectangleRoundedLinesEx(previewPanel, 0.2f, 6, 1.0f, GRAY);
    DrawText("Preview", (int)previewPanel.x + 12, (int)previewPanel.y + 10, 18, DARKGRAY);

    RenderSystem(previewRegistry_, 0.0f, 0.0f, previewTileSize_);

    const char* hint = "Usa ARRIBA/ABAJO y ENTER para continuar";
    int hintWidth = MeasureText(hint, 18);
    DrawText(hint, (WINDOW_WIDTH - hintWidth) / 2, 100, 18, GRAY);
}

void SelectPlayerState::UpdatePreviewForSet(const PlayerSpriteSet& set) {
    if (!set.hasIdle || !set.hasWalk) return;

    if (previewId_ == set.id && previewEntity_ != entt::null && previewRegistry_.valid(previewEntity_)) {
        return;
    }
    previewId_ = set.id;

    auto& rm = ResourceManager::Get();
    const Texture2D& idleTex = rm.GetTexture(set.idlePath);
    const Texture2D& walkTex = rm.GetTexture(set.walkPath);

    int idleFrames = ComputeFramesForTexture(idleTex);
    int walkFrames = ComputeFramesForTexture(walkTex);

    if (previewEntity_ == entt::null || !previewRegistry_.valid(previewEntity_)) {
        previewEntity_ = previewRegistry_.create();
        previewRegistry_.emplace<TransformComponent>(
            previewEntity_, previewPos_, Vector2{ previewTileSize_, previewTileSize_ });
        previewRegistry_.emplace<SpriteComponent>(
            previewEntity_, idleTex, Vector2{ 0.0f, -10.0f }, 2.0f);
        previewRegistry_.emplace<GridClipComponent>(previewEntity_, idleFrames);
        previewRegistry_.emplace<AnimationComponent>(
            previewEntity_, idleTex, walkTex, idleFrames, walkFrames, 0.2f, 0.12f);
        previewRegistry_.emplace<MovementComponent>(previewEntity_, 0.0f);
        return;
    }

    auto& sprite = previewRegistry_.get<SpriteComponent>(previewEntity_);
    auto& grid = previewRegistry_.get<GridClipComponent>(previewEntity_);
    auto& anim = previewRegistry_.get<AnimationComponent>(previewEntity_);
    auto& transform = previewRegistry_.get<TransformComponent>(previewEntity_);

    sprite.texture = idleTex;
    anim.idleTexture = idleTex;
    anim.walkTexture = walkTex;
    anim.idleFrames = idleFrames;
    anim.walkFrames = walkFrames;
    anim.isWalking = false;

    grid.numFrames = idleFrames;
    grid.currentFrame = 0;
    grid.currentRow = 0;
    grid.timer = 0.0f;
    grid.frameTime = anim.idleFrameTime;

    transform.position = previewPos_;
    transform.size = { previewTileSize_, previewTileSize_ };
}
