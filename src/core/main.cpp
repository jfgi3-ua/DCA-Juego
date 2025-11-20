#include "MainGameState.hpp"
#include "GameState.hpp"
#include "StateMachine.hpp"
#include "StartGameState.hpp"
#include <memory>
#include <chrono>
#include "objects/Map.hpp" // Para medir el mapa antes
#include "Config.hpp" // Archivo de configuración global (TILE_SIZE, HUD_HEIGHT)
extern "C" {
  #include <raylib.h>
}

int main() {
  // 0) Medir el tamaño del mapa y preparar la ventana acorde
  Map tmp;
  tmp.loadFromFile("assets/maps/map_6.txt", TILE_SIZE);
  const int MAP_W_PX = tmp.width() * TILE_SIZE;
  const int MAP_H_PX = tmp.height() * TILE_SIZE;

  // 1) Crear ventana con altura extra para el HUD inferior y fijar FPS
  int winW = std::max(MAP_W_PX, WINDOW_WIDTH);
  int winH = MAP_H_PX + HUD_HEIGHT;
  InitWindow(winW, winH, "Escape del Laberinto");
  SetTargetFPS(60);
  
  // Desactivar que ESC cierre la ventana automáticamente
  SetExitKey(KEY_NULL);

  // 2) Arrancar la máquina de estados con MainGameState
  StateMachine state_machine;
  float delta_time = 0.0f;
  state_machine.add_state(std::make_unique<StartGameState>(), false);
  state_machine.handle_state_changes(delta_time);

  // 3) Bucle principal (hasta que se cierre la ventana o termine el juego)
  while (!WindowShouldClose() && !state_machine.is_game_ending()) {
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