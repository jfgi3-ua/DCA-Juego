#pragma once
#include <vector>
#include "core/Config.hpp"
#include "../core/ResourceManager.hpp"

extern "C" {
  #include <raylib.h>
}

enum class MechanismType {
    DOOR,
    TRAP,
    BRIDGE,
    LEVER,
};

class Mechanism {
private:
    MechanismType type_;

    //posiscones en tiles
    IVec2 triggerPos_;
    IVec2 targetPos_;
    bool active_ = true;
    float tileSize_ = TILE_SIZE;

    const Texture2D* mecText_ = nullptr; 
    //verion activada y desactivada en la misma textura pero dsitintas regiones
    Rectangle srcInactive_;
    Rectangle srcActive_;

    const Texture2D* trigerText_ = nullptr;
    Rectangle triggerInactive_;
    Rectangle triggerActive_;


public:
    Mechanism(char type, IVec2 trigger, IVec2 target);

    IVec2 getTriggerPos() const { return triggerPos_; }
    IVec2 getTargetPos() const { return targetPos_; }

    void update();
    void render(int ox, int oy) const;

    void deactivate();

    MechanismType type() const { return type_; }
    bool isActive() const { return active_; }
};
