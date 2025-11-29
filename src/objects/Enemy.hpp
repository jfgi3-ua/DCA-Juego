#pragma once
#include "Map.hpp"
extern "C" {
  #include <raylib.h>
}

// Estados de la IA del enemigo (Árbol de decisiones)
enum class EnemyState {
    PATROL,   // Movimiento aleatorio (patrullando)
    CHASE,    // Persiguiendo al jugador
    RETREAT   // Alejándose después de golpear
};

class Enemy {
public:
    int x = 0;
    int y = 0;
    int targetX = 0;
    int targetY = 0;
    float px = 0.0f;
    float py = 0.0f;
    float speed = 0.0f;
    float moveCooldown = 0.0f;
    float timer = 0.0f;
    bool moving = false;

    float bboxW = 0.0f;
    float bboxH = 0.0f;

    // Variables para el árbol de decisiones de IA
    EnemyState state = EnemyState::PATROL;
    float detectionRange = 6.0f;    // Rango en tiles para detectar al jugador (aumentado para mayor visión)
    float retreatTimer = 0.0f;       // Tiempo restante en estado RETREAT
    float retreatDuration = 3.0f;    // Duración del retroceso (segundos)

    bool collidesWithPlayer(float playerPx, float playerPy, float playerRadius) const;


    Enemy() = default;
    Enemy(int x_, int y_, int tileSize) : x(x_), y(y_), targetX(x_), targetY(y_) {
        px = x * tileSize + tileSize * 0.5f;
        py = y * tileSize + tileSize * 0.5f;

        speed = tileSize * 2.5f;
        moveCooldown = 0.0f;
        timer = 0.0f;
        moving = false;
        bboxW = tileSize * 0.7f;
        bboxH = tileSize * 0.7f;
        state = EnemyState::PATROL;
        retreatTimer = 0.0f;
    }

    void update(const Map &map, float dt, int tileSize, float playerX, float playerY);

    // Método para notificar que golpeó al jugador (cambia a estado RETREAT)
    void onHitPlayer();

    // void draw(int tileSize, Color color = RED) const; // método de dibujado sin offset --- IGNORE ---
    void draw(int tileSize, int ox, int oy, Color color = RED) const; // método de dibujado con offset (ox, oy) ¡¡Hugh!! Este es el que hay que usar ahora

private:
    // Árbol de decisiones de IA
    void updateAI(const Map &map, float dt, int tileSize, float playerX, float playerY);
    
    // Estados específicos
    void patrolBehavior(const Map &map, float dt, int tileSize);
    void chaseBehavior(const Map &map, float dt, int tileSize, float playerX, float playerY);
    void retreatBehavior(const Map &map, float dt, int tileSize, float playerX, float playerY);
    
    // Utilidades
    float distanceToPlayer(float playerX, float playerY) const;
    void moveTowardsTarget(float dt, int tileSize);
    bool hasLineOfSight(const Map &map, int playerCellX, int playerCellY) const;
};