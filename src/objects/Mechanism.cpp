#include "Mechanism.hpp"
#include <iostream>

Mechanism::Mechanism(char type, IVec2 trigger, IVec2 target) {

    auto& rm = ResourceManager::Get();
    std::string base = "sprites/mecs/";

    switch (type) {
        case 'D':
            type_ = MechanismType::DOOR;
            mecText_ = &rm.GetTexture(base + "doors_lever_chest_animation.png");
            srcInactive_ = { 64, 95, 32, 32 };   
            srcActive_   = { 0, 95, 32, 32 };
            break;
        case 'T':
            type_ = MechanismType::TRAP;
            mecText_ = &rm.GetTexture(base + "trap_saw.png");
            srcInactive_ = { 336, 192, 50, 30 };   
            srcActive_   = { 0, 26, 32, 32 };
            break;
        case 'B':
            type_ = MechanismType::BRIDGE;
            mecText_ = &rm.GetTexture(base + "fire_trap.png");
            srcInactive_ = { 45, 128, 65, 65 };  
            srcActive_   = { 715, 128, 65, 65 };
            break;
        case 'L':
            type_ = MechanismType::LEVER;
            mecText_ = &rm.GetTexture(base + "doors_lever_chest_animation.png");
            srcInactive_ = { 64, 64, 32, 32 };   
            srcActive_   = { 0, 64, 32, 32 };
            break;
        default:
            std::cerr << "Warning: Unknown mechanism type '" << type << "'. Defaulting to DOOR." << std::endl;
            type_ = MechanismType::DOOR;
            break;
    }
    
    trigerText_ = &rm.GetTexture(base + "doors_lever_chest_animation.png");
    //posiciones y tamaños de los sprites de los triggers (activadores) en la textura
    triggerInactive_ = { 62, 174, 18, 18 };
    triggerActive_   = { 30, 174, 18, 18 };
    
    triggerPos_ = trigger;
    targetPos_ = target;
}

void Mechanism::update() {}

void Mechanism::render(int ox, int oy) const {
    //Textura y región según estado
    const Rectangle& srcTarget = active_ ? srcActive_ : srcInactive_;
    const Rectangle& srcTriger = active_ ? triggerActive_ : triggerInactive_;
    
    //Calcular posiciones en píxeles
    float tx = (triggerPos_.x * tileSize_) + ox;
    float ty = (triggerPos_.y * tileSize_) + oy;
    float gx = (targetPos_.x * tileSize_) + ox;
    float gy = (targetPos_.y * tileSize_) + oy;
    
    //TARGET
    // Escala uniforme (misma en X e Y) para no deformar
    float scale = std::min(tileSize_ / srcTarget.width, tileSize_ / srcTarget.height);
    
    float destW = srcTarget.width  * scale;
    float destH = srcTarget.height * scale;
    
    Rectangle dest { gx, gy, destW, destH};
    DrawTexturePro(*mecText_, srcTarget, dest, {0,0}, 0.0f, WHITE);
    
    //TRIGER
    Rectangle destTar { tx, ty, tileSize_, tileSize_};
    DrawTexturePro(*trigerText_, srcTriger, destTar, {0,0}, 0.0f, WHITE);
}

void Mechanism::deactivate() { 
    active_ = false; 
    std::cout << "Mechanism at (" << triggerPos_.x << ", " << triggerPos_.y << ") deactivated." << std::endl;}