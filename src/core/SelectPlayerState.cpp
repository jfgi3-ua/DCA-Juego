#include "SelectPlayerState.hpp"
#include "MainGameState.hpp"
#include "PlayerSelection.hpp"
#include "StateMachine.hpp"
#include "Config.hpp"
#include <algorithm>

extern "C" {
    #include <raylib.h>
}

void SelectPlayerState::init() {
    // Descubre los sets disponibles en assets/sprites/player.
    sets_ = DiscoverPlayerSpriteSets();
    // Resuelve el set por defecto (preferimos "Archer" si existe y está completo).
    defaultId_ = ResolveDefaultPlayerSpriteSetId(sets_);

    if (!defaultId_.empty()) {
        auto it = std::find_if(sets_.begin(), sets_.end(),
                               [&](const PlayerSpriteSet& set) { return set.id == defaultId_; });
        if (it != sets_.end()) {
            selectedIndex_ = (int)std::distance(sets_.begin(), it);
        }
        // Guardamos la selección en memoria para que el siguiente estado la consuma.
        PlayerSelection::SetSelectedSpriteId(defaultId_);
    } else if (!sets_.empty()) {
        selectedIndex_ = 0;
        // Fallback: primer set válido si no hay default.
        PlayerSelection::SetSelectedSpriteId(sets_[0].id);
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
    } else if (IsKeyPressed(KEY_DOWN)) {
        selectedIndex_ = (selectedIndex_ + 1) % (int)sets_.size();
        PlayerSelection::SetSelectedSpriteId(sets_[selectedIndex_].id);
    }

    // Confirmar seleccion y continuar.
    if (IsKeyPressed(KEY_ENTER)) {
        if (!this->state_machine) return;
        this->state_machine->add_state(std::make_unique<MainGameState>(), true);
        return;
    }
}

void SelectPlayerState::update(float) {
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

    const char* hint = "Usa ARRIBA/ABAJO y ENTER para continuar";
    int hintWidth = MeasureText(hint, 18);
    DrawText(hint, (WINDOW_WIDTH - hintWidth) / 2, 100, 18, GRAY);
}
