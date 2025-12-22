#include "ecs/systems/PlayerSystems.hpp"
#include "ecs/systems/WorldSystems.hpp"

extern "C" {
    #include <raylib.h>
}

void InputSystem(entt::registry &registry, const Map &map) {
    auto view = registry.view<TransformComponent, MovementComponent, SpriteComponent, PlayerInputComponent, PlayerStateComponent, PlayerCheatComponent>();

    for (auto entity : view) {
        auto &transform = view.get<TransformComponent>(entity);
        auto &move = view.get<MovementComponent>(entity);
        auto &sprite = view.get<SpriteComponent>(entity);
        auto &playerState = view.get<PlayerStateComponent>(entity);
        auto &cheats = view.get<PlayerCheatComponent>(entity);

        if (move.isMoving) continue;

        int dx = 0;
        int dy = 0;

        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    dy = -1;
        else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) dy = 1;
        else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) dx = -1;
        else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dx = 1;

        if (dx == 0 && dy == 0) continue;

        if (dx > 0) sprite.flipX = false;
        else if (dx < 0) sprite.flipX = true;

        float tileSize = (float)map.tile();

        int cellX = (int)std::round((transform.position.x - tileSize / 2.0f) / tileSize);
        int cellY = (int)std::round((transform.position.y - tileSize / 2.0f) / tileSize);

        if (!move.isMoving) {
            float snapX = cellX * tileSize + tileSize / 2.0f;
            float snapY = cellY * tileSize + tileSize / 2.0f;
            transform.position = {snapX, snapY};
        }

        int targetX = cellX + dx;
        int targetY = cellY + dy;

        if (targetX < 0 || targetX >= map.width() || targetY < 0 || targetY >= map.height()) {
            continue;
        }

        if (!cheats.noClip && map.at(targetX, targetY) == '#') {
            continue;
        }

        if (!cheats.noClip && IsMechanismBlockingCell(registry, targetX, targetY)) {
            continue;
        }

        playerState.lastTilePos = transform.position;

        move.startPos = transform.position;
        move.targetPos = {
            targetX * tileSize + tileSize / 2.0f,
            targetY * tileSize + tileSize / 2.0f
        };
        move.duration = (move.speed > 0) ? (tileSize / move.speed) : 0.12f;
        move.progress = 0.0f;
        move.isMoving = true;
    }
}

void InvulnerabilitySystem(entt::registry &registry, float deltaTime) {
    auto view = registry.view<PlayerStateComponent>();
    for (auto entity : view) {
        auto &state = view.get<PlayerStateComponent>(entity);
        if (state.invulnerableTimer > 0.0f && state.invulnerableTimer < state.invulnerableDuration) {
            state.invulnerableTimer += deltaTime;
            if (state.invulnerableTimer > state.invulnerableDuration) {
                state.invulnerableTimer = state.invulnerableDuration;
            }
        }
    }
}
