#include "Mechanism.hpp"
#include <iostream>

Mechanism::Mechanism(MechanismType type, Vector2 trigger, Vector2 target)
    : type_(type), triggerPos_(trigger), targetPos_(target){}

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
            inactiveColor = {150, 100, 180, 255}; // violeta apagado
            activeColor   = {200, 120, 255, 255}; // violeta brillante
            break;

        case MechanismType::TRAP:
            symbol = 'T';
            inactiveColor = {120, 200, 150, 255}; // verde azulado apagado
            activeColor   = {0, 255, 128, 255};   // verde intenso
            break;

        case MechanismType::BRIDGE:
            symbol = 'B';
            inactiveColor = {255, 140, 0, 255};   // naranja oscuro
            activeColor   = {255, 180, 70, 255};  // naranja claro
            break;

        case MechanismType::LEVER:
            symbol = 'U';
            inactiveColor = {200, 80, 130, 255};  // magenta oscuro
            activeColor   = {255, 105, 180, 255}; // rosa fuerte
            break;

        default:
            inactiveColor = {120, 120, 120, 255};
            activeColor   = {180, 180, 180, 255};
            symbol = '?';
            break;
    }

    Color currentColor = active_ ? activeColor : inactiveColor;

    // Dibujar el activador (letra)
    DrawText(TextFormat("%c", symbol), tx + size / 4, ty + size / 6, size / 2, currentColor);

    // Dibujar el objetivo (rectángulo)
    DrawRectangle(gx, gy, size, size, currentColor);
}

bool Mechanism::isBlockingAt(int gridX, int gridY) const {
    int tx = (targetPos_.x);
    int ty = (targetPos_.y);

    if (type_ == MechanismType::DOOR && !active_ &&
        gridX == tx && gridY == ty) {
        return true; // puerta cerrada bloquea
    }
    return false;
}
