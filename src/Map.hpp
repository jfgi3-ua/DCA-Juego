#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>

/**
 * Coordenada entera en el grid del mapa (no en píxeles).
 * x = columna, y = fila.
 */
struct IVec2 { int x{0}, y{0}; };

/**
 * Clase Map
 *  - Carga un mapa ASCII desde archivo.
 *  - Mantiene la rejilla (grid) como vector de strings.
 *  - Expone utilidades para consultar celdas y posiciones iniciales.
 *
 * Formato del mapa (caracteres):
 *   '#' = pared (no transitable)
 *   '.' = camino (transitable)
 *   'P' = posición inicial del jugador (exactamente una)
 *   'E' = posiciones iniciales de enemigos (cero o varias)
 *   'X' = salida (puede haber 0 o 1; en próximas iteraciones podemos validar su existencia)
 *
 * Invariantes tras loadFromFile():
 *  - El mapa es rectangular: todas las filas tienen el mismo ancho.
 *  - Existe exactamente una 'P'; si no, lanza std::runtime_error.
 *  - _w y _h reflejan el tamaño en celdas (ancho x alto).
 *  - _tile contiene el tamaño de celda en píxeles (para dibujado).
 */
class Map {
    public:
        /**
         * Carga el mapa desde un archivo de texto.
         * @param path Ruta al archivo (UTF-8/ASCII). Se tolera fin de línea CRLF (tipo de salto de linea que usan los sistemas Windows -(CR) Carriage return; (LF) Line feed).
         * @param tileSize Tamaño del tile en píxeles (útil para render).
         * @throws std::runtime_error si no se puede abrir, si está vacío,
         *         si no es rectangular o si no hay 'P'.
         */
        bool loadFromFile(const std::string& path, int tileSize = 32);

        /// Dimensiones del mapa en celdas (grid), no en píxeles.
        int width()  const { return _w; }
        int height() const { return _h; }

        /// Tamaño del tile en píxeles (para convertir de celda → pantalla).
        int tile()   const { return _tile; }

        /**
         * Devuelve el carácter de la celda (x,y) con comprobación de rango.
         * @throws std::out_of_range si los índices están fuera de los límites del mapa.
         */
        char at(int x, int y) const;

        /**
         * Devuelve true si la celda (x,y) es transitable.
         *  - Fuera de rango o paredes ('#') → false.
         */
        bool isWalkable(int x, int y) const;

        /// Posición inicial del jugador (en celdas). Garantizado tras loadFromFile().
        IVec2 playerStart() const { return _player; }

        // Llaves detectadas en el mapa
        const std::vector<IVec2>& keyPositions() const { return _keys; }

        // “Retirar” un elemento del mapa (ej. llave)
        // Reemplaza la celda (x,y) por 'replacement' (por defecto, suelo '.').
        // Devuelve true si la celda estaba dentro de rango y se actualizó.
        bool clearCell(int x, int y, char replacement='.');

        /// Posiciones iniciales de enemigos (en celdas). Puede estar vacío.
        const std::vector<IVec2>& enemyStarts() const { return _enemies; }

        /// Acceso de solo lectura al grid completo (útil para debug o validaciones).
        const std::vector<std::string>& grid() const { return _grid; }

    private:
        // Dimensiones en celdas
        int _w = 0, _h = 0;

        // Tamaño del tile en píxeles (para render); no afecta a la lógica.
        int _tile = 32;

        // Rejilla de caracteres: cada string es una fila; grid[y][x] es la celda.
        std::vector<std::string> _grid;

        // Spawns detectados al cargar:
        IVec2 _player{ -1, -1 };
        std::vector<IVec2> _enemies;

        // Lista de llaves 'K' detectadas al cargar
        std::vector<IVec2> _keys;
};
