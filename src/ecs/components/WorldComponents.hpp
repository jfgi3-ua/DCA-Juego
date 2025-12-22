#pragma once
extern "C" {
  #include <raylib.h>
}
#include "ecs/Ecstypes.hpp"
#include "objects/Mechanism.hpp"

// Componente de Transformacion (Posicion y tamano)
struct TransformComponent {
    Vector2 position;
    Vector2 size;
};

// Componente de Sprite (Visualizacion y Animacion)
struct SpriteComponent {
    Texture2D texture;
    int numFrames;
    int currentFrame;
    int currentRow;
    float frameTime;
    float timer;
    Vector2 visualOffset;
    bool flipX;
    Vector2 fixedFrameSize;
    float customScale;

    SpriteComponent(Texture2D tex, int frames = 1, Vector2 offset = {0.0f, 0.0f})
        : texture(tex), numFrames(frames), currentFrame(0), currentRow(0), frameTime(0.1f), timer(0.0f),
          visualOffset(offset), flipX(false), fixedFrameSize({0.0f, 0.0f}), customScale(0.0f) {}
};

// Logica de movimiento por casillas
struct MovementComponent {
    float speed;
    bool isMoving;

    Vector2 startPos;
    Vector2 targetPos;

    float progress;
    float duration;

    MovementComponent(float spd = 150.0f)
        : speed(spd), isMoving(false), startPos{0,0}, targetPos{0,0}, progress(0.0f), duration(0.12f) {}
};

// Componente de Item (Llaves, etc.)
struct ItemComponent {
    bool isKey;
    int value;
    bool collected;

    ItemComponent(bool key = true) : isKey(key), value(1), collected(false) {}
};

// Componente de Colision (Hitbox)
struct ColliderComponent {
    Rectangle rect;
    CollisionType type;
    bool active;

    ColliderComponent(Rectangle r, CollisionType t) : rect(r), type(t), active(true) {}
};

// Componente de Mecanismo (puertas, trampas, etc.)
struct MechanismComponent {
    Mechanism mechanism;

    explicit MechanismComponent(const Mechanism &mech) : mechanism(mech) {}
};

// Componente de Pinchos retractiles
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
