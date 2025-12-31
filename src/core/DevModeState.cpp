#include "DevModeState.hpp"
#include "StateMachine.hpp"
#include "MainGameState.hpp"
#include "GameOverState.hpp"
#include "ecs/Ecs.hpp"
#include <iostream>

extern "C" {
    #include <raylib.h>
}

DevModeState::DevModeState(entt::registry* registry, float* levelTime, bool* freezeEnemies,
                           bool* infiniteTime, bool* keyGivenByCheating, int* totalKeysInMap,
                           int currentLevel)
    : _registry(registry), _playerEntity(entt::null), _levelTime(levelTime), _freezeEnemies(freezeEnemies),
      _infiniteTime(infiniteTime), _keyGivenByCheating(keyGivenByCheating), _totalKeysInMap(totalKeysInMap),
      _currentLevel(currentLevel),
      _awaitingPassword(true), _selectedOption(-1), _authenticated(false),
      _godMode(false), _noClip(false)
{
}

DevModeState::~DevModeState()
{
}

void DevModeState::init()
{
    _awaitingPassword = true;
    _authenticated = false;
    _passwordInput.clear();
    _selectedOption = -1;

    // Sincronizar estado de cheats con ECS
    if (_resolvePlayerEntity() && _registry && _registry->all_of<PlayerCheatComponent>(_playerEntity)) {
        const auto &cheats = _registry->get<PlayerCheatComponent>(_playerEntity);
        _godMode = cheats.godMode;
        _noClip = cheats.noClip;
    }

    // Configurar las opciones de cheats
    _setupCheatOptions();
}

void DevModeState::_setupCheatOptions()
{
    cheatOptions_.clear();

    // Verificar que los punteros son válidos
    if (!_registry || !_freezeEnemies || !_infiniteTime || !_keyGivenByCheating) {
        std::cerr << "ERROR: Punteros nulos en setupCheatOptions!" << std::endl;
        return;
    }

    _resolvePlayerEntity();

    // 0. God Mode
    cheatOptions_.push_back({
        "1. God Mode (Invulnerabilidad)",
        [this]() {
            _godMode = !_godMode;
            if (_registry && _registry->valid(_playerEntity) && _registry->all_of<PlayerCheatComponent>(_playerEntity)) {
                auto &cheats = _registry->get<PlayerCheatComponent>(_playerEntity);
                cheats.godMode = _godMode;
            }
            std::cout << "God Mode: " << (_godMode ? "ON" : "OFF") << std::endl;
        },
        [this]() { return _godMode ? "[ON]" : "[OFF]"; }
    });

    // 1. Congelar Enemigos
    cheatOptions_.push_back({
        "2. Congelar Enemigos",
        [this]() {
            if (_freezeEnemies) {
                *_freezeEnemies = !(*_freezeEnemies);
                std::cout << "Freeze Enemies: " << (*_freezeEnemies ? "ON" : "OFF") << std::endl;
            }
        },
        [this]() { return (_freezeEnemies && *_freezeEnemies) ? "[ON]" : "[OFF]"; }
    });

    // 2. Tiempo Infinito
    cheatOptions_.push_back({
        "3. Tiempo Infinito",
        [this]() {
            if (_infiniteTime) {
                *_infiniteTime = !(*_infiniteTime);
                std::cout << "Infinite Time: " << (*_infiniteTime ? "ON" : "OFF") << std::endl;
            }
        },
        [this]() { return (_infiniteTime && *_infiniteTime) ? "[ON]" : "[OFF]"; }
    });

    // 3. NoClip
    cheatOptions_.push_back({
        "4. NoClip (Atravesar Paredes)",
        [this]() {
            _noClip = !_noClip;
            if (_registry && _registry->valid(_playerEntity) && _registry->all_of<PlayerCheatComponent>(_playerEntity)) {
                auto &cheats = _registry->get<PlayerCheatComponent>(_playerEntity);
                cheats.noClip = _noClip;
            }
            std::cout << "NoClip: " << (_noClip ? "ON" : "OFF") << std::endl;
        },
        [this]() { return _noClip ? "[ON]" : "[OFF]"; }
    });

    // 4. Añadir vida
    cheatOptions_.push_back({
        "5. Anadir +1 Vida",
        [this]() {
            if (_registry && _registry->valid(_playerEntity) && _registry->all_of<PlayerStatsComponent>(_playerEntity)) {
                auto &stats = _registry->get<PlayerStatsComponent>(_playerEntity);
                if (stats.lives < 10) {
                    stats.lives++;
                }
                std::cout << "Vida anadida. Vidas: " << stats.lives << std::endl;
            }
        },
        [this]() { return ""; }
    });

    // 5. Vidas máximas
    cheatOptions_.push_back({
        "6. Vidas Maximas (10)",
        [this]() {
            if (_registry && _registry->valid(_playerEntity) && _registry->all_of<PlayerStatsComponent>(_playerEntity)) {
                auto &stats = _registry->get<PlayerStatsComponent>(_playerEntity);
                stats.lives = 10;
                std::cout << "Vidas al maximo (10)" << std::endl;
            }
        },
        [this]() { return ""; }
    });

    // 6. Dar llave
    cheatOptions_.push_back({
        "7. Obtener Llave",
        [this]() {
            if (_registry && _registry->valid(_playerEntity) && _registry->all_of<PlayerStatsComponent>(_playerEntity)) {
                auto &stats = _registry->get<PlayerStatsComponent>(_playerEntity);
                if (stats.keysCollected == 0) {
                    // Evitar misleading-indentation: usar llaves explícitas
                    if (_keyGivenByCheating) {
                        *_keyGivenByCheating = true;
                    }
                    int targetKeys = _totalKeysInMap ? *_totalKeysInMap : 1;
                    stats.keysCollected = targetKeys;
                    std::cout << "Llave obtenida (por cheat)" << std::endl;
                } else {
                    std::cout << "Ya tienes la llave" << std::endl;
                }
            }
        },
        [this]() {
            if (_registry && _registry->valid(_playerEntity) && _registry->all_of<PlayerStatsComponent>(_playerEntity)) {
                auto &stats = _registry->get<PlayerStatsComponent>(_playerEntity);
                return (stats.keysCollected > 0) ? "[SI]" : "[NO]";
            }
            return "[NO]";
        }
    });

    // 7. Añadir tiempo
    cheatOptions_.push_back({
        "8. Anadir +30 Segundos",
        [this]() {
            if (_levelTime) {
                *_levelTime += 30.0f;
                std::cout << "30 segundos anadidos" << std::endl;
            }
        },
        [this]() { return ""; }
    });

    // 8. Saltar nivel
    cheatOptions_.push_back({
        "9. Saltar Nivel",
        [this]() {
            if (this->state_machine) {
                this->state_machine->remove_overlay_state();
                // Si estamos en el nivel 6 o superior, mostrar pantalla de victoria
                if (_currentLevel >= 6) {
                    std::cout << "Nivel final completado - Mostrando pantalla de victoria..." << std::endl;
                    this->state_machine->add_state(std::make_unique<GameOverState>(6, false, 0.0f, true), true);
                } else {
                    std::cout << "Saltando al siguiente nivel..." << std::endl;
                    this->state_machine->add_state(std::make_unique<MainGameState>(_currentLevel + 1), true);
                }
            }
        },
        [this]() { return ""; }
    });

    // 9. Resetear cheats
    cheatOptions_.push_back({
        "0. Resetear Todos los Cheats",
        [this]() {
            _godMode = false;
            // Evitar misleading-indentation: usar llaves en asignaciones a punteros
            if (_freezeEnemies) {
                *_freezeEnemies = false;
            }
            if (_infiniteTime) {
                *_infiniteTime = false;
            }
            _noClip = false;
            if (_registry && _registry->valid(_playerEntity)) {
                if (_registry->all_of<PlayerCheatComponent>(_playerEntity)) {
                    auto &cheats = _registry->get<PlayerCheatComponent>(_playerEntity);
                    cheats.godMode = false;
                    cheats.noClip = false;
                }
                if (_registry->all_of<PlayerStatsComponent>(_playerEntity)) {
                    auto &stats = _registry->get<PlayerStatsComponent>(_playerEntity);
                    stats.lives = 5;
                }
            }

            if (_keyGivenByCheating && *_keyGivenByCheating && _registry && _registry->valid(_playerEntity) &&
                _registry->all_of<PlayerStatsComponent>(_playerEntity)) {
                auto &stats = _registry->get<PlayerStatsComponent>(_playerEntity);
                stats.keysCollected = 0;
                *_keyGivenByCheating = false;
                std::cout << "Llave quitada (era de cheat)" << std::endl;
            }

            std::cout << "Todos los cheats reseteados" << std::endl;
        },
        [this]() { return ""; }
    });
}

void DevModeState::handleInput()
{
    Vector2 mousePos = GetMousePosition();

    if (_awaitingPassword)
    {
        // Capturar entrada de teclado
        int key = GetCharPressed();
        while (key > 0)
        {
            if ((key >= 32) && (key <= 125) && _passwordInput.length() < 20)
            {
                _passwordInput += (char)key;
            }
            key = GetCharPressed();
        }

        // Backspace
        if (IsKeyPressed(KEY_BACKSPACE) && _passwordInput.length() > 0)
        {
            _passwordInput.pop_back();
        }

        // Botones
        int menuX = GetScreenWidth() / 2 - 250;
        int menuY = 100;
        Rectangle confirmBtn = { (float)(menuX + 150), (float)(menuY + 240), 200.0f, 40.0f };
        Rectangle cancelBtn = { (float)(menuX + 150), (float)(menuY + 290), 200.0f, 40.0f };

        bool confirmClicked = CheckCollisionPointRec(mousePos, confirmBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        bool cancelClicked = CheckCollisionPointRec(mousePos, cancelBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        if (IsKeyPressed(KEY_ENTER) || confirmClicked)
        {
            if (_passwordInput == "developer")
            {
                _authenticated = true;
                _awaitingPassword = false;
                _selectedOption = -1;
                _passwordInput.clear();
                std::cout << "Developer Mode activado!" << std::endl;
            }
            else
            {
                _passwordInput.clear();
                std::cout << "Contraseña incorrecta" << std::endl;
            }
        }

        if (IsKeyPressed(KEY_ESCAPE) || cancelClicked)
        {
            // Cerrar el overlay
            this->state_machine->remove_overlay_state();
        }
    }
    else if (_authenticated)
    {
        // Menú de opciones
        int menuX = GetScreenWidth() / 2 - 250;
        int menuY = 100;

        // Detectar hover y clic en opciones
        for (int i = 0; i < static_cast<int>(cheatOptions_.size()); i++)
        {
            Rectangle optionRect = {
                (float)(menuX + 30),
                (float)(menuY + 100 + i * 38),
                440.0f,
                35.0f
            };

            if (CheckCollisionPointRec(mousePos, optionRect))
            {
                _selectedOption = i;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    cheatOptions_[i].action();
                }
            }
        }

        // Navegación por teclado
        if (IsKeyPressed(KEY_UP))
        {
            _selectedOption--;
            if (_selectedOption < 0) _selectedOption = static_cast<int>(cheatOptions_.size()) - 1;
        }
        if (IsKeyPressed(KEY_DOWN))
        {
            _selectedOption++;
            if (_selectedOption >= static_cast<int>(cheatOptions_.size())) _selectedOption = 0;
        }

        if (IsKeyPressed(KEY_ENTER) && _selectedOption >= 0 && _selectedOption < static_cast<int>(cheatOptions_.size()))
        {
            cheatOptions_[_selectedOption].action();
        }

        // Cerrar menú
        Rectangle closeBtn = { (float)(menuX + 150), (float)(menuY + 490), 200.0f, 25.0f };
        bool closeBtnClicked = CheckCollisionPointRec(mousePos, closeBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        if (IsKeyPressed(KEY_ESCAPE) || closeBtnClicked)
        {
            this->state_machine->remove_overlay_state();
        }
    }
}

void DevModeState::update(float /*deltaTime*/)
{
    // Este estado no actualiza lógica, solo maneja UI
}

bool DevModeState::_resolvePlayerEntity()
{
    if (!_registry){
        return false;
    }

    auto view = _registry->view<PlayerInputComponent>();

    if (view.empty()) {
        _playerEntity = entt::null;
        return false;
    }
    
    _playerEntity = *view.begin();
    return _registry->valid(_playerEntity);
}

void DevModeState::render()
{
    if (_awaitingPassword)
    {
        _renderPasswordScreen();
    }
    else if (_authenticated)
    {
        _renderDevMenu();
    }
}

void DevModeState::_renderPasswordScreen()
{
    Vector2 mousePos = GetMousePosition();

    // Fondo semitransparente
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.7f));

    int menuX = GetScreenWidth() / 2 - 250;
    int menuY = 100;
    int menuWidth = 500;
    int menuHeight = 400;

    // Panel del menú
    DrawRectangle(menuX, menuY, menuWidth, menuHeight, DARKGRAY);
    DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, GOLD);

    DrawText("DEVELOPER MODE", menuX + 100, menuY + 20, 30, GOLD);
    DrawText("Introduce la contrasena:", menuX + 80, menuY + 80, 20, WHITE);

    // Campo de contraseña
    char masked[32] = {0};
    for (size_t i = 0; i < _passwordInput.length() && i < 31; i++)
    {
        masked[i] = '*';
    }
    DrawRectangle(menuX + 50, menuY + 120, 400, 40, LIGHTGRAY);
    DrawRectangleLines(menuX + 50, menuY + 120, 400, 40, DARKGRAY);
    DrawText(masked, menuX + 60, menuY + 128, 25, BLACK);

    // Botones
    Rectangle confirmBtn = { (float)(menuX + 150), (float)(menuY + 240), 200.0f, 40.0f };
    Rectangle cancelBtn = { (float)(menuX + 150), (float)(menuY + 290), 200.0f, 40.0f };

    bool confirmHover = CheckCollisionPointRec(mousePos, confirmBtn);
    bool cancelHover = CheckCollisionPointRec(mousePos, cancelBtn);

    DrawRectangleRec(confirmBtn, confirmHover ? DARKGREEN : GREEN);
    DrawRectangleLinesEx(confirmBtn, 2.0f, BLACK);
    DrawText("CONFIRMAR", menuX + 175, menuY + 248, 20, WHITE);

    DrawRectangleRec(cancelBtn, cancelHover ? MAROON : RED);
    DrawRectangleLinesEx(cancelBtn, 2.0f, BLACK);
    DrawText("CANCELAR", menuX + 180, menuY + 298, 20, WHITE);

    DrawText("Usa teclado o haz clic en los botones", menuX + 90, menuY + 350, 16, LIGHTGRAY);
}

void DevModeState::_renderDevMenu()
{
    Vector2 mousePos = GetMousePosition();

    // Fondo semitransparente
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.7f));

    int menuX = GetScreenWidth() / 2 - 250;
    int menuY = 100;
    int menuWidth = 500;
    int menuHeight = 520;

    // Panel del menú
    DrawRectangle(menuX, menuY, menuWidth, menuHeight, DARKGRAY);
    DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, GOLD);

    DrawText("DEVELOPER MENU", menuX + 120, menuY + 20, 30, GOLD);
    DrawText("Haz clic o usa UP/DOWN y ENTER", menuX + 80, menuY + 60, 16, LIGHTGRAY);

    // Mostrar estado GOD global (cualquier cheat activo)
    bool isGodActive = _godMode || _noClip || (*_freezeEnemies) || (*_infiniteTime);
    if (isGodActive) {
        DrawRectangle(menuX + 10, menuY + 70, menuWidth - 20, 20, Fade(GOLD, 0.3f));
        DrawText("*** GOD MODE ACTIVO ***", menuX + 140, menuY + 73, 16, GOLD);
    }

    for (int i = 0; i < static_cast<int>(cheatOptions_.size()); i++)
    {
        Rectangle optionRect = {
            (float)(menuX + 30),
            (float)(menuY + 100 + i * 38),
            440.0f,
            35.0f
        };

        bool isHovered = CheckCollisionPointRec(mousePos, optionRect);
        bool isSelected = (i == _selectedOption);

        if (isHovered || isSelected)
        {
            DrawRectangleRec(optionRect, Fade(YELLOW, 0.2f));
        }

        Color textColor = (isHovered || isSelected) ? YELLOW : WHITE;
        Color stateColor = (isHovered || isSelected) ? GOLD : LIGHTGRAY;

        DrawText(cheatOptions_[i].label, menuX + 35, menuY + 102 + i * 38, 20, textColor);

        // Estado
        const char* state = cheatOptions_[i].getState();
        DrawText(state, menuX + 400, menuY + 102 + i * 38, 20, stateColor);
    }

    // Botón cerrar
    Rectangle closeBtn = { (float)(menuX + 150), (float)(menuY + 490), 200.0f, 25.0f };
    bool closeHover = CheckCollisionPointRec(mousePos, closeBtn);

    DrawRectangleRec(closeBtn, closeHover ? Fade(RED, 0.3f) : Fade(DARKGRAY, 0.3f));
    DrawRectangleLines(menuX + 150, menuY + 490, 200, 25, closeHover ? RED : LIGHTGRAY);
    DrawText("CERRAR (ESC)", menuX + 170, menuY + 493, 18, closeHover ? RED : LIGHTGRAY);
}
