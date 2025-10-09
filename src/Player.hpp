#pragma once
#include "Map.hpp"
extern "C" {
  #include <raylib.h>
}

class Player {
    public:
        Player();
        
        void init(Vector2 startPos, float radius);
        void handleInput(float deltaTime, const Map& map);
        void update(float deltaTime, const Map& map);
        void render() const;

        // Posición actual del jugador 
        Vector2 position() const {return position_; }
        
    private:
        Vector2 position_ = {0, 0};
        float speed_ = 150.0f;
        float radius_ = 10.0f;
};