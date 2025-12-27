#pragma once
#include <entt/entt.hpp>
#include "objects/Map.hpp"
#include "ecs/components/Player/PlayerCheatComponent.hpp"
#include "ecs/components/Player/PlayerInputComponent.hpp"
#include "ecs/components/Player/PlayerStateComponent.hpp"
#include "ecs/components/Player/PlayerStatsComponent.hpp"
#include "ecs/components/World/ColliderComponent.hpp"
#include "ecs/components/World/TransformComponent.hpp"
#include "ecs/components/World/ItemComponent.hpp"
#include "ecs/components/World/SpikeComponent.hpp"
#include "ecs/components/World/MovementComponent.hpp"
#include "ecs/components/EnemyComponents.hpp"

void CollisionSystem(entt::registry &registry, Map &map);
