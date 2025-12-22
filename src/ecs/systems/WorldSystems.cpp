#include "ecs/systems/WorldSystems.hpp"
#include "ecs/components/PlayerComponents.hpp"

bool IsMechanismBlockingCell(entt::registry &registry, int cellX, int cellY) {
    auto view = registry.view<const MechanismComponent>();
    for (auto entity : view) {
        const auto &mech = view.get<const MechanismComponent>(entity).mechanism;
        if (!mech.isActive()) continue;
        IVec2 target = mech.getTargetPos();
        if (target.x == cellX && target.y == cellY) return true;
    }
    return false;
}

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

void SpikeSystem(entt::registry &registry, float deltaTime) {
    auto view = registry.view<SpikeComponent, SpriteComponent, ColliderComponent>();
    if (!view) return;

    static float timer = 0.0f;
    timer += deltaTime;

    float interval = view.get<SpikeComponent>(*view.begin()).interval;
    if (timer < interval) return;

    timer = 0.0f;

    for (auto entity : view) {
        auto &spike = view.get<SpikeComponent>(entity);
        auto &sprite = view.get<SpriteComponent>(entity);
        auto &collider = view.get<ColliderComponent>(entity);

        spike.active = !spike.active;
        collider.active = spike.active;
        sprite.currentRow = spike.active ? 4 : 0;
    }
}

void MechanismSystem(entt::registry &registry, const Map &map) {
    auto playerView = registry.view<const TransformComponent, PlayerInputComponent>();
    if (!playerView) return;

    auto playerEntity = *playerView.begin();
    const auto &trans = playerView.get<const TransformComponent>(playerEntity);

    float tileSize = (float)map.tile();
    int cellX = (int)(trans.position.x / tileSize);
    int cellY = (int)(trans.position.y / tileSize);

    auto mechView = registry.view<MechanismComponent>();
    for (auto entity : mechView) {
        auto &mech = mechView.get<MechanismComponent>(entity).mechanism;
        if (!mech.isActive()) continue;
        IVec2 trigger = mech.getTriggerPos();
        if (trigger.x == cellX && trigger.y == cellY) {
            mech.deactivate();
        }
    }
}
