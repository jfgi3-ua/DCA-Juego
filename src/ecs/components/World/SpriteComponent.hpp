#pragma once
extern "C" {
  #include <raylib.h>
}

// Componente de Sprite
struct SpriteComponent {
  Texture2D texture;
  Vector2 visualOffset;
  bool flipX;
  float customScale;

  SpriteComponent(Texture2D tex, Vector2 offset = {0.0f, 0.0f}, float scale = 0.0f)
      : texture(tex), visualOffset(offset), flipX(false), customScale(scale) {}
};