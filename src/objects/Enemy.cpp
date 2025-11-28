#include "Enemy.hpp"
extern "C" {
  #include <raylib.h>
}
#include <algorithm>
#include <cmath>

static void shuffledNeighbors(int out[4]) {
    out[0]=0; out[1]=1; out[2]=2; out[3]=3;
    for (int i = 3; i > 0; --i) {
        int j = GetRandomValue(0, i);
        std::swap(out[i], out[j]);
    }
}

// ==================== UTILIDADES ====================

// Calcular distancia en píxeles al jugador
float Enemy::distanceToPlayer(float playerX, float playerY) const {
    float dx = (px - playerX);
    float dy = (py - playerY);
    return std::sqrt(dx * dx + dy * dy);
}

// Notificar que golpeó al jugador (solo si está persiguiendo)
void Enemy::onHitPlayer() {
    // Solo los enemigos que estaban persiguiendo se alejan
    if (state == EnemyState::CHASE) {
        state = EnemyState::RETREAT;
        retreatTimer = retreatDuration;
        moving = false; // Detener movimiento actual para recalcular dirección
    }
}

// ==================== UPDATE PRINCIPAL ====================

void Enemy::update(const Map &map, float dt, int tileSize, float playerX, float playerY)
{
    timer += dt;
    
    // Árbol de decisiones de IA
    updateAI(map, dt, tileSize, playerX, playerY);
    
    // Mover hacia el objetivo si está en movimiento
    if (moving) {
        moveTowardsTarget(dt, tileSize);
    }
}

// ==================== ÁRBOL DE DECISIONES ====================

void Enemy::updateAI(const Map &map, float dt, int tileSize, float playerX, float playerY)
{
    float distToPlayer = distanceToPlayer(playerX, playerY);
    float distInTiles = distToPlayer / tileSize;
    
    // Árbol de decisiones basado en el estado actual
    switch (state) {
        case EnemyState::PATROL:
            // ¿El jugador está cerca? -> Cambiar a CHASE
            if (distInTiles <= detectionRange) {
                state = EnemyState::CHASE;
                timer = 0.0f;
                moving = false; // Recalcular dirección inmediatamente
            } else {
                patrolBehavior(map, dt, tileSize);
            }
            break;
            
        case EnemyState::CHASE:
            // ¿El jugador está lejos? -> Volver a PATROL
            // Usamos histéresis (1.5x) para evitar oscilación
            if (distInTiles > detectionRange * 1.5f) {
                state = EnemyState::PATROL;
                timer = 0.0f;
            } else {
                chaseBehavior(map, dt, tileSize, playerX, playerY);
            }
            break;
            
        case EnemyState::RETREAT:
            retreatTimer -= dt;
            // ¿Terminó el tiempo de retroceso? -> Volver a PATROL
            if (retreatTimer <= 0.0f) {
                state = EnemyState::PATROL;
                retreatTimer = 0.0f;
                timer = 0.0f;
            } else {
                retreatBehavior(map, dt, tileSize, playerX, playerY);
            }
            break;
    }
}

// ==================== COMPORTAMIENTOS ====================

// PATROL: Movimiento aleatorio (comportamiento original)
void Enemy::patrolBehavior(const Map &map, float dt, int tileSize)
{
    if (!moving && (moveCooldown == 0.0f || timer >= moveCooldown)) {
        const int dx[4] = {0, 0, 1, -1};
        const int dy[4] = {1, -1, 0, 0};

        int order[4];
        shuffledNeighbors(order);

        bool found = false;
        for (int k = 0; k < 4; ++k) {
            int i = order[k];
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (map.isWalkableForEnemy(nx, ny)) {
                targetX = nx;
                targetY = ny;
                moving = true;
                timer = 0.0f;
                found = true;
                break;
            }
        }
        if (!found) {
            moving = false;
        }
    }
}

// CHASE: Perseguir al jugador inteligentemente
void Enemy::chaseBehavior(const Map &map, float dt, int tileSize, float playerX, float playerY)
{
    // Decidir siguiente movimiento más frecuentemente cuando persigue
    if (!moving && timer >= 0.05f) {
        // Calcular celda del jugador
        int playerCellX = (int)(playerX / tileSize);
        int playerCellY = (int)(playerY / tileSize);
        
        // Calcular dirección hacia el jugador
        int deltaX = playerCellX - x;
        int deltaY = playerCellY - y;
        
        // Priorizar movimiento según la distancia mayor
        int priorities[4]; // 0=abajo, 1=arriba, 2=derecha, 3=izquierda
        
        if (std::abs(deltaX) > std::abs(deltaY)) {
            // Priorizar movimiento horizontal
            if (deltaX > 0) {
                priorities[0] = 2; // derecha (acercarse)
                priorities[1] = (deltaY > 0) ? 0 : 1; // abajo/arriba
                priorities[2] = (deltaY > 0) ? 1 : 0; // arriba/abajo
                priorities[3] = 3; // izquierda (alejarse)
            } else {
                priorities[0] = 3; // izquierda (acercarse)
                priorities[1] = (deltaY > 0) ? 0 : 1;
                priorities[2] = (deltaY > 0) ? 1 : 0;
                priorities[3] = 2; // derecha (alejarse)
            }
        } else {
            // Priorizar movimiento vertical
            if (deltaY > 0) {
                priorities[0] = 0; // abajo (acercarse)
                priorities[1] = (deltaX > 0) ? 2 : 3; // derecha/izquierda
                priorities[2] = (deltaX > 0) ? 3 : 2; // izquierda/derecha
                priorities[3] = 1; // arriba (alejarse)
            } else {
                priorities[0] = 1; // arriba (acercarse)
                priorities[1] = (deltaX > 0) ? 2 : 3;
                priorities[2] = (deltaX > 0) ? 3 : 2;
                priorities[3] = 0; // abajo (alejarse)
            }
        }
        
        const int dx[4] = {0, 0, 1, -1};
        const int dy[4] = {1, -1, 0, 0};
        
        // Intentar moverse en orden de prioridad
        bool found = false;
        for (int k = 0; k < 4; ++k) {
            int i = priorities[k];
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (map.isWalkableForEnemy(nx, ny)) {
                targetX = nx;
                targetY = ny;
                moving = true;
                timer = 0.0f;
                found = true;
                break;
            }
        }
        if (!found) {
            moving = false;
        }
    }
}

// RETREAT: Alejarse del jugador
void Enemy::retreatBehavior(const Map &map, float dt, int tileSize, float playerX, float playerY)
{
    if (!moving && timer >= 0.1f) {
        // Calcular celda del jugador
        int playerCellX = (int)(playerX / tileSize);
        int playerCellY = (int)(playerY / tileSize);
        
        // Calcular dirección OPUESTA al jugador
        int deltaX = x - playerCellX; // Invertido para alejarse
        int deltaY = y - playerCellY; // Invertido para alejarse
        
        // Priorizar movimiento alejándose
        int priorities[4];
        
        if (std::abs(deltaX) > std::abs(deltaY)) {
            if (deltaX > 0) {
                priorities[0] = 2; // derecha (alejarse)
                priorities[1] = (deltaY > 0) ? 0 : 1;
                priorities[2] = (deltaY > 0) ? 1 : 0;
                priorities[3] = 3; // izquierda (acercarse - evitar)
            } else {
                priorities[0] = 3; // izquierda (alejarse)
                priorities[1] = (deltaY > 0) ? 0 : 1;
                priorities[2] = (deltaY > 0) ? 1 : 0;
                priorities[3] = 2; // derecha (acercarse - evitar)
            }
        } else {
            if (deltaY > 0) {
                priorities[0] = 0; // abajo (alejarse)
                priorities[1] = (deltaX > 0) ? 2 : 3;
                priorities[2] = (deltaX > 0) ? 3 : 2;
                priorities[3] = 1; // arriba (acercarse - evitar)
            } else {
                priorities[0] = 1; // arriba (alejarse)
                priorities[1] = (deltaX > 0) ? 2 : 3;
                priorities[2] = (deltaX > 0) ? 3 : 2;
                priorities[3] = 0; // abajo (acercarse - evitar)
            }
        }
        
        const int dx[4] = {0, 0, 1, -1};
        const int dy[4] = {1, -1, 0, 0};
        
        bool found = false;
        for (int k = 0; k < 4; ++k) {
            int i = priorities[k];
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (map.isWalkableForEnemy(nx, ny)) {
                targetX = nx;
                targetY = ny;
                moving = true;
                timer = 0.0f;
                found = true;
                break;
            }
        }
        if (!found) {
            // Si no puede alejarse, quedarse quieto
            moving = false;
        }
    }
}

// ==================== MOVIMIENTO ====================

void Enemy::moveTowardsTarget(float dt, int tileSize)
{
    float targetPx = targetX * tileSize + tileSize * 0.5f;
    float targetPy = targetY * tileSize + tileSize * 0.5f;

    float dx = targetPx - px;
    float dy = targetPy - py;
    float dist = std::sqrt(dx*dx + dy*dy);

    if (dist <= 0.0001f) {
        px = targetPx;
        py = targetPy;
        x = targetX;
        y = targetY;
        moving = false;
        timer = 0.0f;
        return;
    }

    // Velocidad variable según el estado
    float currentSpeed = speed;
    if (state == EnemyState::CHASE) {
        currentSpeed = speed * 1.3f; // 30% más rápido cuando persigue
    } else if (state == EnemyState::RETREAT) {
        currentSpeed = speed * 1.5f; // 50% más rápido cuando huye
    }

    float step = currentSpeed * dt;
    if (step >= dist) {
        px = targetPx;
        py = targetPy;
        x = targetX;
        y = targetY;
        moving = false;
        timer = 0.0f;
    } else {
        px += dx / dist * step;
        py += dy / dist * step;
    }
}

// ==================== COLISIONES Y RENDER ====================

bool Enemy::collidesWithPlayer(float playerPx, float playerPy, float playerRadius) const
{
    // rectángulo centrado en (px,py) con medidas bboxW/2, bboxH/2
    float halfW = bboxW * 0.5f;
    float halfH = bboxH * 0.5f;
    float rectLeft = px - halfW;
    float rectRight = px + halfW;
    float rectTop = py - halfH;
    float rectBottom = py + halfH;

    // punto más cercano del rectángulo al centro del círculo
    float closestX = playerPx;
    if (closestX < rectLeft) closestX = rectLeft;
    if (closestX > rectRight) closestX = rectRight;

    float closestY = playerPy;
    if (closestY < rectTop) closestY = rectTop;
    if (closestY > rectBottom) closestY = rectBottom;

    float dx = playerPx - closestX;
    float dy = playerPy - closestY;
    return (dx*dx + dy*dy) <= (playerRadius * playerRadius);
}

void Enemy::draw(int tileSize, int ox, int oy, Color color) const {
    float s = tileSize * 0.7f;
    Rectangle r{ (px + ox) - s/2.0f, (py + oy) - s/2.0f, s, s };
    
    // Color según el estado (para debug visual)
    Color drawColor = color;
    if (state == EnemyState::CHASE) {
        drawColor = ORANGE; // Naranja cuando persigue
    } else if (state == EnemyState::RETREAT) {
        drawColor = PURPLE; // Púrpura cuando huye
    }
    // PATROL mantiene el color por defecto (RED)
    
    DrawRectangleRec(r, drawColor);
}
