#pragma once
extern "C" {
  #include <raylib.h>
}

struct PlayerCheatComponent {
    bool godMode;
    bool noClip;

    PlayerCheatComponent(bool god = false, bool noclip = false)
        : godMode(god), noClip(noclip) {}
};