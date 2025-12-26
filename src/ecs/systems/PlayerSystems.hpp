#pragma once
#include <entt/entt.hpp>
#include "objects/Map.hpp"
#include "ecs/components/Player/PlayerStateComponent.hpp"
#include "ecs/components/Player/PlayerInputComponent.hpp"
#include "ecs/components/Player/PlayerCheatComponent.hpp"
#include "ecs/components/WorldComponents.hpp"

void InputSystem(entt::registry &registry, const Map &map);
void InvulnerabilitySystem(entt::registry &registry, float deltaTime);
