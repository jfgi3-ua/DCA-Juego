#include <libintl.h>
#define _(String) gettext(String)
#include "MainGameState.hpp"
#include "GameOverState.hpp"
#include "DevModeState.hpp"
#include "StateMachine.hpp"
#include "ResourceManager.hpp"
#include "PlayerSelection.hpp"
#include "PlayerSpriteCatalog.hpp"
#include "ecs/Ecs.hpp"
#include <algorithm>
extern "C" {
  #include <raylib.h>
}

static int ComputeFramesForTexture(const Texture2D& tex) {
    if (tex.height <= 0) return 1;
    int frames = tex.width / tex.height;
    return frames > 0 ? frames : 1;
}

MainGameState::MainGameState(int level)
{
    _level = level > 0 ? level : 1;
}

void MainGameState::init()
{
    auto& rm = ResourceManager::Get();
    std::string relativePath = "maps/map_" + std::to_string(_level) + ".txt";
    std::string absolutePath = rm.GetAssetPath(relativePath);

    _map.loadFromFile(absolutePath, TILE_SIZE);
    _map.loadTextures(); //lo llamamos aqui ya q tambien se llama en main y no se pueden cargar texturas antes de InitWindow
    _tile = _map.tile();

    // Guardar total de llaves del mapa (antes de que se recojan)
    _totalKeysInMap = _map.getTotalKeys();

    // Cargar entidades del nivel en el registry
    LevelSetupSystem(_registry, _map);

    // Inicializar temporizador: 45s base + 60s por cada nivel adicional
    levelTime_ = 45.0f + (_level - 1) * 60.0f;

    // ---------------------------------------------------------
    // REFACTOR: ENTT
    // ---------------------------------------------------------
    // --- PLAYER ---
    // Verificar si LevelSetupSystem ya creó el jugador
    auto existingPlayerView = _registry.view<PlayerInputComponent>();
    if (!existingPlayerView.empty()) {
        // El jugador ya existe, no crear uno nuevo
        return;
    }

    // 1. Obtener coordenadas del grid donde está la 'P' (ej: x=2, y=3)
    IVec2 startGridPos = _map.playerStart();

    // 2. Convertir a posición de mundo (píxeles)
    // Usamos la esquina superior izquierda del tile como referencia (más fácil para ECS)
    float centerX = (startGridPos.x * _map.tile()) + (_map.tile() / 2.0f);
    float centerY = (startGridPos.y * _map.tile()) + (_map.tile() / 2.0f);
    auto playerEntity = _registry.create();

    // 3. Componente de Stats
    _registry.emplace<PlayerStatsComponent>(playerEntity, 5); // 5 vidas iniciales

    // 4. Guardamos la posición central.
    _registry.emplace<TransformComponent>(playerEntity, Vector2{centerX, centerY}, Vector2{(float)_map.tile(), (float)_map.tile()});

    // 5. Configuración del Sprite
    std::string idlePath;
    std::string walkPath;
    if (PlayerSelection::HasSelectedSpriteSet() &&
        PlayerSelection::SelectedHasIdle() && PlayerSelection::SelectedHasWalk()) {
        idlePath = PlayerSelection::GetSelectedIdlePath();
        walkPath = PlayerSelection::GetSelectedWalkPath();
    } else {
        auto sets = DiscoverPlayerSpriteSets();
        auto defaultId = ResolveDefaultPlayerSpriteSetId(sets);
        auto it = std::find_if(sets.begin(), sets.end(),
                               [&](const PlayerSpriteSet& set) { return set.id == defaultId; });
        if (it != sets.end() && it->hasIdle && it->hasWalk) {
            idlePath = it->idlePath;
            walkPath = it->walkPath;
        } else {
            idlePath = "sprites/player/Archer/Idle.png";
            walkPath = "sprites/player/Archer/Walk.png";
        }
    }

    Texture2D playerIdleTex = rm.GetTexture(idlePath);
    Texture2D playerWalkTex = rm.GetTexture(walkPath);
    Vector2 manualOffset = { 0.0f, -10.0f };  // Ajuste manual del sprite
    int idleFrames = ComputeFramesForTexture(playerIdleTex);
    int walkFrames = ComputeFramesForTexture(playerWalkTex);
    _registry.emplace<SpriteComponent>(playerEntity, playerIdleTex, manualOffset, 1.5f);
    _registry.emplace<GridClipComponent>(playerEntity, idleFrames);
    _registry.emplace<AnimationComponent>(playerEntity, playerIdleTex, playerWalkTex,
                                         idleFrames, walkFrames, 0.2f, 0.12f);

    // 6. Componente de Movimiento (Velocidad 150.0f igual que Player.hpp)
    _registry.emplace<MovementComponent>(playerEntity, 75.0f);

    // 7. Etiqueta de Input (para que sepa que ESTE es el jugador controlable) <-- // ?? Esta parte tengo que estudiarmela mejor
    _registry.emplace<PlayerInputComponent>(playerEntity);

    // 8. Estado de jugador (invulnerabilidad y retroceso)
    _registry.emplace<PlayerStateComponent>(playerEntity, Vector2{centerX, centerY}, 1.5f);

    // 9. Cheats del jugador (god/no-clip)
    _registry.emplace<PlayerCheatComponent>(playerEntity, false, false);

    // -- Colisiones --
    // 1. Añadir ColliderComponent al JUGADOR
    // Ajustamos la caja para que sea un poco más pequeña que el tile (hitbox permisiva... de momento)
    float hitSize = _tile * 0.6f;
    // Offset centrado relativo al centro del personaje (que es donde está transform.position)
    // Como transform.position es el CENTRO, un rect en {-w/2, -h/2} estaría centrado.
    _registry.emplace<ColliderComponent>(playerEntity,
        Rectangle{ -hitSize/2, -hitSize/2, hitSize, hitSize },
        CollisionType::Player
    );

    std::cout << "Nivel cargado. Entidades generadas via ECS." << std::endl;
}

void MainGameState::handleInput()
{
    // 1. Activar menú de desarrollador con CTRL+D
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D)) {
        this->state_machine->add_overlay_state(
            std::make_unique<DevModeState>(&_registry, &levelTime_, &_freezeEnemies, &_infiniteTime,
                                           &_keyGivenByCheating, &_totalKeysInMap, _level)
        );
        return;
    }

    // 2. Salir al estado de Game Over al presionar ESPACIO (simulando derrota)
    if (IsKeyPressed(KEY_SPACE)) {
        // Argumentos de GameOverState: nivel actual, ha muerto (true), tiempo restante, juego terminado (false)
        this->state_machine->add_state(
            std::make_unique<GameOverState>(_level, true, levelTime_, false), 
            true // Reemplazar el estado actual
        );
        return;
    }
}

void MainGameState::_checkGameEndConditions()
{
    auto playerView = _registry.view<TransformComponent, PlayerStatsComponent, PlayerInputComponent>();
    if (!playerView) {
        return;
    }

    auto playerEntity = *playerView.begin();
    const auto &stats = playerView.get<PlayerStatsComponent>(playerEntity);
    const auto &trans = playerView.get<TransformComponent>(playerEntity);

    // Derrota por vidas
    if (stats.lives <= 0) {
        this->state_machine->add_state(std::make_unique<GameOverState>(_level, true, levelTime_, false), true);
        return;
    }

    // Victoria por salida + llaves
    int cellX = (int)(trans.position.x / _tile);
    int cellY = (int)(trans.position.y / _tile);
    if (cellX >= 0 && cellY >= 0 && cellX < _map.width() && cellY < _map.height()) {
        if (_map.at(cellX, cellY) == 'X' && stats.keysCollected >= _totalKeysInMap) {
            bool gameFinished = (_level >= 6);
            this->state_machine->add_state(std::make_unique<GameOverState>(_level, false, levelTime_, gameFinished), true);
        }
    }
}

void MainGameState::update(float deltaTime)
{
    // Reducir temporizador de nivel (excepto si está en modo tiempo infinito)
    if (!_infiniteTime) {
        levelTime_ -= deltaTime;
        if (levelTime_ <= 0.0f) {
            levelTime_ = 0.0f;
            // Tiempo agotado -> Game Over (dead = true)
            this->state_machine->add_state(std::make_unique<GameOverState>(_level, true, 0.0f, false), true);
            return;
        }
    }

    // Primero Input (decide destino), luego Movimiento (mueve)
    InputSystem(_registry, _map);
    if (!_freezeEnemies) {
        EnemyAISystem(_registry, _map, deltaTime);
    }
    MovementSystem(_registry, deltaTime);
    AnimationSystem(_registry, deltaTime);
    SpikeSystem(_registry, deltaTime);
    InvulnerabilitySystem(_registry, deltaTime);
    CollisionSystem(_registry, _map); // Chequeo de colisiones
    MechanismSystem(_registry, _map);

    _checkGameEndConditions();
}

void MainGameState::_renderMap(){
    // Dimensiones
    const int mapWpx = _map.width()  * _tile;
    const int mapHpx = _map.height() * _tile;
    const int viewW  = GetScreenWidth();
    const int viewH  = GetScreenHeight() - HUD_HEIGHT; // Espacio disponible sin el HUD

    // Offset centrado (clamp >= 0) - El mapa queda centrado en el espacio disponible
    const int ox = (viewW > mapWpx) ? (viewW - mapWpx) / 2 : 0;
    const int oy = (viewH > mapHpx) ? (viewH - mapHpx) / 2 : 0;

    // 1) Mapa (dibujado en la zona superior, desde y=0 hasta y=MAP_H_PX)
    _map.render(ox, oy);

    RenderSystem(_registry, (float)ox, (float)oy, (float)_map.tile());
}

void MainGameState::_renderHUD(){
    
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
    DrawText(_("Mochila"), (int)bagHud.x + 10, (int)bagHud.y + 6, 16, DARKGRAY);

    // Panel de vidas (alineado a la derecha dentro del HUD)
    Rectangle livesHud{ (float)GetScreenWidth() - 190.0f, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad };
    DrawRectangleRounded(livesHud, 0.25f, 6, Fade(BLACK, 0.10f));
    DrawRectangleRoundedLinesEx(livesHud, 0.25f, 6, 1.0f, DARKGRAY);
    DrawText(_("Vidas"), (int)livesHud.x + 10, (int)livesHud.y + 6, 16, DARKGRAY);

    _renderPlayerHUD(bagHud, livesHud, baseY);
}

void MainGameState::_renderPlayerHUD(const Rectangle& bagHud, const Rectangle& livesHud, float baseY)
{
    // --------------------------------------------------------
    // 2. HUD - INTERFAZ DE USUARIO (Lectura desde ECS)
    // --------------------------------------------------------

    // Obtenemos la textura de iconos (Corazones y Llaves)
    auto& rm = ResourceManager::Get();
    Texture2D iconsTex = rm.GetTexture("sprites/icons/Icons.png");

    // Buscamos la entidad que sea JUGADOR (tiene Stats y Input)
    auto view = _registry.view<PlayerStatsComponent, TransformComponent, PlayerInputComponent, PlayerCheatComponent, PlayerStateComponent>();

    for(auto entity : view) {
        const auto &stats = view.get<PlayerStatsComponent>(entity);
        const auto &trans = view.get<TransformComponent>(entity);
        const auto &cheats = view.get<PlayerCheatComponent>(entity);
        const auto &playerState = view.get<PlayerStateComponent>(entity);

        // --- A. MOSTRAR LLAVES (MOCHILA) ---
        Rectangle keySrc = {64, 0, 16, 16};
        Rectangle keyDest = {bagHud.x + 10.0f, bagHud.y + 28.0f, 24.0f, 24.0f};
        DrawTexturePro(iconsTex, keySrc, keyDest, {0,0}, 0.0f, WHITE);
        std::string keyText = std::to_string(stats.keysCollected) + " / " + std::to_string(_totalKeysInMap);
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

        // --- C. ESTADO DE CHEATS (HUD) ---
        if (cheats.godMode) {
            DrawText(_("GOD"), (int)livesHud.x + 10, (int)livesHud.y + 8, 16, GOLD);
        }
        if (cheats.noClip) {
            DrawText(_("NOCLIP"), (int)livesHud.x + 60, (int)livesHud.y + 8, 16, GOLD);
        }
        if (playerState.invulnerableTimer > 0.0f && playerState.invulnerableTimer < playerState.invulnerableDuration) {
            DrawText(_("INVULNERABLE"), (int)bagHud.x + 10, (int)bagHud.y - 18, 14, MAROON);
        }

        // --- D. MENSAJE DE SALIDA (Contextual) ---
        // Calcular en qué casilla del mapa está el jugador
        int cx = (int)(trans.position.x / _tile);
        int cy = (int)(trans.position.y / _tile);

        // Verificamos límites del mapa para no leer memoria inválida
        if (cx >= 0 && cx < _map.width() && cy >= 0 && cy < _map.height()) {

            // Si la casilla actual es la SALIDA ('X')
            if (_map.at(cx, cy) == 'X') {
                std::string msg;
                Color msgColor;

                if (stats.keysCollected < _totalKeysInMap) {
                    // Caso: Faltan llaves
                    int remaining = _totalKeysInMap - stats.keysCollected;
                    msg = std::string(_("Necesitas ")) + std::to_string(remaining) + " " + _(remaining > 1 ? "llaves" : "llave") + " " + _("más");
                    msgColor = RED;
                } else {
                    // Caso: Nivel completado
                    msg = _( "¡Presiona ENTER para Siguiente Nivel!" );
                    msgColor = GREEN;
                }

                // Centrar el texto en la pantalla
                int textWidth = MeasureText(msg.c_str(), 24);
                DrawText(msg.c_str(), (GetScreenWidth() - textWidth) / 2, (int)baseY - 30, 24, msgColor);
            }
        }
    }
}

void MainGameState::_renderTimerAndLevel()
{
    // Mostrar temporizador centrado encima del HUD (formato mm:ss)
    int timerFont = 22;
    int minutes = (int)levelTime_ / 60;
    int seconds = (int)levelTime_ % 60;
    std::string timeText = std::string(_("Tiempo: ")) + std::to_string(minutes) + ":" +
                          (seconds < 10 ? "0" : "") + std::to_string(seconds);
    int textW = MeasureText(timeText.c_str(), timerFont);
    DrawText(timeText.c_str(), (GetScreenWidth() - textW) / 2, (int)(GetScreenHeight() - HUD_HEIGHT) + 8, timerFont, DARKGRAY);

    // Mostrar nivel actual arriba a la izquierda
    std::string levelText = std::string(_("Nivel: ")) + std::to_string(_level);
    DrawText(levelText.c_str(), 10, 10, 24, YELLOW);
}

void MainGameState::render()
{
    ClearBackground(RAYWHITE);
    _renderMap();
    _renderHUD();
    _renderTimerAndLevel();
}
