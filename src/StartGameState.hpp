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
        Texture2D background;
        Color backgroundColor = GRAY;

        int selectedOption = 0;               // 0 = Jugar, 1 = Salir

};