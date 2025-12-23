#include "ecs/systems/RenderSystems.hpp"
#include <cmath>

extern "C" {
    #include <raylib.h>
}

void RenderSystem(entt::registry &registry, float offset_x, float offset_y, float tileSize) {
    auto view = registry.view<const TransformComponent, const SpriteComponent>();

    for (auto entity : view) {
        const auto &transform = view.get<const TransformComponent>(entity);
        const auto &sprite = view.get<const SpriteComponent>(entity);

        float frameWidth, frameHeight;
        float scale;

        if (sprite.fixedFrameSize.x > 0 && sprite.fixedFrameSize.y > 0) {
            frameWidth = sprite.fixedFrameSize.x;
            frameHeight = sprite.fixedFrameSize.y;
        } else {
            frameWidth = (float)sprite.texture.width / sprite.numFrames;
            frameHeight = (float)sprite.texture.height;
        }

        if (sprite.customScale > 0.0f) {
            scale = sprite.customScale;
        } else {
            scale = (tileSize * 1.5f) / frameHeight;
        }

        float widthSrc = frameWidth;
        if (sprite.flipX) widthSrc = -frameWidth;

        Rectangle sourceRec = {
            frameWidth * sprite.currentFrame,
            frameHeight * sprite.currentRow,
            widthSrc,
            frameHeight
        };

        if (registry.all_of<SpikeComponent>(entity)) {
            const auto &spike = registry.get<SpikeComponent>(entity);
            float yOffset = spike.active ? spike.activeOffsetY : spike.inactiveOffsetY;
            Rectangle destRec = {
                transform.position.x + offset_x - tileSize / 2.0f,
                transform.position.y + offset_y - tileSize / 2.0f + yOffset,
                tileSize,
                tileSize
            };
            DrawTexturePro(sprite.texture, sourceRec, destRec, {0.0f, 0.0f}, 0.0f, WHITE);
            continue;
        }

        if (registry.all_of<PlayerStateComponent>(entity)) {
            const auto &state = registry.get<PlayerStateComponent>(entity);
            if (state.invulnerableTimer > 0.0f && state.invulnerableTimer < state.invulnerableDuration) {
                float blink = std::fmod(state.invulnerableTimer, 0.2f);
                if (blink < 0.1f) {
                    continue;
                }
            }
        }

        Rectangle destRec = {
            transform.position.x + offset_x + sprite.visualOffset.x,
            transform.position.y + offset_y + sprite.visualOffset.y,
            frameWidth * scale,
            frameHeight * scale
        };

        Vector2 origin = { destRec.width / 2.0f, destRec.height / 2.0f };
        DrawTexturePro(sprite.texture, sourceRec, destRec, origin, 0.0f, WHITE);
    }
}

void RenderMechanismSystem(entt::registry &registry, int offset_x, int offset_y) {
    auto view = registry.view<const MechanismComponent>();
    for (auto entity : view) {
        view.get<const MechanismComponent>(entity).mechanism.render(offset_x, offset_y);
    }
}
