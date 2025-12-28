#pragma once
#include <string>

// Almacenamiento en memoria de la selección del jugador (sesión actual).
// Usamos namespace para evitar variables globales expuestas y agrupar la API
// sin forzar instancias ni singletons.
namespace PlayerSelection {
    void SetSelectedSpriteId(const std::string& spriteId);
    const std::string& GetSelectedSpriteId();
    void ClearSelectedSpriteId();
}
