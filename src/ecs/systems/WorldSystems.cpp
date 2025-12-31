#include "ecs/systems/WorldSystems.hpp"
#include <cmath>

void MovementSystem(entt::registry &registry, float deltaTime) {
    auto view = registry.view<TransformComponent, MovementComponent>();

    view.each([deltaTime](auto &transform, auto &move) {
        if (!move.isMoving) return;

        move.progress += deltaTime;
        float t = move.progress / move.duration;

        if (t >= 1.0f) {
            transform.position = move.targetPos;
            move.isMoving = false;
            move.progress = 0.0f;
        } else {
            transform.position.x = move.startPos.x + (move.targetPos.x - move.startPos.x) * t;
            transform.position.y = move.startPos.y + (move.targetPos.y - move.startPos.y) * t;
        }
    });
}

void AnimationSystem(entt::registry &registry, float deltaTime) {
    auto view = registry.view<SpriteComponent, GridClipComponent, MovementComponent, AnimationComponent>();
    for (auto entity : view) {
        auto &sprite = view.get<SpriteComponent>(entity);
        auto &grid = view.get<GridClipComponent>(entity);
        auto &move = view.get<MovementComponent>(entity);
        auto &anim = view.get<AnimationComponent>(entity);

        bool wantWalk = move.isMoving || move.wantsMove;
        if (wantWalk != anim.isWalking) {
            anim.isWalking = wantWalk;
            sprite.texture = anim.isWalking ? anim.walkTexture : anim.idleTexture;

            grid.numFrames = anim.isWalking ? anim.walkFrames : anim.idleFrames;
            grid.currentFrame = 0;
            grid.currentRow = 0;
            grid.timer = 0.0f;
            grid.frameTime = anim.isWalking ? anim.walkFrameTime : anim.idleFrameTime;
        }

        if (grid.numFrames <= 1) continue;
        if (grid.frameTime <= 0.0f) continue;

        grid.timer += deltaTime;
        if (grid.timer >= grid.frameTime) {
            int steps = (int)std::floor(grid.timer / grid.frameTime);
            grid.timer -= steps * grid.frameTime;
            grid.currentFrame = (grid.currentFrame + steps) % grid.numFrames;
        }
    }
}

void SpikeSystem(entt::registry &registry, float deltaTime) {
    auto view = registry.view<SpikeComponent, ColliderComponent>();
    if (!view) return;

    static float timer = 0.0f;
    timer += deltaTime;

    float interval = view.get<SpikeComponent>(*view.begin()).interval;
    if (timer < interval) return;

    timer = 0.0f;

    for (auto entity : view) {
        auto &spike = view.get<SpikeComponent>(entity);
        auto &collider = view.get<ColliderComponent>(entity);

        spike.active = !spike.active;
        collider.active = spike.active;
    }
}

void MechanismSystem(entt::registry &registry, const Map &map) {
    auto playerView = registry.view<const TransformComponent, PlayerInputComponent>();
    if(!playerView) return;

    auto playerEntity = *playerView.begin();
    const auto &pTrans = playerView.get<const TransformComponent>(playerEntity);

    float tileSize = (float)map.tile();
    int pCellX = (int)std::floor(pTrans.position.x / tileSize);
    int pCellY = (int)std::floor(pTrans.position.y / tileSize);

    // 1) detectar si el player está pisando algún TRIGGER activo
    int triggeredId = -1;

    auto triggerView = registry.view<const MechanismComponent, const MechanismTriggerComponent, const TransformComponent>();
    for (auto e : triggerView) {
        const auto &mech = triggerView.get<const MechanismComponent>(e);
        if (!mech.active) continue;

        const auto &t = triggerView.get<const TransformComponent>(e);
        int cx = (int)std::floor(t.position.x / tileSize);
        int cy = (int)std::floor(t.position.y / tileSize);

        if (cx == pCellX && cy == pCellY) {
            triggeredId = mech.id;     // <-- asumo que MechanismComponent tiene 'int id'
            break;
        }
    }

    if (triggeredId == -1) return;

    // 2) desactivar TODO lo que pertenezca a ese id (trigger + target)
    auto mechView = registry.view<MechanismComponent>();
    for (auto e : mechView) {
        auto &mech = mechView.get<MechanismComponent>(e);
        if (mech.id == triggeredId) {
            mech.active = false;
        }
    }
}


bool IsMechanismBlockingCell(entt::registry &registry, int cellX, int cellY) {
    float tileSize = TILE_SIZE;
    // solo targets bloquean (la puerta/trampa/puente), el trigger no
    auto view = registry.view<const MechanismComponent, const MechanismTargetComponent, const TransformComponent>();

    for (auto entity : view) {
        const auto &mech = view.get<const MechanismComponent>(entity);
        if (!mech.active) continue;

        const auto &tr = view.get<const TransformComponent>(entity);

        int tx = (int)std::floor(tr.position.x / tileSize);
        int ty = (int)std::floor(tr.position.y / tileSize);

        if (tx == cellX && ty == cellY) return true;
    }
    return false;
}