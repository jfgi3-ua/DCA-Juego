#pragma once
#include "GameState.hpp"
#include "MainGameState.hpp"

extern "C" {
    #include <raylib.h>
}

class StartGameState : public GameState {
    public:
        StartGameState();
        ~StartGameState() override;

        void init() override;
        void handleInput() override;
        void update(float dt) override;
        void render() override;
        void pause() override {}
        void resume() override {}

    private:
        int _selectedOption = 0;               // 0 = Jugar, 1 = Salir
};