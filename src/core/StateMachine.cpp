#include "StateMachine.hpp"
#include <iostream>

StateMachine::StateMachine()
{
    this->_new_state = nullptr;
    this->_overlay_state = nullptr;
    this->_new_overlay_state = nullptr;
}

void StateMachine::add_state(std::unique_ptr<GameState> newState, bool is_replacing)
{
    _is_Adding = true;
    this->_is_Replacing = is_replacing;
    // El nuevo estado se almacena en la variable miembro _new_state usando std::move, lo que transfiere la propiedad del puntero único al objeto StateMachine. De este modo, el nuevo estado queda listo para ser gestionado cuando la máquina procese los cambios de estado.
    this->_new_state = std::move(newState);
}

void StateMachine::add_overlay_state(std::unique_ptr<GameState> newState)
{
    _is_adding_overlay = true;
    this->_new_overlay_state = std::move(newState);
}

void StateMachine::remove_overlay_state()
{
    _is_removing_overlay = true;
}

void StateMachine::remove_state(bool value)
{
    _is_removing = true;
    _is_ending = value; // Indica si la eliminación del estado también implica que el juego debe terminar.
}

//  deltaTime representa el tiempo transcurrido desde el último fotograma, y que puede ser reiniciado durante los cambios de estado para evitar inconsistencias en la lógica del juego
void StateMachine::handle_state_changes(float &deltaTime)
{
    // Manejar overlay states
    if (this->_is_removing_overlay && this->_overlay_state)
    {
        this->_overlay_state.reset();
        this->_is_removing_overlay = false;
    }
    
    if (this->_is_adding_overlay)
    {
        if (this->_new_overlay_state)
        {
            this->_new_overlay_state->setStateMachine(this);
            this->_overlay_state = std::move(this->_new_overlay_state);
            this->_overlay_state->init();
        }
        this->_is_adding_overlay = false;
    }
    
    // Manejar estados normales
    if (this->_is_removing && !this->_states_machine.empty())
    {
        this->_states_machine.pop();
        this->_is_removing = false;

        // Protección adicional: si no hay estados en la pila después de eliminar uno, se detiene la máquina de estados.
        if (!this->_is_Adding && !this->_states_machine.empty())
        {
            this->_states_machine.top()->resume();
            deltaTime = 0.0f;
        }
    }

    if (this->_is_Adding)
    {
        if (!this->_states_machine.empty() && this->_is_Replacing)
            this->_states_machine.pop();

        // Asociar la máquina de estados al nuevo estado antes de inicializarlo
        if (this->_new_state)
            this->_new_state->setStateMachine(this);

        this->_states_machine.push(std::move(this->_new_state));
        this->_states_machine.top()->init();
        this->_is_Adding = false;
        this->_is_Replacing = false;
        deltaTime = 0.0f;
    }
}