#include "MainGameState.hpp"
#include <iostream>
extern "C" {
  #include <raylib.h>
}

MainGameState::MainGameState()
{
}

void MainGameState::init()
{
    std::cout << "You are in the Main Game State" << std::endl;

    map_.loadFromFile("assets/maps/level1.txt", 32);
    tile_ = map_.tile();

    IVec2 p = map_.playerStart();
    playerPos_ = { p.x * (float)tile_ + tile_ / 2.0f,
                   p.y * (float)tile_ + tile_ / 2.0f };

    enemiesPos_.clear();
    for (auto e : map_.enemyStarts()) {
        enemiesPos_.push_back({ e.x * (float)tile_ + tile_ / 2.0f,
                                e.y * (float)tile_ + tile_ / 2.0f });
    }
}

void MainGameState::handleInput()
{
    std::cout << "Enter a key: ";
}

void MainGameState::update(float deltaTime)
{
    // this->handleInput();
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

    // Jugador (círculo)
    DrawCircleV(playerPos_, tile_ * 0.35f, BLUE);

    // Enemigos (cuadrados)
    for (auto pos : enemiesPos_) {
        DrawRectangleV({ pos.x - tile_ * 0.35f, pos.y - tile_ * 0.35f },
                       { tile_ * 0.7f, tile_ * 0.7f }, RED);
    }

    EndDrawing();
}