#pragma once
extern "C" {
  #include <raylib.h>
}

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