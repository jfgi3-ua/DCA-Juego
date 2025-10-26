#include "MainGameState.hpp"
#include "GameOverState.hpp"
#include "StateMachine.hpp"
#include "Enemy.hpp"
#include <iostream>
extern "C" {
  #include <raylib.h>
}

static std::vector<Enemy> enemies;

MainGameState::MainGameState()
{
}

void MainGameState::init()
{
    std::cout << "You are in the Main Game State" << std::endl;

    map_.loadFromFile("assets/maps/map_m_20x16.txt", 32);
    tile_ = map_.tile();

    IVec2 p = map_.playerStart();
    Vector2 startPos = { p.x * (float)tile_ + tile_ / 2.0f,
                         p.y * (float)tile_ + tile_ / 2.0f };
    player_.init(startPos, tile_ * 0.35f);

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
}

void MainGameState::handleInput()
{
    if(IsKeyPressed(KEY_SPACE)){
        this->state_machine->add_state(std::make_unique<GameOverState>(1, 1, 1.0f), true);
    }
}

void MainGameState::update(float deltaTime)
{
    activeMechanisms(); //actualizamos un vector con todos los mecanismos activos
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
        this->state_machine->add_state(std::make_unique<GameOverState>(1, 0, 1.0f), true);
    }

    // 5) IA enemigos y colisiones con jugador
    for (auto &e : enemies) e.update(map_, deltaTime, tile_);

    enemiesPos_.clear();
    for (auto &e : enemies) {
        enemiesPos_.push_back({ e.x * (float)tile_ + tile_ / 2.0f,
                                e.y * (float)tile_ + tile_ / 2.0f });
    }

    for (auto &e : enemies) {
        if (e.collidesWithPlayer(player_.getPosition().x, player_.getPosition().y, player_.getRadius())) {
            // si colisiona, cambiar a GameOverState (ajusta parámetros si tu constructor difiere)
            this->state_machine->add_state(std::make_unique<GameOverState>(1, 1, 1.0f), true);
            break;
        }
    }

    //6 Pinchos y colisiones si activo
    spikes_.update(deltaTime);

    // Si el jugador está sobre un pincho activo
    if (spikes_.isActiveAt(cellX, cellY)) {
        std::cout << "Player died by spikes!" << std::endl;
        this->state_machine->add_state(std::make_unique<GameOverState>(1, 1, 1.0f), true);
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
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Dimensiones
    const int mapWpx = map_.width()  * tile_;
    const int mapHpx = map_.height() * tile_;
    const int viewW  = GetScreenWidth();
    const int viewH  = GetScreenHeight() - HUD_HEIGHT;

    // Offset centrado (clamp >= 0)
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

    // 2) Jugador
    player_.render(ox, oy);

    // 3) Enemigos (cuadrados)
    for (auto &e : enemies) {
        e.draw(tile_, ox, oy, RED);
    }

    spikes_.render(ox, oy);

    // Mecanismos
    for (const auto& mech : mechanisms_) {
        mech.render(ox, oy);
    }

    // 4) HUD inferior
    const float baseY = (float)(oy + mapHpx); // empieza justo bajo el mapa
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

    EndDrawing();
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