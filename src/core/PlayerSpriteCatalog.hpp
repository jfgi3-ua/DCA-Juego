#pragma once
#include <string>
#include <vector>

// Descripción simple de un set de sprites del jugador.
struct PlayerSpriteSet {
    std::string id;        // Nombre de la carpeta (p.ej. "Archer")
    std::string idlePath;  // Ruta relativa dentro de assets
    std::string walkPath;  // Ruta relativa dentro de assets
    bool hasIdle = false;
    bool hasWalk = false;
};

// Descubre subcarpetas en assets/sprites/player y busca Idle.png / Walk.png.
std::vector<PlayerSpriteSet> DiscoverPlayerSpriteSets(
    const std::string& baseRelativePath = "sprites/player");

// Resuelve el id por defecto; si no existe, devuelve el primer set válido.
std::string ResolveDefaultPlayerSpriteSetId(
    const std::vector<PlayerSpriteSet>& sets,
    const std::string& preferredId = "Archer");

// Log auxiliar para validar el descubrimiento durante el hito 1.
void LogPlayerSpriteSets(
    const std::vector<PlayerSpriteSet>& sets,
    const std::string& defaultId);
