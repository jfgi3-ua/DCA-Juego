#pragma once
#include <string>

// Almacenamiento en memoria de la selección del jugador (sesión actual).
// Usamos namespace para evitar variables globales expuestas y agrupar la API
// sin forzar instancias ni singletons; es simple y suficiente para este flujo.
namespace PlayerSelection {
    void SetSelectedSpriteSet(const std::string& spriteId,
                              const std::string& idlePath,
                              const std::string& walkPath,
                              bool hasIdle,
                              bool hasWalk);
    bool HasSelectedSpriteSet();
    const std::string& GetSelectedSpriteId();
    const std::string& GetSelectedIdlePath();
    const std::string& GetSelectedWalkPath();
    bool SelectedHasIdle();
    bool SelectedHasWalk();
    void ClearSelectedSprite();
}
