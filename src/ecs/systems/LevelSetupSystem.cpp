#include "ecs/systems/LevelSetupSystem.hpp"
#include "ResourceManager.hpp"
#include "ecs/Ecs.hpp"


void LevelSetupSystem(entt::registry& registry, Map& map) {
    auto& rm = ResourceManager::Get();

    // --- MECANISMOS ---
    for (auto m : map.getMechanisms()) {
        auto entity = registry.create();
        registry.emplace<MechanismComponent>(entity, Mechanism(m.type, m.trigger, m.target));
    }

    // --- PLAYER ---
    // 1. Obtener coordenadas del grid donde está la 'P' (ej: x=2, y=3)
    IVec2 startGridPos = map.playerStart();
    float tile = (float)map.tile();

    // 2. Convertir a posición de mundo (píxeles)
    // Usamos la esquina superior izquierda del tile como referencia (más fácil para ECS)
    float centerX = startGridPos.x * tile + tile / 2.0f;
    float centerY = startGridPos.y * tile + tile / 2.0f;

    auto playerEntity = registry.create();
    
    // 3. Componente de Stats
    registry.emplace<PlayerStatsComponent>(playerEntity, 5);

    // 4. Guardamos la posición central.
    registry.emplace<TransformComponent>(
        playerEntity,
        Vector2{centerX, centerY},
        Vector2{tile, tile}
    );

    // 5. Configuración del Sprite
    Texture2D playerIdleTex = rm.GetTexture("sprites/player/Archer/Idle.png");
    Texture2D playerWalkTex = rm.GetTexture("sprites/player/Archer/Walk.png");
    
    // Ajuste manual del sprite
    Vector2 manualOffset = {0.0f, -10.0f};
   
    registry.emplace<SpriteComponent>(playerEntity, playerIdleTex, manualOffset, 1.5f);
    registry.emplace<GridClipComponent>(playerEntity, 6);
    registry.emplace<AnimationComponent>(playerEntity, playerIdleTex, playerWalkTex, 6, 8, 0.2f, 0.12f);
    
    // 6. Componente de Movimiento (Velocidad 150.0f igual que Player.hpp)
    registry.emplace<MovementComponent>(playerEntity, 75.0f);

    // 7. Etiqueta de Input (para que sepa que ESTE es el jugador controlable) <-- // ?? Esta parte tengo que estudiarmela mejor
    registry.emplace<PlayerInputComponent>(playerEntity);
    
    // 8. Estado de jugador (invulnerabilidad y retroceso)
    registry.emplace<PlayerStateComponent>(playerEntity, Vector2{centerX, centerY}, 1.5f);
    
    // 9. Cheats del jugador (god/no-clip)
    registry.emplace<PlayerCheatComponent>(playerEntity, false, false);

    // -- Colisiones --
    // 1. Añadir ColliderComponent al JUGADOR
    // Ajustamos la caja para que sea un poco más pequeña que el tile (hitbox permisiva... de momento)
    float hitSize = tile * 0.6f;
    
    // Offset centrado relativo al centro del personaje (que es donde está transform.position)
    // Como transform.position es el CENTRO, un rect en {-w/2, -h/2} estaría centrado.
    registry.emplace<ColliderComponent>(playerEntity,
        Rectangle{ -hitSize/2, -hitSize/2, hitSize, hitSize },
        CollisionType::Player
    );

    // --- ENTIDADES DEL MAPA ---
    Texture2D spikeTex = rm.GetTexture("sprites/spikes.png");
    Texture2D enemyIdleTex = rm.GetTexture("sprites/enemy/Skeleton/Idle.png");
    Texture2D enemyWalkTex = rm.GetTexture("sprites/enemy/Skeleton/Walk.png");
    Texture2D keyTex = rm.GetTexture("sprites/icons/Icons.png");

   for (int y = 0; y < map.height(); y++) {
        for (int x = 0; x < map.width(); x++) {
            char cell = map.at(x, y);

            // Calculamos posición central del tile en píxeles
            float centerX = x * map.tile() + map.tile() / 2.0f;
            float centerY = y * map.tile() + map.tile() / 2.0f;
            Vector2 pos = {centerX, centerY};
            Vector2 size = {(float)map.tile(), (float)map.tile()};
            Vector2 manualOffset = {0.0f, 0.0f};

            // --- CASO 1: PINCHOS (^) ---
            if (cell == '^') {
                auto entity = registry.create();
                registry.emplace<TransformComponent>(entity, pos, size);

                manualOffset = Vector2{0.5f, 1.0f};

                //solo textura de pinchos
                registry.emplace<SpriteComponent>(entity, spikeTex, manualOffset, 0.75f);

                // Configuración del recorte manual
                registry.emplace<ManualSpriteComponent>(
                    entity,
                    Rectangle{28, 126, 22, 22}, // ACTIVO
                    Rectangle{28,   0, 22, 22}  // INACTIVO
                );
                // Collider (ajustado)
                float hitSize = map.tile() * 0.9f;
                registry.emplace<ColliderComponent>(entity,
                    Rectangle{-hitSize/2, -hitSize/2, hitSize, hitSize},
                    CollisionType::Spike
                );

                registry.emplace<SpikeComponent>(entity, true, 3.0f);
            }

            // --- CASO 2: ENEMIGOS (E) ---
            else if (cell == 'E') {
                auto entity = registry.create();
                registry.emplace<TransformComponent>(entity, pos, size);

                // Configuración visual del enemigo (igual que el player: 6 frames, offset 8,-8)
                manualOffset = Vector2{-3.0f, -10.0f};
                registry.emplace<SpriteComponent>(entity, enemyIdleTex, manualOffset, 1.5f);
                registry.emplace<GridClipComponent>(entity, 7);
                registry.emplace<AnimationComponent>(entity, enemyIdleTex, enemyWalkTex, 7, 8, 0.2f, 0.12f);

                // Movimiento (IA)
                registry.emplace<MovementComponent>(entity, 40.0f); // Velocidad más lenta que el jugador
                registry.emplace<EnemyAIComponent>(entity);

                // Collider (90% del tile)
                float hitSize = map.tile() * 0.9f;
                registry.emplace<ColliderComponent>(entity,
                    Rectangle{-hitSize/2, -hitSize/2, hitSize, hitSize},
                    CollisionType::Enemy
                );
            }

            // --- CASO 3: LLAVES (K) ---
            if (cell == 'K') {
                auto entity = registry.create();
                registry.emplace<TransformComponent>(entity, pos, size);

                // solo textura de llave
                registry.emplace<SpriteComponent>(entity, keyTex, manualOffset, 0.75f);
               
                // Configuración del recorte manual
                registry.emplace<ManualSpriteComponent>(
                    entity,
                    Rectangle{64, 0, 16, 16}  // FIJO
                );
                // Colisión (tipo Item)
                float hitSize = map.tile() * 0.5f;
                registry.emplace<ColliderComponent>(entity,
                    Rectangle{-hitSize/2, -hitSize/2, hitSize, hitSize},
                    CollisionType::Item
                );

                // Componente lógico de Item
                registry.emplace<ItemComponent>(entity, true); // true = es llave
            }
        }
    }
}
