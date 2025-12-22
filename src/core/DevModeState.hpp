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
    entt::registry* registry_;
    entt::entity playerEntity_;
    float* levelTime_;
    bool* freezeEnemies_;
    bool* infiniteTime_;
    bool* keyGivenByCheating_;
    int* totalKeysInMap_;
    int currentLevel_;

    // UI State
    bool awaitingPassword_;
    std::string passwordInput_;
    int selectedOption_;
    bool authenticated_;

    // Cheat flags locales (sincronizados con Player)
    bool godMode_;
    bool noClip_;

    // Estructura para definir opciones de cheats
    struct CheatOption {
        const char* label;
        std::function<void()> action;
        std::function<const char*()> getState;
    };

    std::vector<CheatOption> cheatOptions_;

    // Helper methods
    void renderPasswordScreen();
    void renderDevMenu();
    void setupCheatOptions();
    bool resolvePlayerEntity();
};
