#pragma once
#include <entt/entt.hpp>
#include "objects/Map.hpp"
#include "ecs/components/WorldComponents.hpp"
#include "ecs/components/Player/PlayerCheatComponent.hpp"
#include "ecs/components/Player/PlayerInputComponent.hpp"
#include "ecs/components/Player/PlayerStateComponent.hpp"
#include "ecs/components/Player/PlayerStatsComponent.hpp"
#include "ecs/components/EnemyComponents.hpp"

void CollisionSystem(entt::registry &registry, Map &map);
