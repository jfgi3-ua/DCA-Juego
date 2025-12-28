#include "PlayerSelection.hpp"

namespace {
    std::string g_selectedSpriteId;
    std::string g_selectedIdlePath;
    std::string g_selectedWalkPath;
    bool g_hasSelection = false;
    bool g_hasIdle = false;
    bool g_hasWalk = false;
}

void PlayerSelection::SetSelectedSpriteSet(const std::string& spriteId,
                                           const std::string& idlePath,
                                           const std::string& walkPath,
                                           bool hasIdle,
                                           bool hasWalk) {
    g_selectedSpriteId = spriteId;
    g_selectedIdlePath = idlePath;
    g_selectedWalkPath = walkPath;
    g_hasIdle = hasIdle;
    g_hasWalk = hasWalk;
    g_hasSelection = true;
}

bool PlayerSelection::HasSelectedSpriteSet() {
    return g_hasSelection;
}

const std::string& PlayerSelection::GetSelectedSpriteId() {
    return g_selectedSpriteId;
}

const std::string& PlayerSelection::GetSelectedIdlePath() {
    return g_selectedIdlePath;
}

const std::string& PlayerSelection::GetSelectedWalkPath() {
    return g_selectedWalkPath;
}

bool PlayerSelection::SelectedHasIdle() {
    return g_hasIdle;
}

bool PlayerSelection::SelectedHasWalk() {
    return g_hasWalk;
}

void PlayerSelection::ClearSelectedSprite() {
    g_selectedSpriteId.clear();
    g_selectedIdlePath.clear();
    g_selectedWalkPath.clear();
    g_hasIdle = false;
    g_hasWalk = false;
    g_hasSelection = false;
}
