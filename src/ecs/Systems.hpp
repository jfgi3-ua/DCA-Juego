#pragma once
#include "Components.hpp"
#include <entt/entt.hpp>
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

        // 3. Recorte (Source)
        Rectangle sourceRec = {
            frameWidth * sprite.currentFrame,
            0.0f,
            // Si el personaje mira a la izquierda, aquí haríamos el flip negativo
            frameWidth,
            frameHeight
        };

        // 4. Destino (Dest)
        // Escalamos el ancho y alto según el factor calculado
        Rectangle destRec = {
            transform.position.x + offset_x, // Posición (Centro del jugador + Offset cámara)
            transform.position.y + offset_y,
            frameWidth * scale,              // Ancho escalado
            frameHeight * scale              // Alto escalado
        };

        // 5. Origen (Punto de anclaje)
        // Definimos el centro de la textura como punto de rotación/posicionamiento
        Vector2 origin = { destRec.width / 2.0f, destRec.height / 2.0f };

        // 6. Dibujar
        DrawTexturePro(sprite.texture, sourceRec, destRec, origin, 0.0f, WHITE);
    });
}