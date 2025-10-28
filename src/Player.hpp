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
        
        // Dibujado con offset (para mapa centrado)
        void render(int ox, int oy) const; 

        // Posición actual del jugador
        Vector2 getPosition() const {return position_; }

        // Radio del jugador (para colisiones)
        float getRadius() const { return radius_; }

        // Comprobar colisiones del jugador
        bool checkCollisionWithWalls(const Vector2& pos, const Map& map) const;

        // Comprobar si está encima de la salida
        bool isOnExit(const Map& map) const;

        // Estado de mochila
        bool hasKey() const { return has_key_; }
        void setHasKey(bool v) { has_key_ = v; }

    private:
        Vector2 position_ = {0, 0};
        float radius_ = 10.0f;

        // Mochila (por ahora solo la llave)
        bool has_key_ = false;

        // Movimiento por casillas
        bool moving_ = false;
        Vector2 move_start_ = {0,0};
        Vector2 move_target_ = {0,0};
        
        // Segundos
        float move_progress_ = 0.0f; 
        
        // Segundos por casilla (calculado en base a tile/speed)
        float move_duration_ = 0.12f; 
};