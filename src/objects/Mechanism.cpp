#include "Mechanism.hpp"
#include <iostream>

Mechanism::Mechanism(char type, IVec2 trigger, IVec2 target) {

    auto& rm = ResourceManager::Get();
    std::string base = "sprites/mecs/";

    switch (type) {
        case 'D':
            _type = MechanismType::DOOR;
            _mecText = &rm.GetTexture(base + "doors_lever_chest_animation.png");
            _srcInactive = { 64, 95, 32, 32 };   
            _srcActive   = { 0, 95, 32, 32 };
            break;
        case 'T':
            _type = MechanismType::TRAP;
            _mecText = &rm.GetTexture(base + "trap_saw.png");
            _srcInactive = { 336, 192, 50, 30 };   
            _srcActive   = { 0, 26, 32, 32 };
            break;
        case 'B':
            _type = MechanismType::BRIDGE;
            _mecText = &rm.GetTexture(base + "fire_trap.png");
            _srcInactive = { 45, 128, 65, 65 };  
            _srcActive   = { 715, 128, 65, 65 };
            break;
        case 'L':
            _type = MechanismType::LEVER;
            _mecText = &rm.GetTexture(base + "doors_lever_chest_animation.png");
            _srcInactive = { 64, 64, 32, 32 };   
            _srcActive   = { 0, 64, 32, 32 };
            break;
        default:
            std::cerr << "Warning: Unknown mechanism type '" << type << "'. Defaulting to DOOR." << std::endl;
            _type = MechanismType::DOOR;
            break;
    }
    
    _trigerText = &rm.GetTexture(base + "doors_lever_chest_animation.png");
    //posiciones y tamaños de los sprites de los triggers (activadores) en la textura
    _triggerInactive = { 62, 174, 18, 18 };
    _triggerActive   = { 30, 174, 18, 18 };
    
    _triggerPos = trigger;
    _targetPos = target;
}

void Mechanism::update() {}

void Mechanism::render(int ox, int oy) const {
    //Textura y región según estado
    const Rectangle& srcTarget = _active ? _srcActive : _srcInactive;
    const Rectangle& srcTriger = _active ? _triggerActive : _triggerInactive;
    
    //Calcular posiciones en píxeles
    float tx = (_triggerPos.x * _tileSize) + ox;
    float ty = (_triggerPos.y * _tileSize) + oy;
    float gx = (_targetPos.x * _tileSize) + ox;
    float gy = (_targetPos.y * _tileSize) + oy;
    
    //TARGET
    // Escala uniforme (misma en X e Y) para no deformar
    float scale = std::min(_tileSize / srcTarget.width, _tileSize / srcTarget.height);
    
    float destW = srcTarget.width  * scale;
    float destH = srcTarget.height * scale;
    
    Rectangle dest { gx, gy, destW, destH};
    DrawTexturePro(*_mecText, srcTarget, dest, {0,0}, 0.0f, WHITE);
    
    //TRIGER
    Rectangle destTar { tx, ty, _tileSize, _tileSize};
    DrawTexturePro(*_trigerText, srcTriger, destTar, {0,0}, 0.0f, WHITE);
}

void Mechanism::deactivate() { 
    _active = false; 
    std::cout << "Mechanism at (" << _triggerPos.x << ", " << _triggerPos.y << ") deactivated." << std::endl;}