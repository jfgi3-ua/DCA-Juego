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

        void stop() {_is_running = false;}
        bool isRunning() {return this->_is_running;}

        bool is_game_ending() {return this->_is_ending;}
        void set_game_ending(bool value) {this->_is_ending = value;}

        std::unique_ptr<GameState>& getCurrentState() {return this->_states_machine.top();}
        GameState* getOverlayState() {return _overlay_state.get();}
        bool hasOverlay() const {return _overlay_state != nullptr;}
        
    private:
        // pila (std::stack) de punteros inteligentes únicos (std::unique_ptr<GameState>) para almacenar los estados
        // activos, permite cambiar entre ellos de manera eficiente y segura en cuanto a gestión de memoria.
        std::stack<std::unique_ptr<GameState>> _states_machine;
        std::unique_ptr<GameState> _new_state;
        std::unique_ptr<GameState> _overlay_state; // Estado superpuesto (como dev mode)
        std::unique_ptr<GameState> _new_overlay_state;
        bool _is_running;

        bool _is_removing = false,
             _is_Adding = false,
             _is_Replacing = false,
             _is_ending = false,
             _is_adding_overlay = false,
             _is_removing_overlay = false;
};