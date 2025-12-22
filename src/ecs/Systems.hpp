#pragma once
#include "Components.hpp"
#include <entt/entt.hpp>
#include "objects/Map.hpp"
#include <cmath>
#include <algorithm>
#include <iostream> // Para debug
extern "C" {
    #include <raylib.h>
}

inline bool IsMechanismBlockingCell(entt::registry &registry, int cellX, int cellY) {
    auto view = registry.view<const MechanismComponent>();
    for (auto entity : view) {
        const auto &mech = view.get<const MechanismComponent>(entity).mechanism;
        if (!mech.isActive()) continue;
        IVec2 target = mech.getTargetPos();
        if (target.x == cellX && target.y == cellY) return true;
    }
    return false;
}

inline void ShuffledNeighbors(int out[4]) {
    out[0]=0; out[1]=1; out[2]=2; out[3]=3;
    for (int i = 3; i > 0; --i) {
        int j = GetRandomValue(0, i);
        std::swap(out[i], out[j]);
    }
}

inline bool HasLineOfSight(const Map &map, int x0, int y0, int x1, int y1) {
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

/**
 * @brief Sistema encargado de dibujar todas las entidades renderizables.
 * Itera sobre todas las entidades que tienen TransformComponent Y SpriteComponent.
 * @param registry Referencia al registro principal de ECS.
 */
inline void RenderSystem(entt::registry &registry, float offset_x, float offset_y, float tileSize) {

    auto view = registry.view<const TransformComponent, const SpriteComponent>();

    for (auto entity : view) {
        const auto &transform = view.get<const TransformComponent>(entity);
        const auto &sprite = view.get<const SpriteComponent>(entity);

        // 1. Medidas del frame individual y escala
        float frameWidth, frameHeight;
        float scale;

        // Si hay un tamaño fijo definido (ej: iconos 16x16), será el que usemos.
        if (sprite.fixedFrameSize.x > 0 && sprite.fixedFrameSize.y > 0) {
            frameWidth = sprite.fixedFrameSize.x;
            frameHeight = sprite.fixedFrameSize.y;
        } else {
            // Lógica por defecto (Tira de animación simple)
            frameWidth = (float)sprite.texture.width / sprite.numFrames;
            frameHeight = (float)sprite.texture.height;
        }

        // 2. Cálculo de escala
        if (sprite.customScale > 0.0f) {
            // Si hay escala manual, la usamos directamente
            scale = sprite.customScale;
        } else {
            // Si es 0.0, usamos la fórmula original del JUGADOR
            scale = (tileSize * 1.5f) / frameHeight; // Escala para que quepa en el tile (con 1.5 para que se vea un poco más grande)
        }

        // --- CORRECCIÓN FLIP (para que se invierta la textura correctamente) ---
        float widthSrc = frameWidth;
        // Si flipX es true, width debe ser negativo en sourceRec para que Raylib invierta la textura
        if (sprite.flipX) widthSrc = -frameWidth;

        // 3. Recorte (Source)
        Rectangle sourceRec = {
            frameWidth * sprite.currentFrame,
            frameHeight * sprite.currentRow,
            widthSrc, // Si el personaje mira a la izquierda, aquí haríamos el flip negativo
            frameHeight
        };

        // Render especial para pinchos: anclado al tile (top-left) como en legacy
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

        // 4. Destino (Dest)
        // Escalamos el ancho y alto según el factor calculado
        Rectangle destRec = {
            transform.position.x + offset_x + sprite.visualOffset.x, // Posición (Centro del jugador + Offset de renderizado)
            transform.position.y + offset_y + sprite.visualOffset.y,
            frameWidth * scale,     // Ancho escalado
            frameHeight * scale     // Alto escalado
        };

        // 5. Origen (Punto de anclaje)
        // Definimos el centro de la textura como punto de rotación/posicionamiento
        Vector2 origin = { destRec.width / 2.0f, destRec.height / 2.0f };

        // 6. Dibujar
        DrawTexturePro(sprite.texture, sourceRec, destRec, origin, 0.0f, WHITE);
    }
}

inline void RenderMechanismSystem(entt::registry &registry, int offset_x, int offset_y) {
    auto view = registry.view<const MechanismComponent>();
    for (auto entity : view) {
        view.get<const MechanismComponent>(entity).mechanism.render(offset_x, offset_y);
    }
}

inline void SpikeSystem(entt::registry &registry, float deltaTime) {
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

        // Ajuste visual según estado (fila del spritesheet)
        sprite.currentRow = spike.active ? 4 : 0;
    }
}

inline void MechanismSystem(entt::registry &registry, const Map &map) {
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

// Sistema de Entrada: Detecta teclas e inicia el movimiento
inline void InputSystem(entt::registry &registry, const Map &map) {
    auto view = registry.view<TransformComponent, MovementComponent, SpriteComponent, PlayerInputComponent>();

    view.each([&map, &registry](auto &transform, auto &move, auto &sprite) {
        // Si ya se está moviendo, no aceptamos nuevo input
        if (move.isMoving) return;

        int dx = 0;
        int dy = 0;

        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    dy = -1;
        else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) dy = 1;
        else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) dx = -1;
        else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dx = 1;

        // Si no hay input, salir
        if (dx == 0 && dy == 0) return;

        // --- Lógica de Dirección Visual (Flip) ---
        if (dx > 0) sprite.flipX = false;      // Derecha
        else if (dx < 0) sprite.flipX = true;  // Izquierda

        // --- Lógica de Coordenadas ---
        float tileSize = (float)map.tile();

        // Calcular casilla actual (basado en la posición actual)
        // Nota: Asumimos que transform.position está en el CENTRO del tile (AQUI HAY QUE TENER CUIDADO)
        int cellX = (int)std::round((transform.position.x - tileSize / 2.0f) / tileSize);
        int cellY = (int)std::round((transform.position.y - tileSize / 2.0f) / tileSize);

        if (!move.isMoving) {
            float snapX = cellX * tileSize + tileSize / 2.0f;
            float snapY = cellY * tileSize + tileSize / 2.0f;
            transform.position = {snapX, snapY};
        }

        int targetX = cellX + dx;
        int targetY = cellY + dy;

        // 1. Verificar límites del mapa
        if (targetX < 0 || targetX >= map.width() || targetY < 0 || targetY >= map.height()) {
            return;
        }

        // 2. Verificar paredes (Map::at lanza excepción si fuera de rango, pero ya comprobamos límites)
        if (map.at(targetX, targetY) == '#') {
            return; // Bloqueado
        }

        // 3. Verificar bloqueo por mecanismo activo
        if (IsMechanismBlockingCell(registry, targetX, targetY)) {
            return;
        }

        // --- Iniciar Transición ---
        move.startPos = transform.position;

        // El destino es el CENTRO del tile objetivo
        move.targetPos = {
            targetX * tileSize + tileSize / 2.0f,
            targetY * tileSize + tileSize / 2.0f
        };

        // Duración basada en velocidad (tiempo = espacio / velocidad)
        // Usamos tileSize como distancia aproximada
        move.duration = (move.speed > 0) ? (tileSize / move.speed) : 0.12f;

        move.progress = 0.0f;
        move.isMoving = true;
    });
}

// Sistema de Movimiento: Interpola la posición
inline void MovementSystem(entt::registry &registry, float deltaTime) {
    auto view = registry.view<TransformComponent, MovementComponent>();

    view.each([deltaTime](auto &transform, auto &move) {
        if (!move.isMoving) return;

        move.progress += deltaTime;

        // Calcular t (0.0 a 1.0)
        float t = move.progress / move.duration;

        if (t >= 1.0f) {
            // Llegamos al destino
            transform.position = move.targetPos;
            move.isMoving = false;
            move.progress = 0.0f;
        } else {
            // Interpolación Lineal (Lerp)
            transform.position.x = move.startPos.x + (move.targetPos.x - move.startPos.x) * t;
            transform.position.y = move.startPos.y + (move.targetPos.y - move.startPos.y) * t;
        }
    });
}

inline void CollisionSystem(entt::registry &registry, Map &map) {
    // 1. Obtener al jugador (asumimos que solo hay uno con PlayerInputComponent y Collider)
    auto playerView = registry.view<TransformComponent, ColliderComponent, PlayerInputComponent>();

    // Si no hay jugador, no hacemos nada
    if (!playerView) return;

    entt::entity playerEntity = *playerView.begin();
    auto &playerTrans = playerView.get<TransformComponent>(playerEntity);
    auto &playerCol = playerView.get<ColliderComponent>(playerEntity);

    // Calcular la caja absoluta del jugador en el mundo
    // Sumamos la posición de la entidad al offset del collider
    Rectangle playerBox = {
        playerTrans.position.x + playerCol.rect.x,
        playerTrans.position.y + playerCol.rect.y,
        playerCol.rect.width,
        playerCol.rect.height
    };

    // 2. Iterar sobre todas las demas entidades con colisionador (Enemigos, Pinchos, etc.)
    // Excluimos al jugador usando 'entt::exclude<PlayerInputComponent>' si quisiéramos,
    // pero aquí simplemente chequeamos colisiones contra "todo lo que sea peligroso".
    auto hazardView = registry.view<TransformComponent, ColliderComponent>();

    hazardView.each([&](auto entity, auto &hazardTrans, auto &hazardCol) {
        // No chequear colisión con uno mismo
        if(entity == playerEntity) return;

        if (!hazardCol.active) return;

        // Calcular caja absoluta del peligro
        Rectangle hazardBox = {
            hazardTrans.position.x + hazardCol.rect.x,
            hazardTrans.position.y + hazardCol.rect.y,
            hazardCol.rect.width,
            hazardCol.rect.height
        };

        // 3. Verificar Solapamiento (AABB)
        if (CheckCollisionRecs(playerBox, hazardBox)) {

            // CASO A: DAÑO (Pincho / Enemigo)
            if (hazardCol.type == CollisionType::Spike || hazardCol.type == CollisionType::Enemy) {
                if (hazardCol.type == CollisionType::Spike && registry.all_of<SpikeComponent>(entity)) {
                    const auto &spike = registry.get<SpikeComponent>(entity);
                    if (!spike.active) return;
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
                std::cout << "¡COLISIÓN DETECTADA! Reiniciando jugador..." << std::endl;

                // Lógica de Respawn simple: Volver al inicio del mapa
                IVec2 startGrid = map.playerStart();
                float tileSize = (float)map.tile();

                // Reiniciar posición física
                playerTrans.position = {
                    startGrid.x * tileSize + tileSize / 2.0f,
                    startGrid.y * tileSize + tileSize / 2.0f
                };

                // IMPORTANTE: Reiniciar lógica de movimiento para evitar que siga interpolando
                if (registry.all_of<MovementComponent>(playerEntity)) {
                    auto &move = registry.get<MovementComponent>(playerEntity);
                    move.isMoving = false;
                    move.progress = 0.0f;
                    move.targetPos = playerTrans.position; // Cancelar destino
                }
                // AQUÍ deberíamos restar vidas del StatsComponent del jugador
                if (registry.all_of<StatsComponent>(playerEntity)) {
                    auto &stats = registry.get<StatsComponent>(playerEntity);
                    stats.lives--;
                    std::cout << "Vidas restantes: " << stats.lives << std::endl;
                    if (stats.lives <= 0) {
                        // TODO: Llevar al Game Over State
                        std::cout << "GAME OVER" << std::endl;
                    }
                }
            }
            // CASO B: ITEMS (Llaves)
            else if (hazardCol.type == CollisionType::Item) {
                // Verificar si es un item válido
                if (registry.all_of<ItemComponent>(entity)) {
                    auto &item = registry.get<ItemComponent>(entity);

                    if (!item.collected) {
                        item.collected = true; // Marcar para borrar

                        // Sumar al jugador
                        if (registry.all_of<StatsComponent>(playerEntity)) {
                            auto &stats = registry.get<StatsComponent>(playerEntity);
                            if (item.isKey) {
                                stats.keysCollected++;
                                std::cout << "Llave recogida! Total: " << stats.keysCollected << std::endl;
                            }
                        }

                        // Reflejar la recogida en el mapa (grid + vector de llaves)
                        float tileSize = (float)map.tile();
                        int cellX = (int)(hazardTrans.position.x / tileSize);
                        int cellY = (int)(hazardTrans.position.y / tileSize);
                        map.clearCell(cellX, cellY);

                        // Destruir la entidad del item inmediatamente
                        registry.destroy(entity);
                    }
                }
            }
        }
    });
}

inline void EnemyAISystem(entt::registry &registry, const Map &map, float deltaTime) {
    auto playerView = registry.view<const TransformComponent, PlayerInputComponent>();
    if (!playerView) return;

    auto playerEntity = *playerView.begin();
    const auto &playerTrans = playerView.get<const TransformComponent>(playerEntity);

    auto view = registry.view<TransformComponent, MovementComponent, ColliderComponent, EnemyAIComponent>();

    for (auto entity : view) {
        auto &transform = view.get<TransformComponent>(entity);
        auto &move = view.get<MovementComponent>(entity);
        auto &col = view.get<ColliderComponent>(entity);
        auto &ai = view.get<EnemyAIComponent>(entity);

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
                            float speedMul = 1.1f;
                            move.duration = (move.speed > 0) ? (tileSize / (move.speed * speedMul)) : 0.12f;
                            move.progress = 0.0f;
                            move.isMoving = true;
                            ai.timer = 0.0f;
                            found = true;
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
                            float speedMul = 1.2f;
                            move.duration = (move.speed > 0) ? (tileSize / (move.speed * speedMul)) : 0.12f;
                            move.progress = 0.0f;
                            move.isMoving = true;
                            ai.timer = 0.0f;
                            found = true;
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
    }
}
