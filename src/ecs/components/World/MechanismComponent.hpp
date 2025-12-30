#pragma once
#include "objects/Mechanism.hpp"

/*
// Componente de Mecanismo (puertas, trampas, etc.)
struct MechanismComponent { 
    IVec2 triggerPos; 
    IVec2 targetPos; 
    MechanismType type; 
    bool active; 
    
    MechanismComponent(IVec2 trigger, IVec2 target, MechanismType mechType, bool isActive = true) 
    : triggerPos(trigger), targetPos(target), type(mechType), active(isActive) {} 
};
*/

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