#include "ecs/systems/CollisionSystems.hpp"

extern "C" {
    #include <raylib.h>
}

void CollisionSystem(entt::registry &registry, Map &map) {
    auto playerView = registry.view<TransformComponent, ColliderComponent, PlayerInputComponent, PlayerStateComponent, PlayerCheatComponent>();

    if (!playerView) return;

    entt::entity playerEntity = *playerView.begin();
    auto &playerTrans = playerView.get<TransformComponent>(playerEntity);
    auto &playerCol = playerView.get<ColliderComponent>(playerEntity);
    auto &playerState = playerView.get<PlayerStateComponent>(playerEntity);
    auto &cheats = playerView.get<PlayerCheatComponent>(playerEntity);

    Rectangle playerBox = {
        playerTrans.position.x + playerCol.rect.x,
        playerTrans.position.y + playerCol.rect.y,
        playerCol.rect.width,
        playerCol.rect.height
    };

    auto hazardView = registry.view<TransformComponent, ColliderComponent>();

    hazardView.each([&](auto entity, auto &hazardTrans, auto &hazardCol) {
        if(entity == playerEntity) return;
        if (!hazardCol.active) return;

        Rectangle hazardBox = {
            hazardTrans.position.x + hazardCol.rect.x,
            hazardTrans.position.y + hazardCol.rect.y,
            hazardCol.rect.width,
            hazardCol.rect.height
        };

        if (CheckCollisionRecs(playerBox, hazardBox)) {
            if (hazardCol.type == CollisionType::Spike || hazardCol.type == CollisionType::Enemy) {
                if (hazardCol.type == CollisionType::Spike && registry.all_of<SpikeComponent>(entity)) {
                    const auto &spike = registry.get<SpikeComponent>(entity);
                    if (!spike.active) return;
                }
                if (cheats.godMode) return;
                if (playerState.invulnerableTimer > 0.0f && playerState.invulnerableTimer < playerState.invulnerableDuration) {
                    return;
                }
                if (hazardCol.type == CollisionType::Enemy && registry.all_of<EnemyAIComponent>(entity)) {
                    auto &ai = registry.get<EnemyAIComponent>(entity);
                    if (ai.state == EnemyAIState::Chase) {
                        ai.state = EnemyAIState::Retreat;
                        ai.retreatTimer = ai.retreatDuration;
                        if (registry.all_of<MovementComponent>(entity)) {
                            auto &move = registry.get<MovementComponent>(entity);
                            move.isMoving = false;
                            move.progress = 0.0f;
                        }
                    }
                }

                float tileSize = (float)map.tile();
                int prevCellX = (int)(playerState.lastTilePos.x / tileSize);
                int prevCellY = (int)(playerState.lastTilePos.y / tileSize);

                bool validRespawn = true;
                if (prevCellX < 0 || prevCellX >= map.width() || prevCellY < 0 || prevCellY >= map.height()) {
                    validRespawn = false;
                } else if (map.at(prevCellX, prevCellY) == '#') {
                    validRespawn = false;
                }

                if (!validRespawn) {
                    IVec2 startGrid = map.playerStart();
                    playerTrans.position = {
                        startGrid.x * tileSize + tileSize / 2.0f,
                        startGrid.y * tileSize + tileSize / 2.0f
                    };
                } else {
                    playerTrans.position = {
                        prevCellX * tileSize + tileSize / 2.0f,
                        prevCellY * tileSize + tileSize / 2.0f
                    };
                }

                if (registry.all_of<MovementComponent>(playerEntity)) {
                    auto &move = registry.get<MovementComponent>(playerEntity);
                    move.isMoving = false;
                    move.progress = 0.0f;
                    move.targetPos = playerTrans.position;
                }

                if (registry.all_of<PlayerStatsComponent>(playerEntity)) {
                    auto &stats = registry.get<PlayerStatsComponent>(playerEntity);
                    stats.lives--;
                }

                playerState.invulnerableTimer = 0.0001f;
            } else if (hazardCol.type == CollisionType::Item) {
                if (registry.all_of<ItemComponent>(entity)) {
                    auto &item = registry.get<ItemComponent>(entity);
                    if (!item.collected) {
                        item.collected = true;
                        if (registry.all_of<PlayerStatsComponent>(playerEntity)) {
                            auto &stats = registry.get<PlayerStatsComponent>(playerEntity);
                            if (item.isKey) {
                                stats.keysCollected++;
                            }
                        }

                        float tileSize = (float)map.tile();
                        int cellX = (int)(hazardTrans.position.x / tileSize);
                        int cellY = (int)(hazardTrans.position.y / tileSize);
                        map.clearCell(cellX, cellY);

                        registry.destroy(entity);
                    }
                }
            }
        }
    });
}
