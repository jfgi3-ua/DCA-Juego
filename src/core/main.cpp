#include "MainGameState.hpp"
#include "GameState.hpp"
#include "StateMachine.hpp"
#include "StartGameState.hpp"
#include <memory>
#include <chrono>
#include "objects/Map.hpp" // Para medir el mapa antes
#include "Config.hpp" // Archivo de configuración global (TILE_SIZE, HUD_HEIGHT)
#include "ResourceManager.hpp"
#include "Localization.hpp"
extern "C" {
  #include <raylib.h>
}

int main() {

  // Inicializar localización (por defecto español)
  InitLocalization("es");

  // 0) Medir el tamaño del mapa y preparar la ventana acorde
  Map tmp;
  auto& rm = ResourceManager::Get();
  int level_ = 6; // Define the level number or get it from user input or configuration
  std::string relativePath = "maps/map_" + std::to_string(level_) + ".txt";
  tmp.loadFromFile(rm.GetAssetPath(relativePath), TILE_SIZE);

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

    // Permitir cambio de idioma con tecla L/l
    if (IsKeyPressed(KEY_L)) {
      SwitchLocalization();
    }

    state_machine.handle_state_changes(delta_time);

    // Si hay overlay, solo procesar input del overlay
    if (state_machine.hasOverlay()) {
      state_machine.getOverlayState()->handleInput();
      // NO actualizar el juego si hay overlay (pausa)
    } else {
      state_machine.getCurrentState()->handleInput();
      state_machine.getCurrentState()->update(delta_time);
    }

    // Renderizar: BeginDrawing una sola vez
    BeginDrawing();

    // Primero el estado principal
    state_machine.getCurrentState()->render();

    // Luego el overlay encima
    if (state_machine.hasOverlay()) {
      state_machine.getOverlayState()->render();
    }

    EndDrawing();
  }

  // 4) Cerrar ventana
  CloseWindow();
  return 0;
}