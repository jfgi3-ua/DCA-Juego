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
    player_.init(startPos, tile_ * 0.35f, 5, "sprites/player/Musketeer"); //inicializamos el jugador con la carpeta de sprites

    enemiesPos_.clear();
    enemies.clear();
    for (auto e : map_.enemyStarts()) {
        enemiesPos_.push_back({ e.x * (float)tile_ + tile_ / 2.0f,
                                e.y * (float)tile_ + tile_ / 2.0f });
        // crear un único Enemy usando el constructor que necesita tile_
        enemies.emplace_back(e.x, e.y, tile_);
    }

    for (auto s : map_.spikesStarts()) {
        spikes_.addSpike(s.x, s.y);
    }

    for (auto m : map_.getMechanisms()) {
        //m es un MechanismPair
        mechanisms_.emplace_back(m.id, m.trigger, m.target);
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

    // 3. Guardamos la posición central.
    registry.emplace<TransformComponent>(playerEntity, Vector2{centerX, centerY}, Vector2{(float)map_.tile(), (float)map_.tile()});

    // 4. Configuración del Sprite
    Texture2D playerTex = LoadTexture("assets/sprites/player/Archer/Idle.png");
    Vector2 manualOffset = { 0.0f, -10.0f };  // Ajuste manual del sprite
    registry.emplace<SpriteComponent>(playerEntity, playerTex, 6, manualOffset);

    // 5. Componente de Movimiento (Velocidad 150.0f igual que Player.hpp)
    registry.emplace<MovementComponent>(playerEntity, 150.0f);

    // 6. Etiqueta de Input (para que sepa que ESTE es el jugador controlable)
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

    // 2. Crear un PINCHO DE PRUEBA (Entidad ECS)
    auto spikeEntity = registry.create();

    float spikeX = (startGridPos.x + 2) * tile_ + tile_ / 2.0f;
    float spikeY = startGridPos.y * tile_ + tile_ / 2.0f;

    registry.emplace<TransformComponent>(spikeEntity, Vector2{spikeX, spikeY}, Vector2{(float)tile_, (float)tile_});

    // Cargar textura de pinchos
    Texture2D spikeTex = ResourceManager::Get().GetTexture("assets/sprites/spikes.png");
    registry.emplace<SpriteComponent>(spikeEntity, spikeTex); // Imagen estática ahora mismo... ya veremos si animarla luego

    // Añadir Collider al pincho
    // hitbox un poco más pequeña para dar margen
    registry.emplace<ColliderComponent>(spikeEntity,
        Rectangle{ -hitSize/2, -hitSize/2, hitSize, hitSize },
        CollisionType::Spike
    );
}

void MainGameState::handleInput()
{
    // Activar menú de desarrollador con CTRL+D
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D)) {
        this->state_machine->add_overlay_state(
            std::make_unique<DevModeState>(&player_, &levelTime_, &freezeEnemies_, &infiniteTime_, &keyGivenByCheating_, level_)
        );
        return;
    }
}

void MainGameState::update(float deltaTime)
{
    activeMechanisms(); //actualizamos un vector con todos los mecanismos activos

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

    // Primero Input (decide destino), luego Movimiento (mueve)
    InputSystem(registry, map_);
    MovementSystem(registry, deltaTime);

    // Chequeo de colisiones
    CollisionSystem(registry, map_);

    // 2) Celda actual del jugador
    int cellX = (int)(player_.getPosition().x) / tile_;
    int cellY = (int)(player_.getPosition().y) / tile_;

    // 3) ¿Está sobre una llave 'K'? -> Recogerla
    try {
        if (map_.at(cellX, cellY) == 'K') {
            player_.addKey();
            map_.clearCell(cellX, cellY); // Reemplaza 'K' por '.', es decir, retira la llave del mapa.
            std::cout << "Llave recogida! (" << player_.getKeyCount() << "/" << totalKeysInMap_ << ")" << std::endl;
            // TODO:Aquí podríamos reproducir un sonido o mostrar un mensaje en pantalla si se desea.
        }
    } catch (const std::out_of_range& e) {
        // Ignorar fuera de rango (no debería ocurrir aquí). Fuera de rango no debería ocurrir si los cálculos de celda son correctos.
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // 4) ¿Está sobre la salida 'X' y tiene todas las llaves? -> Nivel completado
    if (player_.isOnExit(map_) && player_.hasAllKeys(totalKeysInMap_)) {
        std::cout << "Nivel completado" << std::endl;
        // Si es el nivel 6 (último), mostrar pantalla de victoria directamente
        if (level_ >= 6) {
            this->state_machine->add_state(std::make_unique<GameOverState>(level_, false, levelTime_, true), true);
        } else {
            // Pasar a pantalla de nivel completado (dead = false)
            this->state_machine->add_state(std::make_unique<GameOverState>(level_, false, levelTime_, false), true);
        }
        return;
    }

    // 5) IA enemigos y colisiones con jugador
    // Pasar la posición del jugador a los enemigos para el árbol de decisiones
    Vector2 playerPos = player_.getPosition();

    // Solo actualizar enemigos si no están congelados
    if (!freezeEnemies_) {
        for (auto &e : enemies) {
            e.update(map_, deltaTime, tile_, playerPos.x, playerPos.y);
        }
    }

    enemiesPos_.clear();
    for (auto &e : enemies) {
        enemiesPos_.push_back({ e.x * (float)tile_ + tile_ / 2.0f,
                                e.y * (float)tile_ + tile_ / 2.0f });
    }

    for (auto &e : enemies) {
        if (e.collidesWithPlayer(playerPos.x, playerPos.y, player_.getRadius()) && !player_.isInvulnerable()) {
            // Si colisiona, quitar una vida y empujar al jugador a la casilla previa
            player_.onHit(map_);
            // Notificar al enemigo que golpeó (solo los que persiguen se alejarán)
            e.onHitPlayer();
            std::cout << "El jugador ha sido golpeado por un enemigo. Vidas: " << player_.getLives() << std::endl;
        }
    }


    // 6) Pinchos y colisiones si activo
    spikes_.update(deltaTime);
    // Si el jugador está sobre un pincho activo
    if (spikes_.isActiveAt(cellX, cellY) && !player_.isInvulnerable()) {
    // Si colisiona, quitar una vida y empujar al jugador a la casilla previa
    player_.onHit(map_);
        std::cout << "El jugador ha sido golpeado por pinchos. " << player_.getLives() << std::endl;
    }

    // 7) Si el jugador no tiene vidas, cambiar al estado de Game Over
    if (player_.getLives() <= 0) {
        std::cout << "Game Over: El jugador no tiene más vidas." << std::endl;
        // Game Over por muerte (dead = true)
        this->state_machine->add_state(std::make_unique<GameOverState>(level_, true, levelTime_, false), true);
        return;
    }

    //7 Mecanismos, cmprobar si el jugador está sobre un trigger q no este activo
    for (auto& mech : mechanisms_) {
        IVec2 trigPos = mech.getTriggerPos();

         if (mech.isActive() && cellX == trigPos.x && cellY == trigPos.y) {
            mech.deactivate();
        }
    }
}

void MainGameState::render()
{
    ClearBackground(RAYWHITE);

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

    // Mecanismos
    for (const auto& mech : mechanisms_) {
        mech.render(ox, oy);
    }
    // 2) Jugador
    player_.render(ox, oy);
    RenderSystem(registry, (float)ox, (float)oy, (float)map_.tile());

    // 3) Enemigos (cuadrados)
    for (auto &e : enemies) {
        e.draw(tile_, ox, oy, RED);
    }

    spikes_.render(ox, oy);


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
    Rectangle hud{ (float)pad, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad };
    DrawRectangleRounded(hud, 0.25f, 6, Fade(BLACK, 0.10f));
    DrawRectangleRoundedLinesEx(hud, 0.25f, 6, 1.0f, DARKGRAY);
    DrawText("Mochila", (int)hud.x + 10, (int)hud.y + 6, 16, DARKGRAY);

    // --- RENDERIZADO DE LLAVES EN EL HUD ---

    // 1. Carga del Sprite y recorte (mismos parámetros que en Map.cpp)
    const Texture2D& atlasTex = ResourceManager::Get().GetTexture("sprites/icons/Icons.png");
    float spriteSize = 16.0f; // Tamaño real del icono en el PNG (16x16)
    int spriteIndex = 4;      // Índice de la llave

    // Calcular coordenadas dentro del spritesheet
    int iconsPerRow = atlasTex.width / (int)spriteSize;
    if (iconsPerRow == 0) iconsPerRow = 1;

    Rectangle keySourceRect = {
        (float)(spriteIndex % iconsPerRow) * spriteSize, // X en el png
        (float)(spriteIndex / iconsPerRow) * spriteSize, // Y en el png
        spriteSize,
        spriteSize
    };

    // 2. Dibujar las llaves que tiene el jugador
    int iconDisplaySize = 28; // Tamaño visual en el HUD (ligeramente más pequeño que en el mapa)
    int startX = (int)hud.x + 12;
    int startY = (int)hud.y + 30;

    for (int i = 0; i < player_.getKeyCount(); ++i) {
        Rectangle destRect = {
            (float)(startX + i * (iconDisplaySize + 4)), // Separación de 4px entre llaves
            (float)startY,
            (float)iconDisplaySize,
            (float)iconDisplaySize
        };
        DrawTexturePro(atlasTex, keySourceRect, destRect, {0,0}, 0.0f, WHITE);
    }

    // Mostrar contador numérico (ej: "2/3") si hay llaves en el nivel,
    // útil para saber cuántas faltan de un vistazo.
    if (totalKeysInMap_ > 0) {
        std::string keyCountText = std::to_string(player_.getKeyCount()) + "/" + std::to_string(totalKeysInMap_);
        DrawText(keyCountText.c_str(), (int)hud.x + 130, (int)hud.y + 35, 16, DARKGRAY);
    }

    // ----------------------------------------

    // Panel de vidas (alineado a la derecha dentro del HUD)
    DrawRectangleRounded(Rectangle{ (float)GetScreenWidth() - 190.0f, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad }, 0.25f, 6, Fade(BLACK, 0.10f));
    DrawRectangleRoundedLinesEx(Rectangle{ (float)GetScreenWidth() - 190.0f, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad }, 0.25f, 6, 1.0f, DARKGRAY);
    DrawText("Vidas", (int)((float)GetScreenWidth() - 190.0f) + 10, (int)(baseY + pad) + 6, 16, DARKGRAY);

    if (player_.getLives() > 0) {
        for (int i = 0; i < player_.getLives(); ++i) {
            DrawCircleV(Vector2{ (float)GetScreenWidth() - 190.0f + 12.0f + i * 20.0f + 6.0f, baseY + pad + 28.0f + 6.0f }, 5.0f, RED);
        }
    }

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
    const int cx = (int)(player_.getPosition().x / tile_);
    const int cy = (int)(player_.getPosition().y / tile_);
    if (cx >= 0 && cy >= 0 && cx < map_.width() && cy < map_.height()) {
        if (map_.at(cx, cy) == 'X' && !player_.hasAllKeys(totalKeysInMap_)) {
            std::string msg;
            if (totalKeysInMap_ == 1) {
                msg = "Necesitas la llave para salir";
            } else {
                int remaining = totalKeysInMap_ - player_.getKeyCount();
                msg = "Necesitas " + std::to_string(remaining) + " llave" + (remaining > 1 ? "s" : "") + " más";
            }
            const int font = 18;
            const int textW = MeasureText(msg.c_str(), font);
            // 10 px de margen por encima del HUD
            const int textY = (int)baseY - font - 10;
            DrawText(msg.c_str(), (GetScreenWidth()-textW)/2, textY, font, MAROON);
        }
    }

    EndDrawing();
}

void MainGameState::activeMechanisms() {
    activeMechanisms_.clear();
    for (const auto& mech : mechanisms_) {
        if (mech.isActive()) {
            Vector2 target = { (float)mech.getTargetPos().x, (float)mech.getTargetPos().y };
            activeMechanisms_.push_back(target);
        }
    }
}
