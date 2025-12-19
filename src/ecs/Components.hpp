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
    int numFrames;      // Cuántos cuadros tiene la hoja
    int currentFrame;   // Cuadro actual a dibujar
    float frameTime;    // Tiempo por cuadro (para animar luego)
    float timer;        // Acumulador de tiempo

  // Ajuste visual (Offset) - Para renderizar correctamente si la textura no encaja bien
    Vector2 visualOffset;

    SpriteComponent(Texture2D tex, int frames = 1, Vector2 offset = {0.0f, 0.0f})
        : texture(tex), numFrames(frames), currentFrame(0), frameTime(0.1f), timer(0.0f), visualOffset(offset) {}
};