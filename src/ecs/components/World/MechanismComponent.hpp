#pragma once
#include "objects/Mechanism.hpp"

//  - Cada mecanismo se representa con 2 entidades distintas
//  - Ambas comparten un mismo id lógico (MechanismId)
//  - El trigger y el target "saben" q están conectados
//    simplemente porque tienen el mismo id
//
// El sistema de mecanismos se encarga de:
//  - detectar cuándo el jugador pisa un trigger
//  - desactivar el mecanismo (active = false)
//  - el target reacciona a ese estado (puerta abierta, etc.)

// Id lógico que comparten trigger y target
using MechanismId = int;


// Estado común del mecanismo.
// Lo llevan tanto el trigger como el target.
struct MechanismComponent {
    MechanismId id;
    MechanismType type;
    bool active;

    MechanismComponent(MechanismId mechId, MechanismType mechType, bool isActive = true)
        : id(mechId), type(mechType), active(isActive) {}
};


// Marca de entidad TRIGGER (palanca / botón)
struct MechanismTriggerComponent {
    MechanismId id;

    explicit MechanismTriggerComponent(MechanismId mechId)
        : id(mechId) {}
};


// Marca de entidad TARGET (puerta / trampa / puente)
struct MechanismTargetComponent {
    MechanismId id;

    explicit MechanismTargetComponent(MechanismId mechId)
        : id(mechId) {}
};