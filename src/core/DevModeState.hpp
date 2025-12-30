#pragma once
#include "GameState.hpp"
#include "Config.hpp"
#include <string>
#include <functional>
#include <vector>
#include <entt/entt.hpp>
class MainGameState; // Forward declaration

class DevModeState : public GameState
{
public:
    DevModeState(entt::registry* registry, float* levelTime, bool* freezeEnemies,
                 bool* infiniteTime, bool* keyGivenByCheating, int* totalKeysInMap,
                 int currentLevel);
    ~DevModeState();

    void init() override;
    void handleInput() override;
    void update(float deltaTime) override;
    void render() override;
    void pause() override {}
    void resume() override {}

private:
    entt::registry* _registry;
    entt::entity _playerEntity;
    float* _levelTime;
    bool* _freezeEnemies;
    bool* _infiniteTime;
    bool* _keyGivenByCheating;
    int* _totalKeysInMap;
    int _currentLevel;

    // UI State
    bool _awaitingPassword;
    std::string _passwordInput;
    int _selectedOption;
    bool _authenticated;

    // Cheat flags locales (sincronizados con Player)
    bool _godMode;
    bool _noClip;

    // Estructura para definir opciones de cheats
    struct CheatOption {
        const char* label;
        std::function<void()> action;
        std::function<const char*()> getState;
    };

    std::vector<CheatOption> cheatOptions_;

    // Helper methods
    void _renderPasswordScreen();
    void _renderDevMenu();
    void _setupCheatOptions();
    bool _resolvePlayerEntity();
};
