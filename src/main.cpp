#include "GameState.hpp"
#include "StateMachine.hpp"
#include "StartGameState.hpp"
#include <memory>
#include <chrono>
extern "C" {
  #include <raylib.h>
}

int main() {
    // 1) Crear ventana (512 x 512) y fijar FPS
    InitWindow(512, 512, "Escape del Laberinto");
    SetTargetFPS(60);

    // 2) Arrancar la máquina de estados con MainGameState
    StateMachine state_machine;
    float delta_time = 0.0f;
    state_machine.add_state(std::make_unique<StartGameState>(), false);
    state_machine.handle_state_changes(delta_time);

    // 3) Bucle principal (hasta que se cierre la ventana)
    while (!WindowShouldClose()) {
        delta_time = GetFrameTime();

        // 1 - Capturar input
        // 2 - Procesar lógica
        // 3 - Renderizar
        state_machine.handle_state_changes(delta_time);
        state_machine.getCurrentState()->handleInput();
        state_machine.getCurrentState()->update(delta_time);
        state_machine.getCurrentState()->render();
    }

    // 4) Cerrar ventana
    CloseWindow();
    return 0;
}