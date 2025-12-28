#pragma once
#include <entt/entt.hpp>
#include "objects/Map.hpp"
#include "ecs/components/Enemy/EnemyIAComponent.hpp"
#include "ecs/components/Player/PlayerStateComponent.hpp"
#include "ecs/components/Player/PlayerInputComponent.hpp"

void EnemyAISystem(entt::registry &registry, const Map &map, float deltaTime);
