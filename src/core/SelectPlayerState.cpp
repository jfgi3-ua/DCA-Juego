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
    previewPos_ = { WINDOW_WIDTH - 220.0f, 360.0f };

    if (!defaultId_.empty()) {
        auto it = std::find_if(sets_.begin(), sets_.end(),
                               [&](const PlayerSpriteSet& set) { return set.id == defaultId_; });
        if (it != sets_.end()) {
            selectedIndex_ = (int)std::distance(sets_.begin(), it);
            // Guardamos la selección en memoria para que el siguiente estado la consuma.
            const auto& set = *it;
            PlayerSelection::SetSelectedSpriteSet(set.id, set.idlePath, set.walkPath, set.hasIdle, set.hasWalk);
            UpdatePreviewForSet(*it);
            return;
        }
    }

    if (!sets_.empty()) {
        selectedIndex_ = 0;
        // Fallback: primer set válido si no hay default.
        PlayerSelection::SetSelectedSpriteSet(sets_[0].id, sets_[0].idlePath, sets_[0].walkPath,
                                             sets_[0].hasIdle, sets_[0].hasWalk);
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
        const auto& set = sets_[selectedIndex_];
        PlayerSelection::SetSelectedSpriteSet(set.id, set.idlePath, set.walkPath, set.hasIdle, set.hasWalk);
        UpdatePreviewForSet(sets_[selectedIndex_]);
        previewIdleHold_ = 1.0f;
        previewHasFocus_ = true;
    } else if (IsKeyPressed(KEY_DOWN)) {
        selectedIndex_ = (selectedIndex_ + 1) % (int)sets_.size();
        const auto& set = sets_[selectedIndex_];
        PlayerSelection::SetSelectedSpriteSet(set.id, set.idlePath, set.walkPath, set.hasIdle, set.hasWalk);
        UpdatePreviewForSet(sets_[selectedIndex_]);
        previewIdleHold_ = 1.0f;
        previewHasFocus_ = true;
    }

    // Confirmar seleccion y continuar.
    if (IsKeyPressed(KEY_ENTER)) {
        if (!this->state_machine) return;
        this->state_machine->add_state(std::make_unique<MainGameState>(), true);
        return;
    }
}

void SelectPlayerState::update(float deltaTime) {
    if (!previewHasFocus_) {
        previewIdleHold_ = 0.0f;
    }
    previewHasFocus_ = false;
    previewIdleHold_ = std::max(0.0f, previewIdleHold_ - deltaTime);

    previewTimer_ += deltaTime;
    bool wantsWalk = previewIdleHold_ <= 0.0f && std::fmod(previewTimer_, 2.0f) >= 1.0f;

    if (previewEntity_ != entt::null && previewRegistry_.valid(previewEntity_) &&
        previewRegistry_.all_of<MovementComponent>(previewEntity_)) {
        auto& move = previewRegistry_.get<MovementComponent>(previewEntity_);
        move.wantsMove = wantsWalk;
    }

    AnimationSystem(previewRegistry_, deltaTime);
}

void SelectPlayerState::render() {
    ClearBackground(RAYWHITE);
    DrawRectangleGradientV(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, RAYWHITE, Color{240, 240, 240, 255});

    const int margin = 50;
    const int panelHeight = 380;
    const int panelWidth = (WINDOW_WIDTH - margin * 2 - 40) / 2;
    const int panelY = 150;
    Rectangle listPanel{ (float)margin, (float)panelY, (float)panelWidth, (float)panelHeight };
    Rectangle previewPanel{ (float)(margin + panelWidth + 40), (float)panelY, (float)panelWidth, (float)panelHeight };

    DrawRectangleRounded(listPanel, 0.18f, 6, Fade(LIGHTGRAY, 0.35f));
    DrawRectangleRoundedLinesEx(listPanel, 0.18f, 6, 1.0f, GRAY);
    DrawRectangleRounded(previewPanel, 0.18f, 6, Fade(LIGHTGRAY, 0.35f));
    DrawRectangleRoundedLinesEx(previewPanel, 0.18f, 6, 1.0f, GRAY);

    const char* title = "Selecciona tu personaje";
    int titleWidth = MeasureText(title, 28);
    DrawText(title, (WINDOW_WIDTH - titleWidth) / 2, 70, 28, DARKGRAY);
    DrawText("Seleccion", (int)listPanel.x + 14, (int)listPanel.y + 10, 18, DARKGRAY);
    DrawText("Preview", (int)previewPanel.x + 14, (int)previewPanel.y + 10, 18, DARKGRAY);

    if (sets_.empty()) {
        const char* emptyText = "Sin sets disponibles";
        int emptyWidth = MeasureText(emptyText, 20);
        DrawText(emptyText, (WINDOW_WIDTH - emptyWidth) / 2, 120, 20, GRAY);
    } else {
        // Lista navegable simple.
        int lineHeight = 24;
        int listStartY = (int)listPanel.y + 50;
        int listLeftX = (int)listPanel.x + 20;
        for (int i = 0; i < (int)sets_.size(); ++i) {
            int y = listStartY + (i * lineHeight);
            const char* name = sets_[i].id.c_str();
            Color color = (i == selectedIndex_) ? BLACK : DARKGRAY;
            if (i == selectedIndex_) {
                Rectangle hi{ (float)listLeftX - 8.0f, (float)y - 4.0f, listPanel.width - 28.0f, 24.0f };
                DrawRectangleRounded(hi, 0.2f, 4, Fade(GRAY, 0.18f));
                DrawRectangleRoundedLinesEx(hi, 0.2f, 4, 1.0f, Fade(GRAY, 0.4f));
            }
            DrawText(name, listLeftX, y, 20, color);
        }
    }

    // Centrar el preview dentro del panel y ajustar posicion.
    previewTileSize_ = std::min(previewPanel.width, previewPanel.height) * 0.35f;
    previewPos_ = {
        previewPanel.x + (previewPanel.width / 2.0f),
        previewPanel.y + (previewPanel.height * 0.62f)
    };
    if (previewEntity_ != entt::null && previewRegistry_.valid(previewEntity_) &&
        previewRegistry_.all_of<TransformComponent>(previewEntity_)) {
        auto& transform = previewRegistry_.get<TransformComponent>(previewEntity_);
        transform.position = previewPos_;
        transform.size = { previewTileSize_, previewTileSize_ };
    }

    RenderSystem(previewRegistry_, 0.0f, 0.0f, previewTileSize_);

    const char* hint = "Usa ARRIBA/ABAJO y ENTER para continuar";
    int hintWidth = MeasureText(hint, 18);
    DrawText(hint, (WINDOW_WIDTH - hintWidth) / 2, 120, 18, GRAY);
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
            previewEntity_, idleTex, Vector2{ 0.0f, -10.0f }, 2.2f);
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
