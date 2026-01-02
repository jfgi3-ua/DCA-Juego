#pragma once
#include "Map.hpp"
extern "C" {
  #include <raylib.h>
}

class Player {
    public:
        Player();

        void init(Vector2 startPos, float radius, int lives, const std::string& sprite);
        void handleInput(const Map& map, const std::vector<Vector2>& blockedTiles);
        void update(float deltaTime, const Map& map, const std::vector<Vector2>& blockedTiles);
        
        // Dibujado con offset (para mapa centrado)
        void render(int ox, int oy) const; 

        /*
        *  Getters
        */

        // Posición actual del jugador
        Vector2 getPosition() const {return _position; }

        // Radio del jugador (para colisiones)
        float getRadius() const { return _radius; }

        int getLives() const { return _lives; }

        // Comprobar colisiones del jugador
        bool checkCollisionWithWalls(const Vector2& pos, const Map& map, const std::vector<Vector2>& blockedTiles) const;

        // Comprobar si está encima de la salida
        bool isOnExit(const Map& map) const;
        
        // Estado de mochila - Sistema de llaves
        int getKeyCount() const { return _key_count; }
        void addKey() { _key_count++; }
        void setKeyCount(int count) { _key_count = count; }
        bool hasAllKeys(int totalKeys) const { return _key_count >= totalKeys; }
        
        // Compatibilidad con código antiguo
        bool hasKey() const { return _key_count > 0; }

        // Gestionar vida al recibir daño
        void onHit(const Map& map);
        bool isInvulnerable() const;

        // ========== MÉTODOS PARA DEVELOPER MODE ==========
        void setGodMode(bool enabled) { _godMode = enabled; }
        void setNoClip(bool enabled) { _noClip = enabled; }
        void addLife() { if (_lives < 10) _lives++; }
        void setMaxLives() { _lives = 10; }
        void resetLives() { _lives = 5; }
        bool isGodMode() const { return _godMode; }
        bool isNoClip() const { return _noClip; }
        
        // Verificar si algún cheat está activo (para color GOD)
        bool hasAnyCheatsActive() const { return _godMode || _noClip; }
        
    private:
        // Posición
        Vector2 _position = {0, 0};
        Vector2 _lastMoveDir = {0, 0};

        // Velocidad
        float speed_ = 165.0f;

        // Tamaño
        float _radius = 10.0f;

        // Mochila - Sistema de llaves
        int _key_count = 0;

        // Colisiones y vidas
        int _lives = 5;

        // Temporizador dinámico que cambia cuando el jugador recibe daño
        // (va de 0.0f a INVULNERABLE_DURATION)
        float _invulnerableTimer = 0.0f;
        static constexpr float _INVULNERABLE_DURATION = 1.5f;

        // Movimiento por casillas
        bool _moving = false;
        Vector2 _move_start = {0,0};
        Vector2 _move_target = {0,0};
        
        // Segundos
        float _move_progress = 0.0f; 
        
        // Segundos por casilla (calculado en base a tile/speed)
        float _move_duration = 0.12f; 

        // Developer mode flags
        bool _godMode = false;
        bool _noClip = false;

        //sprites folder y texturas
        std::string _characterFolder; 
        const Texture2D* _idleTex = nullptr;   // player/X/Idle.png
        const Texture2D* _walkTex = nullptr;  // player/X/Walk.png
        static constexpr int _WALK_FRAMES = 8;

        //direccion del sprite
        bool _isFacingRight = true;
};