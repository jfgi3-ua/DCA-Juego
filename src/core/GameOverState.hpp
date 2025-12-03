#pragma once
#include "GameState.hpp"
#include <deque> 


extern "C" {
    #include <raylib.h>
}


class GameOverState : public GameState
{

    public:
        GameOverState(int score, bool die, float time, bool isVictory = false);
        ~GameOverState() override;

        void init() override;
        void handleInput() override;
        void update(float dt) override;
        void render() override;

        void pause() override {}
        void resume() override {}

    private:
        int currentLevel = 1;
        Color backgroundColor = GRAY;
        bool dead;
        float remainingTime;
        int selectedOption = 0;
        bool isVictory_ = false;  // true si es victoria total (completó todos los niveles)

        //para cuando tengamos sprites
        const Texture2D* titulo = nullptr;
        const Texture2D* background = nullptr;
};