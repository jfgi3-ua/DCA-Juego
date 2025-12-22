#pragma once
extern "C" {
  #include <raylib.h>
}
#include "objects/Mechanism.hpp"

// Componente de Transformación (Posición y Escala)
struct TransformComponent {
    Vector2 position;
    Vector2 size;
};

// Componente de Sprite (Visualización y Animación)
struct SpriteComponent {
    Texture2D texture;
    int numFrames;          // Cuántos cuadros tiene la hoja
    int currentFrame;       // Cuadro actual a dibujar
    int currentRow;         // Fila actual (para hojas con múltiples filas - ATLAS)
    float frameTime;        // Tiempo por cuadro (para animar luego)
    float timer;            // Acumulador de tiempo
    Vector2 visualOffset;   // Ajuste visual (Offset) - Para renderizar correctamente si la textura no encaja bien
    bool flipX;             // Para girar el sprite
    Vector2 fixedFrameSize; // Tamaño fijo del frame (0,0) si no se usa
    float customScale;      // 0.0f = Auto (Fórmula Player), >0.0f = Manual

    SpriteComponent(Texture2D tex, int frames = 1, Vector2 offset = {0.0f, 0.0f})
            : texture(tex), numFrames(frames), currentFrame(0), currentRow(0), frameTime(0.1f), timer(0.0f),
              visualOffset(offset), flipX(false), fixedFrameSize({0.0f, 0.0f}), customScale(0.0f) {}
};


// Lógica de movimiento por casillas
struct MovementComponent {
    float speed;            // Velocidad en pixels/segundo (ej: 150.0f)
    bool isMoving;          // ¿Se está moviendo actualmente?

    Vector2 startPos;       // Posición de inicio de la transición
    Vector2 targetPos;      // Posición destino (centro de la casilla)

    float progress;         // 0.0f a 1.0f (progreso del movimiento)
    float duration;         // Tiempo total que durará el viaje

    // Constructor con valores por defecto similares a tu Player.cpp
    MovementComponent(float spd = 150.0f)
        : speed(spd), isMoving(false), startPos{0,0}, targetPos{0,0}, progress(0.0f), duration(0.12f) {}
};

// Tipo de colisión para identificar la reacción
enum class CollisionType {
    None,
    Player,
    Enemy,
    Spike,
    Item
};

// Componente de Estadísticas (Para el Jugador)
struct StatsComponent {
    int lives;
    int keysCollected;

    StatsComponent(int l = 5) : lives(l), keysCollected(0) {}
};

// Componente de Ítem (Para Llaves, Pociones, etc. lo que se nos ocurra si es que nos da por meter más cosas)
struct ItemComponent {
    bool isKey;      // true = Llave, false = Puntos/Vida/Otro
    int value;       // Cantidad a sumar (ej: 1 llave, 100 puntos)
    bool collected;  // Para marcar si debe ser borrado

    ItemComponent(bool key = true) : isKey(key), value(1), collected(false) {}
};

// Componente de Colisión (Hitbox)
struct ColliderComponent {
    // Rectángulo relativo a la posición de la entidad
    // x, y son offsets; width, height son el tamaño de la caja
    Rectangle rect;
    CollisionType type;
    bool active; // Para desactivar colisiones temporalmente (ej: invulnerabilidad del modo GOD ese que tenemos)

    ColliderComponent(Rectangle r, CollisionType t) : rect(r), type(t), active(true) {}
};

// Etiqueta para el jugador
struct PlayerInputComponent {};

// Componente de Mecanismo (puertas, trampas, etc.)
struct MechanismComponent {
    Mechanism mechanism;

    explicit MechanismComponent(const Mechanism &mech) : mechanism(mech) {}
};

// Componente de Pinchos retráctiles
struct SpikeComponent {
    bool active;
    float interval;
    float activeOffsetY;
    float inactiveOffsetY;

    SpikeComponent(bool isActive = true, float intervalSeconds = 3.0f,
                   float activeOffset = 0.0f, float inactiveOffset = 0.0f)
        : active(isActive),
          interval(intervalSeconds),
          activeOffsetY(activeOffset),
          inactiveOffsetY(inactiveOffset) {}
};

struct PlayerStateComponent {
    Vector2 lastTilePos;
    float invulnerableTimer;
    float invulnerableDuration;

    PlayerStateComponent(Vector2 startPos = {0.0f, 0.0f}, float duration = 1.5f)
        : lastTilePos(startPos), invulnerableTimer(0.0f), invulnerableDuration(duration) {}
};

enum class EnemyAIState {
    Patrol,
    Chase,
    Retreat
};

struct EnemyAIComponent {
    EnemyAIState state = EnemyAIState::Patrol;
    float detectionRange = 6.0f; // en tiles
    float retreatTimer = 0.0f;
    float retreatDuration = 3.0f;
    float moveCooldown = 0.0f;
    float timer = 0.0f;
};
