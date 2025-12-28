#pragma once

#include "ecs/Ecstypes.hpp"

/**
 * Componentes 
*/

// Componentes de World
#include "ecs/components/World/AnimationComponent.hpp"
#include "ecs/components/World/ColliderComponent.hpp"
#include "ecs/components/World/GridClipComponent.hpp"
#include "ecs/components/World/ItemComponent.hpp"
#include "ecs/components/World/ManualSpriteComponent.hpp"
#include "ecs/components/World/MechanismComponent.hpp"
#include "ecs/components/World/MovementComponent.hpp"
#include "ecs/components/World/SpikeComponent.hpp"
#include "ecs/components/World/SpriteComponent.hpp"
#include "ecs/components/World/TransformComponent.hpp"

// Componentes de Player
#include "ecs/components/Player/PlayerCheatComponent.hpp"
#include "ecs/components/Player/PlayerInputComponent.hpp"
#include "ecs/components/Player/PlayerStateComponent.hpp"
#include "ecs/components/Player/PlayerStatsComponent.hpp"

// Componentes de Enemy
#include "ecs/components/Enemy/EnemyIAComponent.hpp"

#include "ecs/systems/PlayerSystems.hpp"
#include "ecs/systems/EnemySystems.hpp"
#include "ecs/systems/CollisionSystems.hpp"
#include "ecs/systems/RenderSystems.hpp"
#include "ecs/systems/WorldSystems.hpp"
