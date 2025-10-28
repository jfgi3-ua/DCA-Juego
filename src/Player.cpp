#include "Player.hpp"

// Estado de movimiento por casillas a nivel de fichero (no como miembros de clase)
static bool s_moving = false;
static Vector2 s_move_start = {0,0};
static Vector2 s_move_target = {0,0};
static float s_move_progress = 0.0f;
static float s_move_duration = 0.2f;

Player::Player() {}

void Player::init(Vector2 startPos, float radius)
{
    position_ = startPos;
    radius_ = radius;
    has_key_ = false;
}

void Player::handleInput(float deltaTime, const Map& map)
{
    // Si ya estamos moviéndonos, ignorar nuevos inputs aquí
    if (moving_) return;

    int dx = 0, dy = 0;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    dy = -1;
    else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) dy = 1;
    else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) dx = -1;
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dx = 1;

    // Si no hay movimiento, salir para optimizar el rendimiento
    if (dx == 0 && dy == 0) return;

    // Calcular celda actual y destino
    int tileSize = map.tile();
    int cellX = static_cast<int>(position_.x / tileSize);
    int cellY = static_cast<int>(position_.y / tileSize);
    int targetX = cellX + dx;
    int targetY = cellY + dy;

    // Comprobar que la celda destino es accesible
    if (!map.isWalkable(targetX, targetY)) return;

    // Comprobar colisión en la posición objetivo (evita tocar esquinas)
    Vector2 centerTarget = { targetX * (float)tileSize + tileSize / 2.0f,
                             targetY * (float)tileSize + tileSize / 2.0f };
    if (checkCollisionWithWalls(centerTarget, map)) return;

    // Iniciar animación hacia la celda destino
    move_start_ = position_;
    move_target_ = centerTarget;
    move_progress_ = 0.0f;
    moving_ = true;
}

void Player::update(float deltaTime, const Map& map)
{
    // Si estamos en transición entre casillas, interpolar posición
    if (moving_) {
        move_progress_ += deltaTime;
        float t = move_progress_ / move_duration_;
        if (t >= 1.0f) {
            position_ = move_target_;
            moving_ = false;
            move_progress_ = 0.0f;
        } else {
            position_.x = move_start_.x + (move_target_.x - move_start_.x) * t;
            position_.y = move_start_.y + (move_target_.y - move_start_.y) * t;
        }
    } else {
        handleInput(deltaTime, map);
    }
}

void Player::render(int ox, int oy) const
{
    Vector2 p = { position_.x + (float)ox, position_.y + (float)oy };
    DrawCircleV(p, radius_, BLUE);
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