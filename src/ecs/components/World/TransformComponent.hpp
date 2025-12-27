#pragma once
extern "C" {
  #include <raylib.h>
}

// Componente de Transformacion (Posicion y tamano)
struct TransformComponent {
    Vector2 position;
    Vector2 size;
};