#pragma once
#include "GameState.hpp"
#include <deque> 


extern "C" {
    #include <raylib.h>
}


class GameOverState : public GameState
{

    public:
        GameOverState(int score, bool die, float time);
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

        //para cuando tengamos sprites
        Texture2D gameOverText;
        Texture2D levelCompleteText;
        Texture2D background;
};