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
        Vector2 getPosition() const {return position_; }

        // Radio del jugador (para colisiones)
        float getRadius() const { return radius_; }

        // Comprobar colisiones del jugador
        bool checkCollisionWithWalls(const Vector2& pos, const Map& map) const;

        // Comprobar si está encima de la salida
        bool isOnExit(const Map& map) const;

    private:
        Vector2 position_ = {0, 0};
        float speed_ = 150.0f;
        float radius_ = 10.0f;
};