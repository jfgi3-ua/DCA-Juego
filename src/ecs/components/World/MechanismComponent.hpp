#pragma once
#include "objects/Mechanism.hpp"

// Componente de Mecanismo (puertas, trampas, etc.)
struct MechanismComponent {
    Mechanism mechanism;

    explicit MechanismComponent(const Mechanism &mech) : mechanism(mech) {}
};