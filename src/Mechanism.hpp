#pragma once
#include <vector>
#include "Config.hpp"

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

public:
    Mechanism(char type, IVec2 trigger, IVec2 target);

    IVec2 getTriggerPos() const { return triggerPos_; }

    void update();
    void render(int ox, int oy) const;

    void setActive();

    MechanismType type() const { return type_; }
    bool isActive() const { return active_; }
};
