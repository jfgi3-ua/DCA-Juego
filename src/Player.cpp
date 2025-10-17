#include "Player.hpp"
#include "raymath.h"
#include <iostream>

Player::Player() {}

void Player::init(Vector2 startPos, float radius)
{
    position_ = startPos;
    radius_ = radius;
    has_key_ = false;
}

/**
 * Movimiento con detección de colisiones usando el mapa.
 * No se mueve por casillas: es continuo, con control por teclado.
 */
void Player::handleInput(float deltaTime, const Map& map)
{
    // Nueva posición
    Vector2 newPos = position_;

    // Posición destino
    Vector2 moveDir = { 0, 0 };

    // Actualizar posición dependiendo de la tecla pulsada
    // WASD / △ ◁ ▽ ▷
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    moveDir.y -= speed_ * deltaTime;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  moveDir.y += speed_ * deltaTime;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  moveDir.x -= speed_ * deltaTime;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveDir.x += speed_ * deltaTime;

    // Normalizar dirección si se mueve en diagonal
    if (moveDir.x != 0 || moveDir.y != 0)
        moveDir = Vector2Normalize(moveDir);

    // Posible nueva posición X
    Vector2 tryPosX = {
        position_.x + moveDir.x * speed_ * deltaTime,
        position_.y
    };

    // Comprobación de colisión con paredes en el eje X
    if (!checkCollisionWithWalls(tryPosX, map)) newPos.x = tryPosX.x;

    // Posible nueva posición Y
    Vector2 tryPosY = {
        newPos.x,
        position_.y + moveDir.y * speed_ * deltaTime
    };

    // Comprobación de colisión con paredes en el eje Y
    if (!checkCollisionWithWalls(tryPosY, map)) newPos.y = tryPosY.y;

    // Actualizar posición final
    position_ = newPos;
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
    // Bucle que itera todo el mapa
    for (int y = 0; y < map.height(); ++y) {
        for (int x = 0; x < map.width(); ++x) {
            // Si en la posición indicada hay una pared (#), le crea una variable tipo Rectangle
            if (map.at(x, y) == '#') {
                Rectangle wallRect = {
                    // Posición con el tamaño de las casillas del mapa
                    (float)(x * map.tile()),
                    (float)(y * map.tile()),

                    // Mismo tamaño que las casillas del mapa
                    (float)map.tile(),
                    (float)map.tile()
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

bool Player::isOnExit(const Map& map) const
{
    int tileSize = map.tile();

    // Convertir la posición actual de nuestro jugador a coordenadas de celda
    int cellX = static_cast<int>(position_.x / tileSize);
    int cellY = static_cast<int>(position_.y / tileSize);

    // Comprobar si el carácter del mapa es 'X'
    return map.at(cellX, cellY) == 'X';
}