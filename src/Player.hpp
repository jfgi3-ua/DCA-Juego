#pragma once
#include "Map.hpp"
extern "C" {
  #include <raylib.h>
}

class Player {
    public:
        Player();

        void init(Vector2 startPos, float radius, int lives);
        void handleInput(float deltaTime, const Map& map);
        void update(float deltaTime, const Map& map);
        void render(int ox, int oy) const; // Dibujado con offset (para mapa centrado)

        /*
        *  Getters
        */

        // Posición actual del jugador
        Vector2 getPosition() const {return position_; }

        // Radio del jugador (para colisiones)
        float getRadius() const { return radius_; }

        int getLives() const { return lives_; }

        // Comprobar colisiones del jugador
        bool checkCollisionWithWalls(const Vector2& pos, const Map& map) const;
        
        // Comprobar si está encima de la salida
        bool isOnExit(const Map& map) const;
        
        // Estado de mochila
        bool hasKey() const { return has_key_; }
        void setHasKey(bool v) { has_key_ = v; }

        // Gestionar vida al recibir daño
        void onHit(const Map& map);
        bool isInvulnerable() const;
        
    private:
        // Posición
        Vector2 position_ = {0, 0};
        Vector2 lastMoveDir_ = {0, 0};

        // Velocidad
        float speed_ = 150.0f;

        // Tamaño
        float radius_ = 10.0f;

        // Mochila (por ahora solo la llave)
        bool has_key_ = false;

        // Colisiones y vidas
        int lives_ = 5;

        // Temporizador dinámico que cambia cuando el jugador recibe daño
        // (va de 0.0f a INVULNERABLE_DURATION)
        float invulnerableTimer_ = 0.0f;
        static constexpr float INVULNERABLE_DURATION = 1.5f;
};