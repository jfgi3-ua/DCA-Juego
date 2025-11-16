#pragma once

inline constexpr int TILE_SIZE  = 32; // Tamaño de tile global
inline constexpr int HUD_HEIGHT = 80; // Altura de la franja de HUD inferior
inline constexpr int WINDOW_WIDTH  = 1280; // Anchura de la ventana
inline constexpr int WINDOW_HEIGHT = 800;  // Altura de la ventana

/**
 * Coordenada entera en el grid del mapa (no en píxeles).
 * x = columna, y = fila.
 */
 struct IVec2 { int x{0}, y{0}; };

/**
 * Pareja de mecanismo
 * id = char que identifica el mecanismo en el mapa
 * trigger = posicion del trigger
 * target = posicion del target
 */
struct MechanismPair {
    char id;
    IVec2 trigger;
    IVec2 target;
};


