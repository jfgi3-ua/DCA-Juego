#pragma once
#include <entt/entt.hpp>
#include "objects/Map.hpp"
#include "ecs/components/WorldComponents.hpp"
#include "ecs/components/PlayerComponents.hpp"
#include "ecs/components/EnemyComponents.hpp"

void CollisionSystem(entt::registry &registry, Map &map);
