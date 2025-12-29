#pragma once
#include <entt/entt.hpp>
#include "ecs/components/World/TransformComponent.hpp"
#include "ecs/components/World/SpriteComponent.hpp"
#include "ecs/components/World/ManualSpriteComponent.hpp"
#include "ecs/components/World/GridClipComponent.hpp"
#include "ecs/components/World/SpikeComponent.hpp"
#include "ecs/components/World/MechanismComponent.hpp"
#include "ecs/components/Player/PlayerStateComponent.hpp"

void RenderSystem(entt::registry &registry, float offset_x, float offset_y, float tileSize);
void RenderMechanismSystem(entt::registry &registry, int offset_x, int offset_y);
