#pragma once
#include <entt/entt.hpp>
#include "ecs/components/WorldComponents.hpp"
#include "ecs/components/Player/PlayerStateComponent.hpp"

void RenderSystem(entt::registry &registry, float offset_x, float offset_y, float tileSize);
void RenderMechanismSystem(entt::registry &registry, int offset_x, int offset_y);
