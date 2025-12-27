#include <iostream>
#include "MainGameState.hpp"
#include "GameOverState.hpp"
#include "DevModeState.hpp"
#include "StateMachine.hpp"
#include "ResourceManager.hpp"
#include "ecs/Ecs.hpp"
extern "C" {
  #include <raylib.h>
}

MainGameState::MainGameState(int level)
{
    level_ = level > 0 ? level : 1;
}

void MainGameState::init()
{
    auto& rm = ResourceManager::Get();
    std::string relativePath = "maps/map_" + std::to_string(level_) + ".txt";
    std::string absolutePath = rm.GetAssetPath(relativePath);

    //std::string absolutePath = "/usr/share/game/assets/maps/map_1.txt"; //probando poque da error al intentar abrilo en carpeta random
    map_.loadFromFile(absolutePath, TILE_SIZE);
    map_.loadTextures(); //lo llamamos aqui ya q tambien se llama en main y no se pueden cargar texturas antes de InitWindow
    tile_ = map_.tile();

    // Guardar total de llaves del mapa (antes de que se recojan)
    totalKeysInMap_ = map_.getTotalKeys();

    // Cargar entidades del nivel en el registry
    LevelSetupSystem(registry, map_);

    // Inicializar temporizador: 30s base + 30s por cada nivel adicional
    levelTime_ = 30.0f + (level_ - 1) * 30.0f;
}

void MainGameState::handleInput()
{
    // 1. Activar menú de desarrollador con CTRL+D
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D)) {
        this->state_machine->add_overlay_state(
            std::make_unique<DevModeState>(&registry, &levelTime_, &freezeEnemies_, &infiniteTime_,
                                           &keyGivenByCheating_, &totalKeysInMap_, level_)
        );
        return;
    }

    // 2. Salir al estado de Game Over al presionar ESPACIO (simulando derrota)
    if (IsKeyPressed(KEY_SPACE)) {
        // Argumentos de GameOverState: nivel actual, ha muerto (true), tiempo restante, juego terminado (false)
        this->state_machine->add_state(
            std::make_unique<GameOverState>(level_, true, levelTime_, false), 
            true // Reemplazar el estado actual
        );
        return;
    }
}

void MainGameState::update(float deltaTime)
{
    // Reducir temporizador de nivel (excepto si está en modo tiempo infinito)
    if (!infiniteTime_) {
        levelTime_ -= deltaTime;
        if (levelTime_ <= 0.0f) {
            levelTime_ = 0.0f;
            // Tiempo agotado -> Game Over (dead = true)
            this->state_machine->add_state(std::make_unique<GameOverState>(level_, true, 0.0f, false), true);
            return;
        }
    }

    // Primero Input (decide destino), luego Movimiento (mueve)
    InputSystem(registry, map_);
    if (!freezeEnemies_) {
        EnemyAISystem(registry, map_, deltaTime);
    }
    MovementSystem(registry, deltaTime);
    AnimationSystem(registry, deltaTime);
    SpikeSystem(registry, deltaTime);
    InvulnerabilitySystem(registry, deltaTime);
    CollisionSystem(registry, map_); // Chequeo de colisiones
    MechanismSystem(registry, map_);

    // --- FLUJO DE JUEGO ECS: derrota/victoria ---
    auto playerView = registry.view<TransformComponent, PlayerStatsComponent, PlayerInputComponent>();
    if (playerView) {
        auto playerEntity = *playerView.begin();
        const auto &stats = playerView.get<PlayerStatsComponent>(playerEntity);
        const auto &trans = playerView.get<TransformComponent>(playerEntity);

        // Derrota por vidas
        if (stats.lives <= 0) {
            this->state_machine->add_state(std::make_unique<GameOverState>(level_, true, levelTime_, false), true);
            return;
        }

        // Victoria por salida + llaves
        int cellX = (int)(trans.position.x / tile_);
        int cellY = (int)(trans.position.y / tile_);
        if (cellX >= 0 && cellY >= 0 && cellX < map_.width() && cellY < map_.height()) {
            if (map_.at(cellX, cellY) == 'X' && stats.keysCollected >= totalKeysInMap_) {
                if (level_ >= 6) {
                    this->state_machine->add_state(std::make_unique<GameOverState>(level_, false, levelTime_, true), true);
                } else {
                    this->state_machine->add_state(std::make_unique<GameOverState>(level_, false, levelTime_, false), true);
                }
                return;
            }
        }
    }

}

void MainGameState::render()
{
    ClearBackground(RAYWHITE);
    auto& rm = ResourceManager::Get();

    // Dimensiones
    const int mapWpx = map_.width()  * tile_;
    const int mapHpx = map_.height() * tile_;
    const int viewW  = GetScreenWidth();
    const int viewH  = GetScreenHeight() - HUD_HEIGHT; // Espacio disponible sin el HUD

    // Offset centrado (clamp >= 0) - El mapa queda centrado en el espacio disponible
    // const int ox = std::max(0, (viewW - mapWpx) / 2);
    // const int oy = std::max(0, (viewH - mapHpx) / 2);
    const int ox = (viewW > mapWpx) ? (viewW - mapWpx) / 2 : 0;
    const int oy = (viewH > mapHpx) ? (viewH - mapHpx) / 2 : 0;

    // 1) Mapa (dibujado en la zona superior, desde y=0 hasta y=MAP_H_PX)
    map_.render(ox, oy);

    RenderMechanismSystem(registry, ox, oy);
    RenderSystem(registry, (float)ox, (float)oy, (float)map_.tile());

    // 4) HUD inferior - se coloca en la parte inferior de la ventana
    const float baseY = (float)(GetScreenHeight() - HUD_HEIGHT); // HUD siempre abajo
    // Fondo del HUD a lo ancho de la ventana
    Rectangle hudBg{ 0.0f, baseY, (float)GetScreenWidth(), (float)HUD_HEIGHT };
    DrawRectangleRec(hudBg, Fade(BLACK, 0.06f));
    // sombreado sutil encima del HUD
    for (int i=0; i<6; ++i) {
        Color c = Fade(BLACK, 0.05f * (6 - i));
        DrawLine(0, (int)baseY - i, GetScreenWidth(), (int)baseY - i, c);
    }

    // Panel de mochila (alineado a la izquierda dentro del HUD)
    const int pad = 10;
    Rectangle bagHud{ (float)pad, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad };
    DrawRectangleRounded(bagHud, 0.25f, 6, Fade(BLACK, 0.10f));
    DrawRectangleRoundedLinesEx(bagHud, 0.25f, 6, 1.0f, DARKGRAY);
    DrawText("Mochila", (int)bagHud.x + 10, (int)bagHud.y + 6, 16, DARKGRAY);

    // Panel de vidas (alineado a la derecha dentro del HUD)
    Rectangle livesHud{ (float)GetScreenWidth() - 190.0f, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad };
    DrawRectangleRounded(livesHud, 0.25f, 6, Fade(BLACK, 0.10f));
    DrawRectangleRoundedLinesEx(livesHud, 0.25f, 6, 1.0f, DARKGRAY);
    DrawText("Vidas", (int)livesHud.x + 10, (int)livesHud.y + 6, 16, DARKGRAY);

    // --------------------------------------------------------
    // 2. HUD - INTERFAZ DE USUARIO (Lectura desde ECS)
    // --------------------------------------------------------

    // Obtenemos la textura de iconos (Corazones y Llaves)
    Texture2D iconsTex = rm.GetTexture("sprites/icons/Icons.png");

    // Buscamos la entidad que sea JUGADOR (tiene Stats y Input)
    auto view = registry.view<PlayerStatsComponent, TransformComponent, PlayerInputComponent, PlayerCheatComponent, PlayerStateComponent>();

    for(auto entity : view) {
        const auto &stats = view.get<PlayerStatsComponent>(entity);
        const auto &trans = view.get<TransformComponent>(entity);
        const auto &cheats = view.get<PlayerCheatComponent>(entity);
        const auto &playerState = view.get<PlayerStateComponent>(entity);

        // --- A. MOSTRAR LLAVES (MOCHILA) ---
        Rectangle keySrc = {64, 0, 16, 16};
        Rectangle keyDest = {bagHud.x + 10.0f, bagHud.y + 28.0f, 24.0f, 24.0f};
        DrawTexturePro(iconsTex, keySrc, keyDest, {0,0}, 0.0f, WHITE);
        std::string keyText = std::to_string(stats.keysCollected) + " / " + std::to_string(totalKeysInMap_);
        DrawText(keyText.c_str(), (int)bagHud.x + 42, (int)bagHud.y + 30, 20, DARKGRAY);

        // --- B. MOSTRAR VIDAS ---
        Rectangle heartSrc = {80, 0, 16, 16};
        for (int i = 0; i < stats.lives; i++) {
            float hX = livesHud.x + 10.0f + (i * 24.0f);
            float hY = livesHud.y + 28.0f;
            DrawTexturePro(iconsTex, heartSrc,
                Rectangle{hX, hY, 20.0f, 20.0f},
                {0,0}, 0.0f, WHITE);
        }

        // --- C. ESTADO DE CHEATS (HUD) ---
        if (cheats.godMode) {
            DrawText("GOD", (int)livesHud.x + 10, (int)livesHud.y + 8, 16, GOLD);
        }
        if (cheats.noClip) {
            DrawText("NOCLIP", (int)livesHud.x + 60, (int)livesHud.y + 8, 16, GOLD);
        }
        if (playerState.invulnerableTimer > 0.0f && playerState.invulnerableTimer < playerState.invulnerableDuration) {
            DrawText("INVULNERABLE", (int)bagHud.x + 10, (int)bagHud.y - 18, 14, MAROON);
        }

        // --- D. MENSAJE DE SALIDA (Contextual) ---
        // Calcular en qué casilla del mapa está el jugador
        int cx = (int)(trans.position.x / tile_);
        int cy = (int)(trans.position.y / tile_);

        // Verificamos límites del mapa para no leer memoria inválida
        if (cx >= 0 && cx < map_.width() && cy >= 0 && cy < map_.height()) {

            // Si la casilla actual es la SALIDA ('X')
            if (map_.at(cx, cy) == 'X') {
                std::string msg;
                Color msgColor;

                if (stats.keysCollected < totalKeysInMap_) {
                    // Caso: Faltan llaves
                    int remaining = totalKeysInMap_ - stats.keysCollected;
                    msg = "Necesitas " + std::to_string(remaining) + " llave" + (remaining > 1 ? "s" : "") + " más";
                    msgColor = RED;
                } else {
                    // Caso: Nivel completado
                    msg = "¡Presiona ENTER para Siguiente Nivel!";
                    msgColor = GREEN;
                }

                // Centrar el texto en la pantalla
                int textWidth = MeasureText(msg.c_str(), 24);
                DrawText(msg.c_str(), (GetScreenWidth() - textWidth) / 2, (int)baseY - 30, 24, msgColor);
            }
        }
    }


    // Mostrar temporizador centrado encima del HUD (formato mm:ss)
    int timerFont = 22;
    int minutes = (int)levelTime_ / 60;
    int seconds = (int)levelTime_ % 60;
    std::string timeText = "Tiempo: " + std::to_string(minutes) + ":" +
                          (seconds < 10 ? "0" : "") + std::to_string(seconds);
    int textW = MeasureText(timeText.c_str(), timerFont);
    DrawText(timeText.c_str(), (GetScreenWidth() - textW) / 2, (int)baseY + 8, timerFont, DARKGRAY);

    // Mostrar nivel actual arriba a la izquierda
    std::string levelText = "Nivel: " + std::to_string(level_);
    DrawText(levelText.c_str(), 10, 10, 24, DARKGRAY);

}
