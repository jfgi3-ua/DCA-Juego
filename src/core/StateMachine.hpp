#pragma once
#include "GameState.hpp"
#include <stack>
#include <memory>
#include <unordered_map>
#include <string>

class StateMachine
{
    public:
        StateMachine();
        ~StateMachine() = default;

        void add_state(std::unique_ptr<GameState> state, bool is_replacing);
        void add_overlay_state(std::unique_ptr<GameState> state);
        void remove_overlay_state();
        void remove_state(bool value);
        void handle_state_changes(float& deltaTime);

        void stop() {is_running = false;}
        bool isRunning() {return this->is_running;}

        bool is_game_ending() {return this->is_ending;}
        void set_game_ending(bool value) {this->is_ending = value;}

        std::unique_ptr<GameState>& getCurrentState() {return this->states_machine.top();}
        GameState* getOverlayState() {return overlay_state.get();}
        bool hasOverlay() const {return overlay_state != nullptr;}
        
    private:
        // pila (std::stack) de punteros inteligentes únicos (std::unique_ptr<GameState>) para almacenar los estados
        // activos, permite cambiar entre ellos de manera eficiente y segura en cuanto a gestión de memoria.
        std::stack<std::unique_ptr<GameState>> states_machine;
        std::unique_ptr<GameState> new_state;
        std::unique_ptr<GameState> overlay_state; // Estado superpuesto (como dev mode)
        std::unique_ptr<GameState> new_overlay_state;
        bool is_running;

        bool is_removing = false,
             is_Adding = false,
             is_Replacing = false,
             is_ending = false,
             is_adding_overlay = false,
             is_removing_overlay = false;
};