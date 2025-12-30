#pragma once
#include <entt/entt.hpp>
#include "objects/Map.hpp"

void LevelSetupSystem(entt::registry& registry, Map& map);

inline const char* MechanismTypeToString(MechanismType type) {
    switch (type) {
        case MechanismType::DOOR:   return "DOOR";
        case MechanismType::LEVER:  return "LEVER";
        case MechanismType::TRAP:   return "TRAP";
        case MechanismType::BRIDGE: return "BRIDGE";
        default:                    return "UNKNOWN";
    }
}

static void createMechanism_( entt::registry& registry, const MechanismPair& m, float tile, int mechId);

static int ComputeFramesForTexture(const Texture2D& tex);
