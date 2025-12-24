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

// Componente de Sprite
struct SpriteComponent {
  Texture2D texture;
  Vector2 visualOffset;
  bool flipX;
  float customScale;

  SpriteComponent(Texture2D tex, Vector2 offset = {0.0f, 0.0f}, float scale = 0.0f)
      : texture(tex), visualOffset(offset), flipX(false), customScale(scale) {}
};

//componente para la animacion o estructura de filas y columnas
struct GridClipComponent {
  int numFrames;
  int currentFrame;
  int currentRow;
  float frameTime;
  float timer;
  Vector2 fixedFrameSize;

  GridClipComponent(int frames = 1)
      : numFrames(frames), currentFrame(0), currentRow(0), frameTime(0.1f), timer(0.0f), fixedFrameSize({0.0f, 0.0f}) {}
};

//Componente de sprites estaticos (sin animacion)
//calculados a mano, no en base a una rejilla
//activo/inactivo (spikes y mecanismos) o fijo
struct ManualSpriteComponent {
  Rectangle src;
  Rectangle srcActive;
  Rectangle srcInactive;

  // Constructor para sprite FIJO como mapa
  explicit ManualSpriteComponent(Rectangle fixed)
      : src(fixed),
        srcActive({0,0,0,0}),
        srcInactive({0,0,0,0}) {}

  // Constructor para sprite ACTIVO / INACTIVO como spikes y mecanismos
  ManualSpriteComponent(Rectangle active, Rectangle inactive)
      : src({0,0,0,0}),
        srcActive(active),
        srcInactive(inactive) {}
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

struct AnimationComponent {
    Texture2D idleTexture;
    Texture2D walkTexture;
    int idleFrames;
    int walkFrames;
    float idleFrameTime;
    float walkFrameTime;
    bool isWalking;

    AnimationComponent(Texture2D idleTex = {}, Texture2D walkTex = {},
                       int idleCount = 1, int walkCount = 1,
                       float idleTime = 0.2f, float walkTime = 0.12f)
        : idleTexture(idleTex),
          walkTexture(walkTex),
          idleFrames(idleCount),
          walkFrames(walkCount),
          idleFrameTime(idleTime),
          walkFrameTime(walkTime),
          isWalking(false) {}
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
