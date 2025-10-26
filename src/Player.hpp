#pragma once
#include "Map.hpp"
extern "C" {
  #include <raylib.h>
}

class Player {
    public:
        Player();

        void init(Vector2 startPos, float radius);
        void handleInput(float deltaTime, const Map& map, const std::vector<Vector2>& blockedTiles);
        void update(float deltaTime, const Map& map, const std::vector<Vector2>& blockedTiles);
        void render(int ox, int oy) const; // Dibujado con offset (para mapa centrado)

        // Posición actual del jugador
        Vector2 getPosition() const {return position_; }

        // Radio del jugador (para colisiones)
        float getRadius() const { return radius_; }

        // Comprobar colisiones del jugador
        bool checkCollisionWithWalls(const Vector2& pos, const Map& map, const std::vector<Vector2>& blockedTiles) const;

        // Comprobar si está encima de la salida
        bool isOnExit(const Map& map) const;

        // Estado de mochila
        bool hasKey() const { return has_key_; }
        void setHasKey(bool v) { has_key_ = v; }

    private:
        Vector2 position_ = {0, 0};
        float speed_ = 150.0f;
        float radius_ = 10.0f;

        // Mochila (por ahora solo la llave)
        bool has_key_ = false;
};