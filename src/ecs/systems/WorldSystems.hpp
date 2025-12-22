#pragma once
#include <entt/entt.hpp>
#include "objects/Map.hpp"
#include "ecs/components/WorldComponents.hpp"

bool IsMechanismBlockingCell(entt::registry &registry, int cellX, int cellY);
void MovementSystem(entt::registry &registry, float deltaTime);
void AnimationSystem(entt::registry &registry, float deltaTime);
void SpikeSystem(entt::registry &registry, float deltaTime);
void MechanismSystem(entt::registry &registry, const Map &map);
