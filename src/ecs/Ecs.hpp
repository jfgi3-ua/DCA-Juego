#pragma once

#include "ecs/Ecstypes.hpp"

/**
 * Componentes 
*/

// Componentes de World

#include "ecs/components/WorldComponents.hpp"

// Componentes de Player
#include "ecs/components/Player/PlayerCheatComponent.hpp"
#include "ecs/components/Player/PlayerInputComponent.hpp"
#include "ecs/components/Player/PlayerStateComponent.hpp"
#include "ecs/components/Player/PlayerStatsComponent.hpp"

// Componentes de Enemy
#include "ecs/components/EnemyComponents.hpp"

#include "ecs/systems/PlayerSystems.hpp"
#include "ecs/systems/EnemySystems.hpp"
#include "ecs/systems/CollisionSystems.hpp"
#include "ecs/systems/RenderSystems.hpp"
#include "ecs/systems/WorldSystems.hpp"
