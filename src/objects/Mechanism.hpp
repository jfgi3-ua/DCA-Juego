#pragma once
#include <vector>
#include "core/Config.hpp"
#include "../core/ResourceManager.hpp"

extern "C" {
  #include <raylib.h>
}

class Mechanism {
public:
    Mechanism(MechanismType type, IVec2 trigger, IVec2 target);

    IVec2 getTriggerPos() const { return _triggerPos; }
    IVec2 getTargetPos() const { return _targetPos; }

    void update();
    void render(int ox, int oy) const;

    void deactivate();

    MechanismType type() const { return _type; }
    bool isActive() const { return _active; }

private:
    MechanismType _type;

    //posiscones en tiles
    IVec2 _triggerPos;
    IVec2 _targetPos;
    bool _active = true;
    float _tileSize = TILE_SIZE;

    const Texture2D* _mecText = nullptr; 
    //verion activada y desactivada en la misma textura pero dsitintas regiones
    Rectangle _srcInactive;
    Rectangle _srcActive;

    const Texture2D* _trigerText = nullptr;
    Rectangle _triggerInactive;
    Rectangle _triggerActive;
};
