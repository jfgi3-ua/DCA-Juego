#include "MainGameState.hpp"
#include "GameOverState.hpp"
#include "DevModeState.hpp"
#include "StateMachine.hpp"
#include "objects/Enemy.hpp"
#include <iostream>
extern "C" {
  #include <raylib.h>
}

static std::vector<Enemy> enemies;

MainGameState::MainGameState(int level)
{
    level_ = level > 0 ? level : 1;
}

void MainGameState::init()
{
    std::cout << "You are in the Main Game State, level " << level_ << std::endl;

    std::string path = "assets/maps/map_" + std::to_string(level_) + ".txt";
    map_.loadFromFile(path, 32);
    tile_ = map_.tile();

    IVec2 p = map_.playerStart();
    Vector2 startPos = { p.x * (float)tile_ + tile_ / 2.0f,
                         p.y * (float)tile_ + tile_ / 2.0f };
    player_.init(startPos, tile_ * 0.35f, 5);

    enemiesPos_.clear();
    enemies.clear();
    for (auto e : map_.enemyStarts()) {
        enemiesPos_.push_back({ e.x * (float)tile_ + tile_ / 2.0f,
                                e.y * (float)tile_ + tile_ / 2.0f });
        // crear un único Enemy usando el constructor que necesita tile_
        enemies.emplace_back(e.x, e.y, tile_);
    }

    for (auto s : map_.spikesStarts()) {
        spikes_.addSpike(s.x, s.y);
    }

    for (auto m : map_.getMechanisms()) {
        //m es un MechanismPair
        mechanisms_.emplace_back(m.id, m.trigger, m.target);
    }

    // Inicializar temporizador: 30s base + 30s por cada nivel adicional
    levelTime_ = 30.0f + (level_ - 1) * 30.0f;
}

void MainGameState::handleInput()
{
    // Activar menú de desarrollador con CTRL+D
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D)) {
        this->state_machine->add_overlay_state(
            std::make_unique<DevModeState>(&player_, &levelTime_, &freezeEnemies_, &infiniteTime_, &keyGivenByCheating_, level_)
        );
        return;
    }
}

void MainGameState::update(float deltaTime)
{
    activeMechanisms(); //actualizamos un vector con todos los mecanismos activos
    
    // Reducir temporizador de nivel (excepto si está en modo tiempo infinito)
    if (!infiniteTime_) {
        levelTime_ -= deltaTime;
        if (levelTime_ <= 0.0f) {
            levelTime_ = 0.0f;
            // Tiempo agotado -> Game Over (dead = true)
            this->state_machine->add_state(std::make_unique<GameOverState>(level_, true, 0.0f), true);
            return;
        }
    }

    // 1) Actualizar (movimiento) jugador con colisiones de mapa
    player_.update(deltaTime, map_, activeMechanisms_);

    // 2) Celda actual del jugador
    int cellX = (int)(player_.getPosition().x) / tile_;
    int cellY = (int)(player_.getPosition().y) / tile_;

    // 3) ¿Está sobre una llave 'K'? -> Recogerla
    try {
        if (map_.at(cellX, cellY) == 'K') {
            player_.setHasKey(true);
            map_.clearCell(cellX, cellY); // Reemplaza 'K' por '.', es decir, retira la llave del mapa.
            std::cout << "Llave recogida!" << std::endl; // Para debug
            // TODO:Aquí podríamos reproducir un sonido o mostrar un mensaje en pantalla si se desea.
        }
    } catch (const std::out_of_range& e) {
        // Ignorar fuera de rango (no debería ocurrir aquí). Fuera de rango no debería ocurrir si los cálculos de celda son correctos.
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // 4) ¿Está sobre la salida 'X' y tiene la llave? -> Nivel completado
    if (player_.isOnExit(map_) && player_.hasKey()) {
        std::cout << "Nivel completado" << std::endl;
        // Pasar a pantalla de nivel completado (dead = false)
        this->state_machine->add_state(std::make_unique<GameOverState>(level_, false, levelTime_), true);
        return;
    }

    // 5) IA enemigos y colisiones con jugador
    // Pasar la posición del jugador a los enemigos para el árbol de decisiones
    Vector2 playerPos = player_.getPosition();
    
    // Solo actualizar enemigos si no están congelados
    if (!freezeEnemies_) {
        for (auto &e : enemies) {
            e.update(map_, deltaTime, tile_, playerPos.x, playerPos.y);
        }
    }

    enemiesPos_.clear();
    for (auto &e : enemies) {
        enemiesPos_.push_back({ e.x * (float)tile_ + tile_ / 2.0f,
                                e.y * (float)tile_ + tile_ / 2.0f });
    }

    for (auto &e : enemies) {
        if (e.collidesWithPlayer(playerPos.x, playerPos.y, player_.getRadius()) && !player_.isInvulnerable()) {
            // Si colisiona, quitar una vida y empujar al jugador a la casilla previa
            player_.onHit(map_);
            // Notificar al enemigo que golpeó (solo los que persiguen se alejarán)
            e.onHitPlayer();
            std::cout << "El jugador ha sido golpeado por un enemigo. Vidas: " << player_.getLives() << std::endl;
        }
    }


    // 6) Pinchos y colisiones si activo
    spikes_.update(deltaTime);
    // Si el jugador está sobre un pincho activo
    if (spikes_.isActiveAt(cellX, cellY) && !player_.isInvulnerable()) {
    // Si colisiona, quitar una vida y empujar al jugador a la casilla previa
    player_.onHit(map_);
        std::cout << "El jugador ha sido golpeado por pinchos. " << player_.getLives() << std::endl;
    }

    // 7) Si el jugador no tiene vidas, cambiar al estado de Game Over
    if (player_.getLives() <= 0) {
        std::cout << "Game Over: El jugador no tiene más vidas." << std::endl;
        // Game Over por muerte (dead = true)
        this->state_machine->add_state(std::make_unique<GameOverState>(level_, true, levelTime_), true);
        return;
    }

    //7 Mecanismos, cmprobar si el jugador está sobre un trigger q no este activo
    for (auto& mech : mechanisms_) {
        IVec2 trigPos = mech.getTriggerPos();

         if (mech.isActive() && cellX == trigPos.x && cellY == trigPos.y) {
            mech.deactivate();
        }
    }
}

void MainGameState::render()
{
    ClearBackground(RAYWHITE);

    // Dimensiones
    const int mapWpx = map_.width()  * tile_;
    const int mapHpx = map_.height() * tile_;
    const int viewW  = GetScreenWidth();
    const int viewH  = GetScreenHeight() - HUD_HEIGHT; // Espacio disponible sin el HUD

    // Offset centrado (clamp >= 0) - El mapa queda centrado en el espacio disponible
    const int ox = std::max(0, (viewW - mapWpx) / 2);
    const int oy = std::max(0, (viewH - mapHpx) / 2);

    // 1) Mapa (dibujado en la zona superior, desde y=0 hasta y=MAP_H_PX)
    for (int y = 0; y < map_.height(); ++y) {
        for (int x = 0; x < map_.width(); ++x) {
            const char c = map_.at(x, y);
            Rectangle r{ (float)(ox + x * tile_), (float)(oy + y * tile_),(float)tile_, (float)tile_ };

            // Suelo + paredes
            DrawRectangleRec(r, (c == '#') ? LIGHTGRAY : WHITE);

            if (c == '#') DrawRectangleLinesEx(r, 1.0f, DARKGRAY);  // Paredes
            else if (c == 'X') DrawRectangleRec(r, LIME);           // Salida
            else if (c == 'K') {                                    // Llave
                // Pequeño icono dorado centrado en la celda
                Rectangle keyRect{
                    r.x + r.width * 0.25f,
                    r.y + r.height * 0.35f,
                    r.width * 0.5f,
                    r.height * 0.3f
                };
                DrawRectangleRec(keyRect, GOLD);
                DrawRectangleLinesEx(keyRect, 1.5f, BROWN);
            }
        }
    }

    // Mecanismos
    for (const auto& mech : mechanisms_) {
        mech.render(ox, oy);
    }
    // 2) Jugador
    player_.render(ox, oy);

    // 3) Enemigos (cuadrados)
    for (auto &e : enemies) {
        e.draw(tile_, ox, oy, RED);
    }

    spikes_.render(ox, oy);


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
    Rectangle hud{ (float)pad, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad };
    DrawRectangleRounded(hud, 0.25f, 6, Fade(BLACK, 0.10f));
    DrawRectangleRoundedLinesEx(hud, 0.25f, 6, 1.0f, DARKGRAY);
    DrawText("Mochila", (int)hud.x + 10, (int)hud.y + 6, 16, DARKGRAY);

    // Slot de llave
    Rectangle slot{ hud.x + 12, hud.y + 28, 28.0f, 28.0f };
    DrawRectangleLinesEx(slot, 1.0f, GRAY);
    DrawText("Llave", (int)hud.x + 50, (int)hud.y + 30, 16, GRAY);

    if (player_.hasKey()) {
        Rectangle keyIcon{ slot.x + 4, slot.y + 8, slot.width - 8, slot.height - 12 };
        DrawRectangleRec(keyIcon, GOLD);
        DrawRectangleLinesEx(keyIcon, 1.2f, BROWN);
    }

    // Panel de vidas (alineado a la derecha dentro del HUD)
    DrawRectangleRounded(Rectangle{ (float)GetScreenWidth() - 190.0f, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad }, 0.25f, 6, Fade(BLACK, 0.10f));
    DrawRectangleRoundedLinesEx(Rectangle{ (float)GetScreenWidth() - 190.0f, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad }, 0.25f, 6, 1.0f, DARKGRAY);
    DrawText("Vidas", (int)((float)GetScreenWidth() - 190.0f) + 10, (int)(baseY + pad) + 6, 16, DARKGRAY);

    if (player_.getLives() > 0) {
        for (int i = 0; i < player_.getLives(); ++i) {
            DrawCircleV(Vector2{ (float)GetScreenWidth() - 190.0f + 12.0f + i * 20.0f + 6.0f, baseY + pad + 28.0f + 6.0f }, 5.0f, RED);
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

    // 5) Mensaje contextual por encima del HUD
    const int cx = (int)(player_.getPosition().x / tile_);
    const int cy = (int)(player_.getPosition().y / tile_);
    if (cx >= 0 && cy >= 0 && cx < map_.width() && cy < map_.height()) {
        if (map_.at(cx, cy) == 'X' && !player_.hasKey()) {
            const char* msg = "Necesitas la llave para salir";
            const int font = 18;
            const int textW = MeasureText(msg, font);
            // 10 px de margen por encima del HUD
            const int textY = (int)baseY - font - 10;
            DrawText(msg, (GetScreenWidth()-textW)/2, textY, font, MAROON);
        }
    }
}

void MainGameState::activeMechanisms() {
    activeMechanisms_.clear();
    for (const auto& mech : mechanisms_) {
        if (mech.isActive()) {
            Vector2 target = { (float)mech.getTargetPos().x, (float)mech.getTargetPos().y };
            activeMechanisms_.push_back(target);
        }
    }
}
