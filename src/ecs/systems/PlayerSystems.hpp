#pragma once
#include <entt/entt.hpp>
#include "objects/Map.hpp"
#include "ecs/components/PlayerComponents.hpp"
#include "ecs/components/WorldComponents.hpp"

void InputSystem(entt::registry &registry, const Map &map);
void InvulnerabilitySystem(entt::registry &registry, float deltaTime);
