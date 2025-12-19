#pragma once
#include "Components.hpp"
#include <entt/entt.hpp>
#include "objects/Map.hpp"
#include <cmath>
extern "C" {
    #include <raylib.h>
}

/**
 * @brief Sistema encargado de dibujar todas las entidades renderizables.
 * Itera sobre todas las entidades que tienen TransformComponent Y SpriteComponent.
 * @param registry Referencia al registro principal de ECS.
 */
inline void RenderSystem(entt::registry &registry, float offset_x, float offset_y, float tileSize) {

    auto view = registry.view<const TransformComponent, const SpriteComponent>();

    view.each([offset_x, offset_y, tileSize](const auto &transform, const auto &sprite) {

        // 1. Medidas del frame individual
        float frameWidth = (float)sprite.texture.width / sprite.numFrames;
        float frameHeight = (float)sprite.texture.height;

        // 2. FÓRMULA ORIGINAL DE PLAYER.CPP
        // "Escala para que quepa en el tile (con 1.5 para que se vea un poco más grande)"
        float scale = (tileSize * 1.5f) / frameHeight;

        // --- CORRECCIÓN FLIP ---
        float widthSrc = frameWidth;
        // Si flipX es true, width debe ser negativo en sourceRec para que Raylib invierta la textura
        if (sprite.flipX) widthSrc = -frameWidth;

        // 3. Recorte (Source)
        Rectangle sourceRec = {
            frameWidth * sprite.currentFrame,
            0.0f,
            // Si el personaje mira a la izquierda, aquí haríamos el flip negativo
            widthSrc,
            frameHeight
        };

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
    });
}

// Sistema de Entrada: Detecta teclas e inicia el movimiento
inline void InputSystem(entt::registry &registry, const Map &map) {
    auto view = registry.view<TransformComponent, MovementComponent, SpriteComponent, PlayerInputComponent>();

    view.each([&map](auto &transform, auto &move, auto &sprite) {
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
        int cellX = static_cast<int>((transform.position.x - tileSize/2) / tileSize);
        int cellY = static_cast<int>((transform.position.y - tileSize/2) / tileSize);

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