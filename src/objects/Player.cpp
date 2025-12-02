#include "Player.hpp"

// Estado de movimiento por casillas a nivel de fichero (no como miembros de clase)
static bool s_moving = false;
static Vector2 s_move_start = {0,0};
static Vector2 s_move_target = {0,0};
static float s_move_progress = 0.0f;
static float s_move_duration = 0.2f;

Player::Player() {}

void Player::init(Vector2 startPos, float radius, int lives)
{
    position_ = startPos;
    radius_ = radius;
    key_count_ = 0;
    lives_ = lives;
}

/**
 * Movimiento con detección de colisiones usando el mapa.
 * No se mueve por casillas: es continuo, con control por teclado.
 */
void Player::handleInput(float deltaTime, const Map& map, const std::vector<Vector2>& blockedTiles)
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
    
    // Verificar límites del mapa (incluso en NoClip)
    if (targetX < 0 || targetX >= map.width() || targetY < 0 || targetY >= map.height()) {
        return; // No permitir salir del mapa
    }
    
    // Comprobar colisión en la posición objetivo (evita tocar esquinas)
    // En modo noClip se puede atravesar paredes INTERNAS, pero no los bordes del mapa
    Vector2 centerTarget = { targetX * (float)tileSize + tileSize / 2.0f,
                             targetY * (float)tileSize + tileSize / 2.0f };
    if (!noClip_ && checkCollisionWithWalls(centerTarget, map, blockedTiles)) return;

    // Guardar última dirección de movimiento
    lastMoveDir_ = position_;

    // Iniciar animación hacia la celda destino
    move_start_ = position_;
    move_target_ = centerTarget;
    move_progress_ = 0.0f;
    moving_ = true;
}

void Player::update(float deltaTime, const Map& map, const std::vector<Vector2>& blockedTiles)
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
        handleInput(deltaTime, map, blockedTiles);
    }
    if (invulnerableTimer_ > 0.0f && invulnerableTimer_ < Player::INVULNERABLE_DURATION) {
        invulnerableTimer_ += deltaTime;
        if (invulnerableTimer_ > Player::INVULNERABLE_DURATION) invulnerableTimer_ = Player::INVULNERABLE_DURATION;
    }
}

void Player::render(int ox, int oy) const
{
    Vector2 p = { position_.x + (float)ox, position_.y + (float)oy };
    
    // Color: DORADO si hay algún cheat activo, AZUL normal
    Color playerColor = hasAnyCheatsActive() ? GOLD : BLUE;
    
    if (invulnerableTimer_ > 0.0f && invulnerableTimer_ < Player::INVULNERABLE_DURATION && !godMode_) {
        // Parpadeo de invulnerabilidad (no en god mode)
        int blinkFrequency = 10; // Frecuencia de parpadeo
        if (static_cast<int>(invulnerableTimer_ * blinkFrequency) % 2 == 0) {
            DrawCircleV(p, radius_, playerColor);
        }
    } else {
        DrawCircleV(p, radius_, playerColor);
    }
}

bool Player::checkCollisionWithWalls(const Vector2& pos,
                                     const Map& map,
                                     const std::vector<Vector2>& blockedTiles) const
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
    //Comprobar mecanismos activos (bloqueos temporales)
    for (const auto& b : blockedTiles) {
        Rectangle mechRect = {
            b.x * map.tile(),
            b.y * map.tile(),
            (float)map.tile(),
            (float)map.tile()
        };
        if (CheckCollisionCircleRec(pos, radius_, mechRect))
            return true;
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

void Player::onHit(const Map& map) 
{
    // En modo God, no recibe daño
    if (godMode_) return;
    
    // Si no está en periodo de invulnerabilidad (valor >= INVULNERABLE_DURATION) o es el estado inicial (<= 0.0),
    // recibe daño y se inicia el conteo desde 0.0 hasta INVULNERABLE_DURATION.
    if (lives_ > 0 && !isInvulnerable()) {
        // Quitar vida
        lives_--;

        // Calcular la celda previa usando la posición guardada en lastMoveDir_
        int tileSize = map.tile();
        int prevCellX = static_cast<int>(lastMoveDir_.x) / tileSize;
        int prevCellY = static_cast<int>(lastMoveDir_.y) / tileSize;

        // Validar que la posición de respawn esté dentro de los límites del mapa
        // y que no sea una pared
        bool validRespawn = true;
        
        if (prevCellX < 0 || prevCellX >= map.width() || 
            prevCellY < 0 || prevCellY >= map.height()) {
            validRespawn = false;
        } else if (map.at(prevCellX, prevCellY) == '#') {
            validRespawn = false;
        }

        // Si la posición de respawn no es válida, usar la posición inicial del jugador
        Vector2 respawnPos;
        if (!validRespawn) {
            IVec2 startPos = map.playerStart();
            respawnPos.x = startPos.x * (float)tileSize + tileSize / 2.0f;
            respawnPos.y = startPos.y * (float)tileSize + tileSize / 2.0f;
        } else {
            respawnPos.x = prevCellX * (float)tileSize + tileSize / 2.0f;
            respawnPos.y = prevCellY * (float)tileSize + tileSize / 2.0f;
        }

        // Validar nuevamente que la posición final no tenga colisión con paredes
        // (por si acaso, validación extra de seguridad)
        std::vector<Vector2> emptyBlocked; // Vector vacío para la validación
        if (!checkCollisionWithWalls(respawnPos, map, emptyBlocked)) {
            position_ = respawnPos;
        } else {
            // Si incluso la posición inicial tiene colisión (caso muy raro),
            // colocar en la posición de spawn sin validación adicional
            IVec2 startPos = map.playerStart();
            position_.x = startPos.x * (float)tileSize + tileSize / 2.0f;
            position_.y = startPos.y * (float)tileSize + tileSize / 2.0f;
        }

        // Iniciar invulnerabilidad
        invulnerableTimer_ = 0.0001f;
    }
}

bool Player::isInvulnerable() const
{
    // God mode es invulnerabilidad permanente
    if (godMode_) return true;
    
    return invulnerableTimer_ > 0.0f && invulnerableTimer_ < Player::INVULNERABLE_DURATION;
}