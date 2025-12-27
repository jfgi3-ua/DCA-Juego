#pragma once
extern "C" {
  #include <raylib.h>
}
#include <ecs/Ecstypes.hpp>

// Componente de Colision (Hitbox)
struct ColliderComponent {
    Rectangle rect;
    CollisionType type;
    bool active;

    ColliderComponent(Rectangle r, CollisionType t) : rect(r), type(t), active(true) {}
};