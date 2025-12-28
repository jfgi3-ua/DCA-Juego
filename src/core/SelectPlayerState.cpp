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
    // Sin UI navegable aún. Enter continúa al juego.
    if (IsKeyPressed(KEY_ENTER)) {
        if (!this->state_machine) {
            std::cerr << "[SelectPlayerState] state_machine es null al confirmar." << std::endl;
            return;
        }
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

    // Placeholder: solo mostramos el id seleccionado.
    std::string current = sets_.empty() ? "Sin sets disponibles" : sets_[selectedIndex_].id;
    std::string line = "Seleccion actual: " + current;
    int lineWidth = MeasureText(line.c_str(), 20);
    DrawText(line.c_str(), (WINDOW_WIDTH - lineWidth) / 2, 110, 20, GRAY);

    const char* hint = "Pulsa ENTER para continuar";
    int hintWidth = MeasureText(hint, 18);
    DrawText(hint, (WINDOW_WIDTH - hintWidth) / 2, 160, 18, GRAY);
}
