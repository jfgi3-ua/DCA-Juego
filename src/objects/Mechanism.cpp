#include "Mechanism.hpp"
#include <iostream>

Mechanism::Mechanism(char type, IVec2 trigger, IVec2 target) {

    auto& rm = ResourceManager::Get();
    std::string base = "sprites/mecs/";

    switch (type) {
        case 'D':
            type_ = MechanismType::DOOR;
            mecText_ = &rm.GetTexture(base + "doors_lever_chest_animation.png");
            srcInactive_ = {  64, 95, 32, 32 };   
            srcActive_   = {  0,  95, 32, 32 };
            break;
        case 'T':
            type_ = MechanismType::TRAP;
            mecText_ = &rm.GetTexture(base + "trap_saw.png");
            srcInactive_ = {  336,  192, 50, 30 };   
            srcActive_   = {  0,  26, 32, 32 };
            break;
        case 'B':
            type_ = MechanismType::BRIDGE;
            mecText_ = &rm.GetTexture(base + "fire_trap.png");
            srcInactive_ = {  170,  32, 32, 32 };   
            srcActive_   = {  0,  32, 32, 32 };
            break;
        case 'L':
            type_ = MechanismType::LEVER;
            mecText_ = &rm.GetTexture(base + "doors_lever_chest_animation.png");
            srcInactive_ = {  64,  64, 32, 32 };   
            srcActive_   = {  0,  64, 32, 32 };
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
    
    const Rectangle& src = active_ ? srcActive_ : srcInactive_;
    
    // Escala uniforme (misma en X e Y) para no deformar
    float scale = std::min(tileSize_ / src.width, tileSize_ / src.height);

    float destW = src.width  * scale;
    float destH = src.height * scale;

    Rectangle dest {
        gx,
        gy,
        destW,
        destH
    };

    DrawTexturePro(*mecText_, src, dest, {0,0}, 0.0f, WHITE);


    /** 
     int size = (tileSize_);
    //Este metodo habra q cambiarlo una cuendo usemos sprites
    Color inactiveColor;
    Color activeColor;

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
    */
    // Dibujar el activador (letra)
    if(active_){
        DrawText(TextFormat("%c", "?"), tx + TILE_SIZE / 4, ty + TILE_SIZE / 6, TILE_SIZE / 2, BLACK);
    }

    // Dibujar el objetivo (rectángulo)
    //DrawRectangle(gx, gy, size, size, currentColor);
}

void Mechanism::deactivate() { 
    active_ = false; 
    std::cout << "Mechanism at (" << triggerPos_.x << ", " << triggerPos_.y << ") deactivated." << std::endl;}