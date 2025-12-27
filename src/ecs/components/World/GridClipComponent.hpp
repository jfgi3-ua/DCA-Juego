#pragma once
extern "C" {
  #include <raylib.h>
}

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