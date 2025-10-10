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

    map_.loadFromFile("assets/maps/level1.txt", 32);
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
}

void MainGameState::handleInput()
{
    if(IsKeyPressed(KEY_SPACE)){
        this->state_machine->add_state(std::make_unique<GameOverState>(1, 1, 1.0f), true);
    }
}

void MainGameState::update(float deltaTime)
{
    // Actualizar jugador con colisiones de mapa
    player_.update(deltaTime, map_);

    if (player_.isOnExit(map_)) {
        std::cout << "Nivel completado" << std::endl;
        this->state_machine->add_state(std::make_unique<GameOverState>(1, 0, 1.0f), true);
    }
    
    //this->handleInput();

    for (auto &e : enemies) e.update(map_, deltaTime, tile_);

    enemiesPos_.clear();
    for (auto &e : enemies) {
        enemiesPos_.push_back({ e.x * (float)tile_ + tile_ / 2.0f,
                                e.y * (float)tile_ + tile_ / 2.0f });
    }

    float playerRadius = tile_ * 0.35f;
    for (auto &e : enemies) {
        if (e.collidesWithPlayer(playerPos_.x, playerPos_.y, playerRadius)) {
            // si colisiona, cambiar a GameOverState (ajusta parámetros si tu constructor difiere)
            this->state_machine->add_state(std::make_unique<GameOverState>(1, 1, 1.0f), true);
            break;
        }
    }
}
void MainGameState::render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    for (int y = 0; y < map_.height(); ++y) {
        for (int x = 0; x < map_.width(); ++x) {
            const char c = map_.at(x, y);
            Rectangle r{ (float)(x * tile_), (float)(y * tile_), (float)tile_, (float)tile_ };

            // Suelo
            DrawRectangleRec(r, (c == '#') ? LIGHTGRAY : WHITE);

            // Paredes
            if (c == '#') DrawRectangleLinesEx(r, 1.0f, DARKGRAY);

            // Salida
            if (c == 'X') DrawRectangleRec(r, LIME);
        }
    }

    // Jugador 
    player_.render();

    // Enemigos (cuadrados)
    for (auto &e : enemies) {
        e.draw(tile_, RED);
    }

    EndDrawing();
}