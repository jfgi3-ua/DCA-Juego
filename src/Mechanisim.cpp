#include "Mechanism.hpp"
#include <iostream>

Mechanism::Mechanism(char type, IVec2 trigger, IVec2 target) {
    switch (type) {
        case 'D':
            type_ = MechanismType::DOOR;
            break;
        case 'T':
            type_ = MechanismType::TRAP;
            break;
        case 'B':
            type_ = MechanismType::BRIDGE;
            break;
        case 'L':
            type_ = MechanismType::LEVER;
            break;
        default:
            std::cerr << "Warning: Unknown mechanism type '" << type << "'. Defaulting to DOOR." << std::endl;
            type_ = MechanismType::DOOR;
            break;
    }
    
    triggerPos_ = trigger;
    targetPos_ = target;
}

void Mechanism::update() {}

void Mechanism::render(int ox, int oy) const {
    int tx = (triggerPos_.x * tileSize_) + ox;
    int ty = (triggerPos_.y * tileSize_) + oy;
    int gx = (targetPos_.x * tileSize_) + ox;
    int gy = (targetPos_.y * tileSize_) + oy;
    int size = (tileSize_);

    //Este metodo habra q cambiarlo una cuendo usemos sprites
    Color inactiveColor;
    Color activeColor;
    char symbol = '?';

    switch (type_) {
        case MechanismType::DOOR:
            symbol = 'D';
            inactiveColor = {230, 210, 255, 255}; // muy claro
            activeColor   = {90, 0, 140, 255};    // muy oscuro
            break;

        case MechanismType::TRAP:
            symbol = 'T';
            inactiveColor = {200, 255, 220, 255}; // muy claro
            activeColor   = {0, 90, 40, 255};     // muy oscuro
            break;

        case MechanismType::BRIDGE:
            symbol = 'B';
            inactiveColor = {255, 220, 180, 255}; // muy claro
            activeColor   = {120, 60, 0, 255};    // muy oscuro
            break;

        case MechanismType::LEVER:
            symbol = 'L';
            inactiveColor = {255, 220, 240, 255}; // muy claro
            activeColor   = {120, 0, 60, 255};    // muy oscuro
            break;

        default:
            inactiveColor = {180, 180, 180, 255};
            activeColor   = {60, 60, 60, 255};
            symbol = '?';
            break;
    }

    Color currentColor = active_ ? activeColor : inactiveColor;

    // Dibujar el activador (letra)
    DrawText(TextFormat("%c", symbol), tx + size / 4, ty + size / 6, size / 2, currentColor);

    // Dibujar el objetivo (rectángulo)
    DrawRectangle(gx, gy, size, size, currentColor);
}

void Mechanism::deactivate() { 
    active_ = false; 
    std::cout << "Mechanism at (" << triggerPos_.x << ", " << triggerPos_.y << ") deactivated." << std::endl;}