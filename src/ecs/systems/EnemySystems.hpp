#pragma once
#include <entt/entt.hpp>
#include "objects/Map.hpp"
#include "ecs/components/EnemyComponents.hpp"
#include "ecs/components/WorldComponents.hpp"
#include "ecs/components/PlayerComponents.hpp"

void EnemyAISystem(entt::registry &registry, const Map &map, float deltaTime);
