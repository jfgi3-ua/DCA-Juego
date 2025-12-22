#include <iostream>
#include "MainGameState.hpp"
#include "GameOverState.hpp"
#include "DevModeState.hpp"
#include "StateMachine.hpp"
#include "ResourceManager.hpp"
#include "objects/Enemy.hpp"
#include "ecs/Components.hpp"
#include "ecs/Systems.hpp"
extern "C" {
  #include <raylib.h>
}

static std::vector<Enemy> enemies;

MainGameState::MainGameState(int level)
{
    level_ = level > 0 ? level : 1;
}

void MainGameState::init()
{
    auto& rm = ResourceManager::Get();
    std::string relativePath = "maps/map_" + std::to_string(level_) + ".txt";
    std::string absolutePath = rm.GetAssetPath(relativePath);

    //std::string absolutePath = "/usr/share/game/assets/maps/map_1.txt"; //probando poque da error al intentar abrilo en carpeta random
    map_.loadFromFile(absolutePath, TILE_SIZE);
    map_.loadTextures(); //lo llamamos aqui ya q tambien se llama en main y no se pueden cargar texturas antes de InitWindow
    tile_ = map_.tile();

    // Guardar total de llaves del mapa (antes de que se recojan)
    totalKeysInMap_ = map_.getTotalKeys();

    IVec2 p = map_.playerStart();
    Vector2 startPos = { p.x * (float)tile_ + tile_ / 2.0f,
                         p.y * (float)tile_ + tile_ / 2.0f };
    // player_.init(startPos, tile_ * 0.35f, 5, "sprites/player/Musketeer"); //inicializamos el jugador con la carpeta de sprites // !! --- CÓDIGO ANTIGUO  ---

    // !! --- CÓDIGO ANTIGUO  ---
    // enemiesPos_.clear();
    // enemies.clear();
    // for (auto e : map_.enemyStarts()) {
    //     enemiesPos_.push_back({ e.x * (float)tile_ + tile_ / 2.0f,
    //                             e.y * (float)tile_ + tile_ / 2.0f }); // !! --- CÓDIGO ANTIGUO  ---
    //     // crear un único Enemy usando el constructor que necesita tile_
    //     enemies.emplace_back(e.x, e.y, tile_);
    // }

    // for (auto s : map_.spikesStarts()) {
    //     spikes_.addSpike(s.x, s.y);
    // }

    // Cargar mecanismos desde el mapa (ECS)
    for (auto m : map_.getMechanisms()) {
        auto entity = registry.create();
        registry.emplace<MechanismComponent>(entity, Mechanism(m.id, m.trigger, m.target));
    }

    // Inicializar temporizador: 30s base + 30s por cada nivel adicional
    levelTime_ = 30.0f + (level_ - 1) * 30.0f;

    // ---------------------------------------------------------
    // REFACTOR: ENTT
    // ---------------------------------------------------------
    // --- PLAYER ---
    // 1. Obtener coordenadas del grid donde está la 'P' (ej: x=2, y=3)
    IVec2 startGridPos = map_.playerStart();

    // 2. Convertir a posición de mundo (píxeles)
    // Usamos la esquina superior izquierda del tile como referencia (más fácil para ECS)
    float centerX = (startGridPos.x * map_.tile()) + (map_.tile() / 2.0f);
    float centerY = (startGridPos.y * map_.tile()) + (map_.tile() / 2.0f);
    auto playerEntity = registry.create();

    // 3. Componente de Stats
    registry.emplace<StatsComponent>(playerEntity, 5); // 5 vidas iniciales

    // 4. Guardamos la posición central.
    registry.emplace<TransformComponent>(playerEntity, Vector2{centerX, centerY}, Vector2{(float)map_.tile(), (float)map_.tile()});

    // 5. Configuración del Sprite
    Texture2D playerTex = LoadTexture("assets/sprites/player/Archer/Idle.png");
    Vector2 manualOffset = { 0.0f, -10.0f };  // Ajuste manual del sprite
    registry.emplace<SpriteComponent>(playerEntity, playerTex, 6, manualOffset);

    // 6. Componente de Movimiento (Velocidad 150.0f igual que Player.hpp)
    registry.emplace<MovementComponent>(playerEntity, 150.0f);

    // 7. Etiqueta de Input (para que sepa que ESTE es el jugador controlable) <-- // ?? Esta parte tengo que estudiarmela mejor
    registry.emplace<PlayerInputComponent>(playerEntity);

    // -- Colisiones --
    // 1. Añadir ColliderComponent al JUGADOR
    // Ajustamos la caja para que sea un poco más pequeña que el tile (hitbox permisiva... de momento)
    float hitSize = tile_ * 0.6f;
    float offset = (tile_ - hitSize) / 2.0f;
    // Offset centrado relativo al centro del personaje (que es donde está transform.position)
    // Como transform.position es el CENTRO, un rect en {-w/2, -h/2} estaría centrado.
    registry.emplace<ColliderComponent>(playerEntity,
        Rectangle{ -hitSize/2, -hitSize/2, hitSize, hitSize },
        CollisionType::Player
    );

    // 2. Crear ENEMIGOS y PINCHOS
    loadLevelEntities();

    std::cout << "Nivel cargado. Entidades generadas via ECS." << std::endl;
}

// !! --- CÓDIGO ANTIGUO  ---
void MainGameState::handleInput()
{
    // Activar menú de desarrollador con CTRL+D
    // if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D)) {
    //     this->state_machine->add_overlay_state(
    //         std::make_unique<DevModeState>(&player_, &levelTime_, &freezeEnemies_, &infiniteTime_, &keyGivenByCheating_, level_)
    //     );
    //     return;
    // }
}

void MainGameState::update(float deltaTime)
{
    // Reducir temporizador de nivel (excepto si está en modo tiempo infinito)
    if (!infiniteTime_) {
        levelTime_ -= deltaTime;
        if (levelTime_ <= 0.0f) {
            levelTime_ = 0.0f;
            // Tiempo agotado -> Game Over (dead = true)
            this->state_machine->add_state(std::make_unique<GameOverState>(level_, true, 0.0f, false), true);
            return;
        }
    }

    // 1) Actualizar (movimiento) jugador con colisiones de mapa
    // player_.update(deltaTime, map_, activeMechanisms_); // LOGICA ANTIGUA

    // LOGICA NUEVA CON ECS
    // Primero Input (decide destino), luego Movimiento (mueve)
    InputSystem(registry, map_);
    EnemyAISystem(registry, map_);
    MovementSystem(registry, deltaTime);
    SpikeSystem(registry, deltaTime);
    CollisionSystem(registry, map_); // Chequeo de colisiones
    MechanismSystem(registry, map_);

    // --- FLUJO DE JUEGO ECS: derrota/victoria ---
    auto playerView = registry.view<TransformComponent, StatsComponent, PlayerInputComponent>();
    if (playerView) {
        auto playerEntity = *playerView.begin();
        const auto &stats = playerView.get<StatsComponent>(playerEntity);
        const auto &trans = playerView.get<TransformComponent>(playerEntity);

        // Derrota por vidas
        if (stats.lives <= 0) {
            this->state_machine->add_state(std::make_unique<GameOverState>(level_, true, levelTime_, false), true);
            return;
        }

        // Victoria por salida + llaves
        int cellX = (int)(trans.position.x / tile_);
        int cellY = (int)(trans.position.y / tile_);
        if (cellX >= 0 && cellY >= 0 && cellX < map_.width() && cellY < map_.height()) {
            if (map_.at(cellX, cellY) == 'X' && stats.keysCollected >= totalKeysInMap_) {
                if (level_ >= 6) {
                    this->state_machine->add_state(std::make_unique<GameOverState>(level_, false, levelTime_, true), true);
                } else {
                    this->state_machine->add_state(std::make_unique<GameOverState>(level_, false, levelTime_, false), true);
                }
                return;
            }
        }
    }

    // 2) Celda actual del jugador
    // !! --- CÓDIGO ANTIGUO  ---
    // int cellX = (int)(player_.getPosition().x) / tile_;
    // int cellY = (int)(player_.getPosition().y) / tile_;

    // 3) ¿Está sobre una llave 'K'? -> Recogerla
    // !! --- CÓDIGO ANTIGUO  ---
    // try {
    //     if (map_.at(cellX, cellY) == 'K') {
    //         player_.addKey();
    //         map_.clearCell(cellX, cellY); // Reemplaza 'K' por '.', es decir, retira la llave del mapa.
    //         std::cout << "Llave recogida! (" << player_.getKeyCount() << "/" << totalKeysInMap_ << ")" << std::endl;
    //         // TODO:Aquí podríamos reproducir un sonido o mostrar un mensaje en pantalla si se desea.
    //     }
    // } catch (const std::out_of_range& e) {
    //     // Ignorar fuera de rango (no debería ocurrir aquí). Fuera de rango no debería ocurrir si los cálculos de celda son correctos.
    //     std::cerr << "Error: " << e.what() << std::endl;
    // }

    // 4) ¿Está sobre la salida 'X' y tiene todas las llaves? -> Nivel completado
    // !! --- CÓDIGO ANTIGUO  ---
    // if (player_.isOnExit(map_) && player_.hasAllKeys(totalKeysInMap_)) {
    //     std::cout << "Nivel completado" << std::endl;
    //     // Si es el nivel 6 (último), mostrar pantalla de victoria directamente
    //     if (level_ >= 6) {
    //         this->state_machine->add_state(std::make_unique<GameOverState>(level_, false, levelTime_, true), true);
    //     } else {
    //         // Pasar a pantalla de nivel completado (dead = false)
    //         this->state_machine->add_state(std::make_unique<GameOverState>(level_, false, levelTime_, false), true);
    //     }
    //     return;
    // }

    // 5) IA enemigos y colisiones con jugador
    // Pasar la posición del jugador a los enemigos para el árbol de decisiones
    // Vector2 playerPos = player_.getPosition(); // !! --- CÓDIGO ANTIGUO  ---

    // Solo actualizar enemigos si no están congelados
    // if (!freezeEnemies_) {
    //     for (auto &e : enemies) {
    //         // e.update(map_, deltaTime, tile_, playerPos.x, playerPos.y); // !! --- CÓDIGO ANTIGUO  ---
    //     }
    // }

    // !! --- CÓDIGO ANTIGUO  ---
    // enemiesPos_.clear();
    // for (auto &e : enemies) {
    //     enemiesPos_.push_back({ e.x * (float)tile_ + tile_ / 2.0f,
    //                             e.y * (float)tile_ + tile_ / 2.0f });
    // }

    // !! --- CÓDIGO ANTIGUO  ---
    // for (auto &e : enemies) {
    //     if (e.collidesWithPlayer(playerPos.x, playerPos.y, player_.getRadius()) && !player_.isInvulnerable()) {
    //         // Si colisiona, quitar una vida y empujar al jugador a la casilla previa
    //         player_.onHit(map_);
    //         // Notificar al enemigo que golpeó (solo los que persiguen se alejarán)
    //         e.onHitPlayer();
    //         std::cout << "El jugador ha sido golpeado por un enemigo. Vidas: " << player_.getLives() << std::endl;
    //     }
    // }


    // 6) Pinchos y colisiones si activo
    // spikes_.update(deltaTime); // !! --- CÓDIGO ANTIGUO  ---

    // Si el jugador está sobre un pincho activo
    // !! --- CÓDIGO ANTIGUO  ---
    // if (spikes_.isActiveAt(cellX, cellY) && !player_.isInvulnerable()) {
    //     // Si colisiona, quitar una vida y empujar al jugador a la casilla previa
    //     player_.onHit(map_);
    //         std::cout << "El jugador ha sido golpeado por pinchos. " << player_.getLives() << std::endl;
    // }

    // 7) Si el jugador no tiene vidas, cambiar al estado de Game Over
    // !! --- CÓDIGO ANTIGUO  ---
    // if (player_.getLives() <= 0) {
    //     std::cout << "Game Over: El jugador no tiene más vidas." << std::endl;
    //     // Game Over por muerte (dead = true)
    //     this->state_machine->add_state(std::make_unique<GameOverState>(level_, true, levelTime_, false), true);
    //     return;
    // }

    //7 Mecanismos, cmprobar si el jugador está sobre un trigger q no este activo
    // !! --- CÓDIGO ANTIGUO  ---
    // for (auto& mech : mechanisms_) {
    //     IVec2 trigPos = mech.getTriggerPos();

    //      if (mech.isActive() && cellX == trigPos.x && cellY == trigPos.y) {
    //         mech.deactivate();
    //     }
    // }
}

void MainGameState::render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    auto& rm = ResourceManager::Get();

    // Dimensiones
    const int mapWpx = map_.width()  * tile_;
    const int mapHpx = map_.height() * tile_;
    const int viewW  = GetScreenWidth();
    const int viewH  = GetScreenHeight() - HUD_HEIGHT; // Espacio disponible sin el HUD

    // Offset centrado (clamp >= 0) - El mapa queda centrado en el espacio disponible
    // const int ox = std::max(0, (viewW - mapWpx) / 2);
    // const int oy = std::max(0, (viewH - mapHpx) / 2);
    const int ox = (viewW > mapWpx) ? (viewW - mapWpx) / 2 : 0;
    const int oy = (viewH > mapHpx) ? (viewH - mapHpx) / 2 : 0;

    // 1) Mapa (dibujado en la zona superior, desde y=0 hasta y=MAP_H_PX)
    map_.render(ox, oy);

    // Mecanismos (ECS)
    RenderMechanismSystem(registry, ox, oy);
    // 2) Jugador
    // player_.render(ox, oy); // !! --- CÓDIGO ANTIGUO  ---
    RenderSystem(registry, (float)ox, (float)oy, (float)map_.tile());

    // !! --- CÓDIGO ANTIGUO  ---
    // 3) Enemigos (cuadrados)
    // for (auto &e : enemies) {
    //     e.draw(tile_, ox, oy, RED);
    // }

    // spikes_.render(ox, oy); // !! --- CÓDIGO ANTIGUO  ---

    // 4) HUD inferior - se coloca en la parte inferior de la ventana
    const float baseY = (float)(GetScreenHeight() - HUD_HEIGHT); // HUD siempre abajo
    // Fondo del HUD a lo ancho de la ventana
    Rectangle hudBg{ 0.0f, baseY, (float)GetScreenWidth(), (float)HUD_HEIGHT };
    DrawRectangleRec(hudBg, Fade(BLACK, 0.06f));
    // sombreado sutil encima del HUD
    for (int i=0; i<6; ++i) {
        Color c = Fade(BLACK, 0.05f * (6 - i));
        DrawLine(0, (int)baseY - i, GetScreenWidth(), (int)baseY - i, c);
    }

    // Panel de mochila (alineado a la izquierda dentro del HUD)
    const int pad = 10;
    Rectangle bagHud{ (float)pad, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad };
    DrawRectangleRounded(bagHud, 0.25f, 6, Fade(BLACK, 0.10f));
    DrawRectangleRoundedLinesEx(bagHud, 0.25f, 6, 1.0f, DARKGRAY);
    DrawText("Mochila", (int)bagHud.x + 10, (int)bagHud.y + 6, 16, DARKGRAY);

    // Panel de vidas (alineado a la derecha dentro del HUD)
    Rectangle livesHud{ (float)GetScreenWidth() - 190.0f, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad };
    DrawRectangleRounded(livesHud, 0.25f, 6, Fade(BLACK, 0.10f));
    DrawRectangleRoundedLinesEx(livesHud, 0.25f, 6, 1.0f, DARKGRAY);
    DrawText("Vidas", (int)livesHud.x + 10, (int)livesHud.y + 6, 16, DARKGRAY);

    // --------------------------------------------------------
    // 2. HUD - INTERFAZ DE USUARIO (Lectura desde ECS)
    // --------------------------------------------------------

    // Obtenemos la textura de iconos (Corazones y Llaves)
    Texture2D iconsTex = rm.GetTexture("assets/sprites/icons/Icons.png");

    // Buscamos la entidad que sea JUGADOR (tiene Stats y Input)
    auto view = registry.view<StatsComponent, TransformComponent, PlayerInputComponent>();

    for(auto entity : view) {
        const auto &stats = view.get<StatsComponent>(entity);
        const auto &trans = view.get<TransformComponent>(entity);

        // --- A. MOSTRAR LLAVES (MOCHILA) ---
        Rectangle keySrc = {64, 0, 16, 16};
        Rectangle keyDest = {bagHud.x + 10.0f, bagHud.y + 28.0f, 24.0f, 24.0f};
        DrawTexturePro(iconsTex, keySrc, keyDest, {0,0}, 0.0f, WHITE);
        std::string keyText = std::to_string(stats.keysCollected) + " / " + std::to_string(totalKeysInMap_);
        DrawText(keyText.c_str(), (int)bagHud.x + 42, (int)bagHud.y + 30, 20, DARKGRAY);

        // --- B. MOSTRAR VIDAS ---
        Rectangle heartSrc = {80, 0, 16, 16};
        for (int i = 0; i < stats.lives; i++) {
            float hX = livesHud.x + 10.0f + (i * 24.0f);
            float hY = livesHud.y + 28.0f;
            DrawTexturePro(iconsTex, heartSrc,
                Rectangle{hX, hY, 20.0f, 20.0f},
                {0,0}, 0.0f, WHITE);
        }

        // --- C. MENSAJE DE SALIDA (Contextual) ---
        // Calcular en qué casilla del mapa está el jugador
        int cx = (int)(trans.position.x / tile_);
        int cy = (int)(trans.position.y / tile_);

        // Verificamos límites del mapa para no leer memoria inválida
        if (cx >= 0 && cx < map_.width() && cy >= 0 && cy < map_.height()) {

            // Si la casilla actual es la SALIDA ('X')
            if (map_.at(cx, cy) == 'X') {
                std::string msg;
                Color msgColor;

                if (stats.keysCollected < totalKeysInMap_) {
                    // Caso: Faltan llaves
                    int remaining = totalKeysInMap_ - stats.keysCollected;
                    msg = "Necesitas " + std::to_string(remaining) + " llave" + (remaining > 1 ? "s" : "") + " más";
                    msgColor = RED;
                } else {
                    // Caso: Nivel completado
                    msg = "¡Presiona ENTER para Siguiente Nivel!";
                    msgColor = GREEN;
                }

                // Centrar el texto en la pantalla
                int textWidth = MeasureText(msg.c_str(), 24);
                DrawText(msg.c_str(), (GetScreenWidth() - textWidth) / 2, (int)baseY - 30, 24, msgColor);
            }
        }
    }

    // --- RENDERIZADO DE LLAVES EN EL HUD ---
    // 1. Carga del Sprite y recorte (mismos parámetros que en Map.cpp)
    // const Texture2D& atlasTex = ResourceManager::Get().GetTexture("sprites/icons/Icons.png");
    // float spriteSize = 16.0f; // Tamaño real del icono en el PNG (16x16)
    // int spriteIndex = 4;      // Índice de la llave

    // Calcular coordenadas dentro del spritesheet
    // int iconsPerRow = atlasTex.width / (int)spriteSize;
    // if (iconsPerRow == 0) iconsPerRow = 1;

    // Rectangle keySourceRect = {
    //     (float)(spriteIndex % iconsPerRow) * spriteSize, // X en el png
    //     (float)(spriteIndex / iconsPerRow) * spriteSize, // Y en el png
    //     spriteSize,
    //     spriteSize
    // };

    // 2. Dibujar las llaves que tiene el jugador
    // int iconDisplaySize = 28; // Tamaño visual en el HUD (ligeramente más pequeño que en el mapa)
    // int startX = (int)hud.x + 12;
    // int startY = (int)hud.y + 30;

    // !! --- CÓDIGO ANTIGUO  ---
    // for (int i = 0; i < player_.getKeyCount(); ++i) {
    //     Rectangle destRect = {
    //         (float)(startX + i * (iconDisplaySize + 4)), // Separación de 4px entre llaves
    //         (float)startY,
    //         (float)iconDisplaySize,
    //         (float)iconDisplaySize
    //     };
    //     DrawTexturePro(atlasTex, keySourceRect, destRect, {0,0}, 0.0f, WHITE);
    // }

    // Mostrar contador numérico (ej: "2/3") si hay llaves en el nivel,
    // útil para saber cuántas faltan de un vistazo.
    // !! --- CÓDIGO ANTIGUO  ---
    // if (totalKeysInMap_ > 0) {
    //     std::string keyCountText = std::to_string(player_.getKeyCount()) + "/" + std::to_string(totalKeysInMap_);
    //     DrawText(keyCountText.c_str(), (int)hud.x + 130, (int)hud.y + 35, 16, DARKGRAY);
    // }

    // ----------------------------------------

    // Panel de vidas (alineado a la derecha dentro del HUD)
    // DrawRectangleRounded(Rectangle{ (float)GetScreenWidth() - 190.0f, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad }, 0.25f, 6, Fade(BLACK, 0.10f));
    // DrawRectangleRoundedLinesEx(Rectangle{ (float)GetScreenWidth() - 190.0f, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad }, 0.25f, 6, 1.0f, DARKGRAY);
    // DrawText("Vidas", (int)((float)GetScreenWidth() - 190.0f) + 10, (int)(baseY + pad) + 6, 16, DARKGRAY);

    // !! --- CÓDIGO ANTIGUO  ---
    // if (player_.getLives() > 0) {
    //     for (int i = 0; i < player_.getLives(); ++i) {
    //         DrawCircleV(Vector2{ (float)GetScreenWidth() - 190.0f + 12.0f + i * 20.0f + 6.0f, baseY + pad + 28.0f + 6.0f }, 5.0f, RED);
    //     }
    // }

    // Mostrar temporizador centrado encima del HUD (formato mm:ss)
    int timerFont = 22;
    int minutes = (int)levelTime_ / 60;
    int seconds = (int)levelTime_ % 60;
    std::string timeText = "Tiempo: " + std::to_string(minutes) + ":" +
                          (seconds < 10 ? "0" : "") + std::to_string(seconds);
    int textW = MeasureText(timeText.c_str(), timerFont);
    DrawText(timeText.c_str(), (GetScreenWidth() - textW) / 2, (int)baseY + 8, timerFont, DARKGRAY);

    // Mostrar nivel actual arriba a la izquierda
    std::string levelText = "Nivel: " + std::to_string(level_);
    DrawText(levelText.c_str(), 10, 10, 24, DARKGRAY);

    // 5) Mensaje contextual por encima del HUD
    // !! --- CÓDIGO ANTIGUO  ---
    // const int cx = (int)(player_.getPosition().x / tile_);
    // const int cy = (int)(player_.getPosition().y / tile_);
    // if (cx >= 0 && cy >= 0 && cx < map_.width() && cy < map_.height()) {
    //     if (map_.at(cx, cy) == 'X' && !player_.hasAllKeys(totalKeysInMap_)) {
    //         std::string msg;
    //         if (totalKeysInMap_ == 1) {
    //             msg = "Necesitas la llave para salir";
    //         } else {
    //             int remaining = totalKeysInMap_ - player_.getKeyCount();
    //             msg = "Necesitas " + std::to_string(remaining) + " llave" + (remaining > 1 ? "s" : "") + " más";
    //         }
    //         const int font = 18;
    //         const int textW = MeasureText(msg.c_str(), font);
    //         // 10 px de margen por encima del HUD
    //         const int textY = (int)baseY - font - 10;
    //         DrawText(msg.c_str(), (GetScreenWidth()-textW)/2, textY, font, MAROON);
    //     }
    // }

    EndDrawing();
}

// void MainGameState::activeMechanisms() {
//     activeMechanisms_.clear();
//     for (const auto& mech : mechanisms_) {
//         if (mech.isActive()) {
//             Vector2 target = { (float)mech.getTargetPos().x, (float)mech.getTargetPos().y };
//             activeMechanisms_.push_back(target);
//         }
//     }
// }

void MainGameState::loadLevelEntities() {
    auto& rm = ResourceManager::Get();

    // Texturas precargadas
    Texture2D spikeTex = rm.GetTexture("assets/sprites/spikes.png");
    Texture2D enemyTex = rm.GetTexture("assets/sprites/player/Musketeer/Idle.png");
    Texture2D keyTex = rm.GetTexture("assets/sprites/icons/Icons.png");

    for (int y = 0; y < map_.height(); y++) {
        for (int x = 0; x < map_.width(); x++) {
            char cell = map_.at(x, y);

            // Calculamos posición central del tile en píxeles
            float centerX = x * map_.tile() + map_.tile() / 2.0f;
            float centerY = y * map_.tile() + map_.tile() / 2.0f;
            Vector2 pos = {centerX, centerY};
            Vector2 size = {(float)map_.tile(), (float)map_.tile()};
            Vector2 manualOffset = {0.0f, 0.0f};

            // --- CASO 1: PINCHOS (^) ---
            if (cell == '^') {
                auto entity = registry.create();
                registry.emplace<TransformComponent>(entity, pos, size);

                auto &sprite = registry.emplace<SpriteComponent>(entity, spikeTex);

                // 1. DEFINIR TAMAÑO DEL RECORTE (GRID)
                float frameW = spikeTex.width / 4.0f;
                float frameH = spikeTex.height / 8.0f;
                sprite.fixedFrameSize = {frameW, frameH};

                // 2. SELECCIONAR SPRITE ESPECÍFICO
                // Ahora "4" significará 4 bloques de 32px, no de 16px.
                sprite.currentRow = 4;   // Fila 5 (empezando en 0) -> activo
                sprite.currentFrame = 1; // Columna 2 (empezando en 0) como en legacy

                // 3. ESCALA PERSONALIZADA
                // Usamos el ancho del frame para cubrir el tile completo
                sprite.customScale = (float)map_.tile() / frameW;

                // Mantener centrado el sprite en el tile
                sprite.visualOffset = {0.0f, 0.0f};

                // Collider (ajustado)
                float hitSize = map_.tile() * 0.9f;
                registry.emplace<ColliderComponent>(entity,
                    Rectangle{-hitSize/2, -hitSize/2, hitSize, hitSize},
                    CollisionType::Spike
                );

                // Pinchos retráctiles (intervalo por defecto = 3s)
                // Ajuste de alineación vertical basado en el sprite actual (spikes.png)
                float scale = (float)map_.tile() / frameW;
                float inactiveOffsetY = (float)map_.tile() - (30.0f * scale); // fila 0: bbox bottom ~30px
                float activeOffsetY = (float)map_.tile() - (24.0f * scale);   // fila 4: bbox bottom ~24px
                registry.emplace<SpikeComponent>(entity, true, 3.0f, activeOffsetY, inactiveOffsetY);
            }

            // --- CASO 2: ENEMIGOS (E) ---
            else if (cell == 'E') {
                auto entity = registry.create();
                registry.emplace<TransformComponent>(entity, pos, size);

                // Configuración visual del enemigo (igual que el player: 6 frames, offset 8,-8)
                manualOffset = Vector2{-3.0f, -10.0f};
                registry.emplace<SpriteComponent>(entity, enemyTex, 6, manualOffset);

                // Movimiento (IA)
                registry.emplace<MovementComponent>(entity, 100.0f); // Velocidad un poco más lenta que el jugador

                // Collider (90% del tile)
                float hitSize = map_.tile() * 0.9f;
                registry.emplace<ColliderComponent>(entity,
                    Rectangle{-hitSize/2, -hitSize/2, hitSize, hitSize},
                    CollisionType::Enemy
                );
            }

            // --- CASO 3: LLAVES (K) ---
            if (cell == 'K') {
                auto entity = registry.create();
                registry.emplace<TransformComponent>(entity, pos, size);

                // 1. Creamos el componente básico
                auto &sprite = registry.emplace<SpriteComponent>(entity, keyTex);

                // 2. CONFIGURACIÓN DEL RECORTE MANUAL
                // Forzamos a que el recorte sea de 16x16 píxeles
                sprite.fixedFrameSize = {16.0f, 16.0f};

                // 3. Seleccionamos el índice 4 (X = 16 * 4 = 64)
                sprite.currentFrame = 4;

                // La llave original es de 16px.
                // Con escala 1.5 -> 24px (queda centrada con margen en el tile de 32)
                sprite.customScale = 1.5f;

                // Colisión (tipo Item)
                float hitSize = map_.tile() * 0.5f;
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
