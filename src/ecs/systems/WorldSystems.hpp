#pragma once
#include <entt/entt.hpp>
#include "objects/Map.hpp"
#include "ecs/components/World/TransformComponent.hpp"
#include "ecs/components/World/MovementComponent.hpp"
#include "ecs/components/World/SpriteComponent.hpp"
#include "ecs/components/World/GridClipComponent.hpp"
#include "ecs/components/World/AnimationComponent.hpp"
#include "ecs/components/World/SpikeComponent.hpp"
#include "ecs/components/World/MechanismComponent.hpp"
#include "ecs/components/World/ColliderComponent.hpp"
#include "ecs/components/Player/PlayerInputComponent.hpp"

bool IsMechanismBlockingCell(entt::registry &registry, int cellX, int cellY);
void MovementSystem(entt::registry &registry, float deltaTime);
void AnimationSystem(entt::registry &registry, float deltaTime);
void SpikeSystem(entt::registry &registry, float deltaTime);
void MechanismSystem(entt::registry &registry, const Map &map);
