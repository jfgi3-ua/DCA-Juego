#include "Player.hpp"
#include <iostream>

Player::Player() {}

void Player::init(Vector2 startPos, float radius)
{
    position_ = startPos;
    radius_ = radius;
}

/**
 * Movimiento con detección de colisiones usando el mapa.
 * No se mueve por casillas: es continuo, con control por teclado.
 */
void Player::handleInput(float deltaTime, const Map& map)
{
    Vector2 newPos = position_;
    int tileSize = map.tile();

    // Movimiento base
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    newPos.y -= speed_ * deltaTime;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  newPos.y += speed_ * deltaTime;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  newPos.x -= speed_ * deltaTime;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) newPos.x += speed_ * deltaTime;

    // Colisiones en eje X 
    Vector2 tryX = { newPos.x, position_.y };
    int gridX = static_cast<int>(tryX.x / tileSize);
    int gridY = static_cast<int>(position_.y / tileSize);

    if (map.isWalkable(gridX, gridY)) {
        position_.x = tryX.x;
    }

    // Colisiones en eje Y 
    Vector2 tryY = { position_.x, newPos.y };
    int gridX2 = static_cast<int>(position_.x / tileSize);
    int gridY2 = static_cast<int>(tryY.y / tileSize);

    if (map.isWalkable(gridX2, gridY2)) {
        position_.y = tryY.y;
    }
}

void Player::update(float deltaTime, const Map& map)
{
    handleInput(deltaTime, map);
}

void Player::render() const
{
    DrawCircleV(position_, radius_, BLUE);
}

bool Player::checkCollisionWithWalls(const Vector2& pos, const Map& map) const
{
    int tileSize = map.tile();

    // Bucle que itera todo el mapa
    for (int y = 0; y < map.height(); ++y) {
        for (int x = 0; x < map.width(); ++x) {
            // Si en la posición indicada hay una pared (#), le crea una variable tipo Rectangle
            if (map.at(x, y) == '#') {
                Rectangle wallRect = {
                    // Posición con el tamaño de las casillas del mapa
                    (float)(x * tileSize),
                    (float)(y * tileSize),

                    // Mismo tamaño que las casillas del mapa
                    (float)tileSize,
                    (float)tileSize
                };

                // Si nuestro jugador choca contra una de estas lo detecta
                if (CheckCollisionCircleRec(pos, radius_, wallRect)) {
                    // Hay colisión
                    return true; 
                }
            }
        }
    }

     // No hay colisión
    return false;
}