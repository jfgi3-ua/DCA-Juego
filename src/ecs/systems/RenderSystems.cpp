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

        //PARPADEO JUGADOR 
                if (registry.all_of<PlayerStateComponent>(entity)) {
            const auto &state = registry.get<PlayerStateComponent>(entity);
            if (state.invulnerableTimer > 0.0f &&
                state.invulnerableTimer < state.invulnerableDuration) {

                float blink = std::fmod(state.invulnerableTimer, 0.2f);
                if (blink < 0.1f) {
                    continue; // no dibujar este frame
                }
            }
        }

        
        //variables para el escalado ancho/alto al tileSize
        float frameW = 0.0;
        float frameH = 0.0f;
        Rectangle src;

        //1 SPRITE MANUAL 
        if (registry.all_of<ManualSpriteComponent>(entity)) {
            const auto &manual = registry.get<ManualSpriteComponent>(entity);

            // Sprite fijo
            if (manual.src.width > 0.0f) {
                src = manual.src;
            }
            // Sprite activo / inactivo
            else {
                bool active = true;

                if (registry.all_of<SpikeComponent>(entity)) {
                    active = registry.get<SpikeComponent>(entity).active;
                } else if (registry.all_of<MechanismComponent>(entity)) {
                    active = registry.get<MechanismComponent>(entity).mechanism.isActive();
                }

                src = active ? manual.srcActive : manual.srcInactive;
            }

            frameW = src.width;
            frameH = src.height;
        
        //2 SPRITE GRID CLIP
        } else if (registry.all_of<GridClipComponent>(entity)) {
            const auto &grid = registry.get<GridClipComponent>(entity);

            if (grid.fixedFrameSize.x > 0.0f && grid.fixedFrameSize.y > 0.0f) {
                frameW = grid.fixedFrameSize.x;
                frameH = grid.fixedFrameSize.y;
            } else {
                frameW = sprite.texture.width / (float)grid.numFrames;
                frameH = sprite.texture.height;
            }

            src = {
                frameW * grid.currentFrame,
                frameH * grid.currentRow,
                frameW,
                frameH
            };
        
        //3 SPRITE COMPLETO
        } else {
            frameW = (float)sprite.texture.width;
            frameH = (float)sprite.texture.height;
            src = {0, 0, frameW, frameH};
        }

        //escala
        float baseScale = sprite.customScale != 0.0f ? sprite.customScale : 1.5f;
        float tileScale = (tileSize / frameH) * baseScale;
        
        Rectangle destRec = {
            transform.position.x + offset_x + sprite.visualOffset.x,
            transform.position.y + offset_y + sprite.visualOffset.y,
            frameW * tileScale,
            frameH * tileScale
        };

        Vector2 origin = { destRec.width / 2.0f, destRec.height / 2.0f };

        DrawTexturePro(sprite.texture, src, destRec, origin, 0.0f, WHITE);
    }
}

void RenderMechanismSystem(entt::registry &registry, int offset_x, int offset_y) {
    auto view = registry.view<const MechanismComponent>();
    for (auto entity : view) {
        view.get<const MechanismComponent>(entity).mechanism.render(offset_x, offset_y);
    }
}
