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
    UNKNOWN
};

class Mechanism {
private:
    MechanismType type_;

    //posiscones en tiles
    Vector2 triggerPos_;
    Vector2 targetPos_;
    bool active_ = false;
    float tileSize_ = TILE_SIZE;

public:
    Mechanism(MechanismType type, Vector2 trigger, Vector2 target);

    void update();
    void render(int ox, int oy) const;

    bool isBlockingAt(int gridX, int gridY) const;
    void setActive(bool state) { active_ = state; }

    MechanismType type() const { return type_; }
    bool isActive() const { return active_; }
};
