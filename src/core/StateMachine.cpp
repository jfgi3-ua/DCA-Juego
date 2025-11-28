#include "StateMachine.hpp"
#include <iostream>

StateMachine::StateMachine()
{
    this->new_state = nullptr;
    this->overlay_state = nullptr;
    this->new_overlay_state = nullptr;
}

void StateMachine::add_state(std::unique_ptr<GameState> newState, bool is_replacing)
{
    is_Adding = true;
    this->is_Replacing = is_replacing;
    // El nuevo estado se almacena en la variable miembro new_state usando std::move, lo que transfiere la propiedad del puntero único al objeto StateMachine. De este modo, el nuevo estado queda listo para ser gestionado cuando la máquina procese los cambios de estado.
    this->new_state = std::move(newState);
}

void StateMachine::add_overlay_state(std::unique_ptr<GameState> newState)
{
    is_adding_overlay = true;
    this->new_overlay_state = std::move(newState);
}

void StateMachine::remove_overlay_state()
{
    is_removing_overlay = true;
}

void StateMachine::remove_state(bool value)
{
    is_removing = true;
    is_ending = value; // Indica si la eliminación del estado también implica que el juego debe terminar.
}

//  deltaTime representa el tiempo transcurrido desde el último fotograma, y que puede ser reiniciado durante los cambios de estado para evitar inconsistencias en la lógica del juego
void StateMachine::handle_state_changes(float &deltaTime)
{
    // Manejar overlay states
    if (this->is_removing_overlay && this->overlay_state)
    {
        this->overlay_state.reset();
        this->is_removing_overlay = false;
    }
    
    if (this->is_adding_overlay)
    {
        if (this->new_overlay_state)
        {
            this->new_overlay_state->setStateMachine(this);
            this->overlay_state = std::move(this->new_overlay_state);
            this->overlay_state->init();
        }
        this->is_adding_overlay = false;
    }
    
    // Manejar estados normales
    if (this->is_removing && !this->states_machine.empty())
    {
        this->states_machine.pop();
        this->is_removing = false;

        // Protección adicional: si no hay estados en la pila después de eliminar uno, se detiene la máquina de estados.
        if (!this->is_Adding && !this->states_machine.empty())
        {
            this->states_machine.top()->resume();
            deltaTime = 0.0f;
        }
    }

    if (this->is_Adding)
    {
        if (!this->states_machine.empty() && this->is_Replacing)
            this->states_machine.pop();

        // Asociar la máquina de estados al nuevo estado antes de inicializarlo
        if (this->new_state)
            this->new_state->setStateMachine(this);

        this->states_machine.push(std::move(this->new_state));
        this->states_machine.top()->init();
        this->is_Adding = false;
        this->is_Replacing = false;
        deltaTime = 0.0f;
    }
}