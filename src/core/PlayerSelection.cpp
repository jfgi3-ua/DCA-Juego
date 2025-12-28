#include "PlayerSelection.hpp"

namespace {
    std::string g_selectedSpriteId;
}

void PlayerSelection::SetSelectedSpriteId(const std::string& spriteId) {
    g_selectedSpriteId = spriteId;
}

const std::string& PlayerSelection::GetSelectedSpriteId() {
    return g_selectedSpriteId;
}

void PlayerSelection::ClearSelectedSpriteId() {
    g_selectedSpriteId.clear();
}
