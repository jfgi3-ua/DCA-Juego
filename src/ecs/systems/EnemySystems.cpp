#include "ecs/systems/EnemySystems.hpp"
#include "ecs/systems/WorldSystems.hpp"
#include <algorithm>
#include <cmath>

extern "C" {
    #include <raylib.h>
}

static void ShuffledNeighbors(int out[4]) {
    out[0]=0; out[1]=1; out[2]=2; out[3]=3;
    for (int i = 3; i > 0; --i) {
        int j = GetRandomValue(0, i);
        std::swap(out[i], out[j]);
    }
}

static bool HasLineOfSight(const Map &map, int x0, int y0, int x1, int y1) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int currentX = x0;
    int currentY = y0;

    while (true) {
        if (currentX == x1 && currentY == y1) {
            return true;
        }

        if (!(currentX == x0 && currentY == y0)) {
            if (currentX < 0 || currentX >= map.width() || currentY < 0 || currentY >= map.height()) {
                return false;
            }
            char cell = map.at(currentX, currentY);
            if (cell == '#') {
                return false;
            }
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            currentX += sx;
        }
        if (e2 < dx) {
            err += dx;
            currentY += sy;
        }
    }
}

void EnemyAISystem(entt::registry &registry, const Map &map, float deltaTime) {
    auto playerView = registry.view<const TransformComponent, PlayerInputComponent>();
    if (!playerView) return;

    auto playerEntity = *playerView.begin();
    const auto &playerTrans = playerView.get<const TransformComponent>(playerEntity);

    auto view = registry.view<TransformComponent, MovementComponent, ColliderComponent, EnemyAIComponent, SpriteComponent>();

    for (auto entity : view) {
        auto &transform = view.get<TransformComponent>(entity);
        auto &move = view.get<MovementComponent>(entity);
        auto &col = view.get<ColliderComponent>(entity);
        auto &ai = view.get<EnemyAIComponent>(entity);
        auto &sprite = view.get<SpriteComponent>(entity);

        if (col.type != CollisionType::Enemy) continue;

        ai.timer += deltaTime;

        float tileSize = (float)map.tile();
        int cellX = (int)std::round((transform.position.x - tileSize / 2.0f) / tileSize);
        int cellY = (int)std::round((transform.position.y - tileSize / 2.0f) / tileSize);

        int playerCellX = (int)std::round((playerTrans.position.x - tileSize / 2.0f) / tileSize);
        int playerCellY = (int)std::round((playerTrans.position.y - tileSize / 2.0f) / tileSize);

        float dxp = transform.position.x - playerTrans.position.x;
        float dyp = transform.position.y - playerTrans.position.y;
        float distToPlayer = std::sqrt(dxp * dxp + dyp * dyp);
        float distInTiles = distToPlayer / tileSize;

        bool hasLos = HasLineOfSight(map, cellX, cellY, playerCellX, playerCellY);
        bool startedMovementThisFrame = false;


        switch (ai.state) {
            case EnemyAIState::Patrol: {
                if (distInTiles <= ai.detectionRange && hasLos) {
                    ai.state = EnemyAIState::Chase;
                    ai.timer = 0.0f;
                } else if (!move.isMoving && (ai.moveCooldown == 0.0f || ai.timer >= ai.moveCooldown)) {
                    const int dx[4] = {0, 0, 1, -1};
                    const int dy[4] = {1, -1, 0, 0};

                    int order[4];
                    ShuffledNeighbors(order);

                    bool found = false;
                    for (int k = 0; k < 4; ++k) {
                        int i = order[k];
                        int nx = cellX + dx[i];
                        int ny = cellY + dy[i];
                        if (map.isWalkableForEnemy(nx, ny) && !IsMechanismBlockingCell(registry, nx, ny)) {
                            move.startPos = transform.position;
                            move.targetPos = { nx * tileSize + tileSize / 2.0f, ny * tileSize + tileSize / 2.0f };
                            move.duration = (move.speed > 0) ? (tileSize / move.speed) : 0.12f;
                            move.progress = 0.0f;
                            move.isMoving = true;
                            ai.timer = 0.0f;
                            found = true;
                            startedMovementThisFrame = true;
                            break;
                        }
                    }
                    if (!found) {
                        move.isMoving = false;
                    }
                }
                break;
            }
            case EnemyAIState::Chase: {
                if (distInTiles > ai.detectionRange * 1.5f || !hasLos) {
                    ai.state = EnemyAIState::Patrol;
                    ai.timer = 0.0f;
                } else if (!move.isMoving && ai.timer >= 0.05f) {
                    int deltaX = playerCellX - cellX;
                    int deltaY = playerCellY - cellY;

                    int priorities[4];
                    if (std::abs(deltaX) > std::abs(deltaY)) {
                        if (deltaX > 0) {
                            priorities[0] = 2;
                            priorities[1] = (deltaY > 0) ? 0 : 1;
                            priorities[2] = (deltaY > 0) ? 1 : 0;
                            priorities[3] = 3;
                        } else {
                            priorities[0] = 3;
                            priorities[1] = (deltaY > 0) ? 0 : 1;
                            priorities[2] = (deltaY > 0) ? 1 : 0;
                            priorities[3] = 2;
                        }
                    } else {
                        if (deltaY > 0) {
                            priorities[0] = 0;
                            priorities[1] = (deltaX > 0) ? 2 : 3;
                            priorities[2] = (deltaX > 0) ? 3 : 2;
                            priorities[3] = 1;
                        } else {
                            priorities[0] = 1;
                            priorities[1] = (deltaX > 0) ? 2 : 3;
                            priorities[2] = (deltaX > 0) ? 3 : 2;
                            priorities[3] = 0;
                        }
                    }

                    const int dx[4] = {0, 0, 1, -1};
                    const int dy[4] = {1, -1, 0, 0};

                    bool found = false;
                    for (int k = 0; k < 4; ++k) {
                        int i = priorities[k];
                        int nx = cellX + dx[i];
                        int ny = cellY + dy[i];
                        if (map.isWalkableForEnemy(nx, ny) && !IsMechanismBlockingCell(registry, nx, ny)) {
                            move.startPos = transform.position;
                            move.targetPos = { nx * tileSize + tileSize / 2.0f, ny * tileSize + tileSize / 2.0f };
                            float speedMul = 1.05f;
                            move.duration = (move.speed > 0) ? (tileSize / (move.speed * speedMul)) : 0.12f;
                            move.progress = 0.0f;
                            move.isMoving = true;
                            ai.timer = 0.0f;
                            found = true;
                            startedMovementThisFrame = true;
                            break;
                        }
                    }
                    if (!found) {
                        move.isMoving = false;
                    }
                }
                break;
            }
            case EnemyAIState::Retreat: {
                ai.retreatTimer -= deltaTime;
                if (ai.retreatTimer <= 0.0f) {
                    ai.state = EnemyAIState::Patrol;
                    ai.retreatTimer = 0.0f;
                    ai.timer = 0.0f;
                } else if (!move.isMoving && ai.timer >= 0.1f) {
                    int deltaX = cellX - playerCellX;
                    int deltaY = cellY - playerCellY;

                    int priorities[4];
                    if (std::abs(deltaX) > std::abs(deltaY)) {
                        if (deltaX > 0) {
                            priorities[0] = 2;
                            priorities[1] = (deltaY > 0) ? 0 : 1;
                            priorities[2] = (deltaY > 0) ? 1 : 0;
                            priorities[3] = 3;
                        } else {
                            priorities[0] = 3;
                            priorities[1] = (deltaY > 0) ? 0 : 1;
                            priorities[2] = (deltaY > 0) ? 1 : 0;
                            priorities[3] = 2;
                        }
                    } else {
                        if (deltaY > 0) {
                            priorities[0] = 0;
                            priorities[1] = (deltaX > 0) ? 2 : 3;
                            priorities[2] = (deltaX > 0) ? 3 : 2;
                            priorities[3] = 1;
                        } else {
                            priorities[0] = 1;
                            priorities[1] = (deltaX > 0) ? 2 : 3;
                            priorities[2] = (deltaX > 0) ? 3 : 2;
                            priorities[3] = 0;
                        }
                    }

                    const int dx[4] = {0, 0, 1, -1};
                    const int dy[4] = {1, -1, 0, 0};

                    bool found = false;
                    for (int k = 0; k < 4; ++k) {
                        int i = priorities[k];
                        int nx = cellX + dx[i];
                        int ny = cellY + dy[i];
                        if (map.isWalkableForEnemy(nx, ny) && !IsMechanismBlockingCell(registry, nx, ny)) {
                            move.startPos = transform.position;
                            move.targetPos = { nx * tileSize + tileSize / 2.0f, ny * tileSize + tileSize / 2.0f };
                            float speedMul = 1.1f;
                            move.duration = (move.speed > 0) ? (tileSize / (move.speed * speedMul)) : 0.12f;
                            move.progress = 0.0f;
                            move.isMoving = true;
                            ai.timer = 0.0f;
                            found = true;
                            startedMovementThisFrame = true;
                            break;
                        }
                    }
                    if (!found) {
                        move.isMoving = false;
                    }
                }
                break;
            }
        }
        // Actualizar orientación del sprite según dirección de movimiento
        if (startedMovementThisFrame) {
            float dirX = move.targetPos.x - transform.position.x;
            if (dirX > 0.0f) {
                sprite.flipX = false; // der
            } else if (dirX < 0.0f) {
                sprite.flipX = true;  // izq
            }
        }    
    }
}
