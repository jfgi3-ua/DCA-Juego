#pragma once
extern "C" {
  #include <raylib.h>
}

// Componente de Transformación (Posición y Escala)s
struct TransformComponent {
    Vector2 position;
    Vector2 size;
};

// Componente de Sprite (Visualización y Animación)
struct SpriteComponent {
    Texture2D texture;
    int numFrames;        // Cuántos cuadros tiene la hoja
    int currentFrame;     // Cuadro actual a dibujar
    float frameTime;      // Tiempo por cuadro (para animar luego)
    float timer;          // Acumulador de tiempo
    Vector2 visualOffset; // Ajuste visual (Offset) - Para renderizar correctamente si la textura no encaja bien
    bool flipX;           // Para girar el sprite

  SpriteComponent(Texture2D tex, int frames = 1, Vector2 offset = {0.0f, 0.0f})
          : texture(tex), numFrames(frames), currentFrame(0), frameTime(0.1f), timer(0.0f),
            visualOffset(offset), flipX(false) {} // Inicializamos flipX en false
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

// Etiqueta para el jugador
struct PlayerInputComponent {};