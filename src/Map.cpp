#include "Map.hpp"
#include <fstream>
#include <sstream>   // para construir mensajes de error detallados

/**
 * Carga un mapa ASCII desde archivo.
 * - Lee línea a línea y rellena _grid.
 * - Valida que el mapa sea no vacío y rectangular.
 * - Escanea 'P' y 'E' para inicializar spawns.
 *
 * Errores comunes gestionados:
 *  - Archivo inexistente/imposible de abrir → throw runtime_error.
 *  - Archivo vacío → throw runtime_error.
 *  - Filas con longitudes distintas → throw runtime_error.
 *  - Ausencia de 'P' → throw runtime_error.
 *
 * Detalle CRLF:
 *  - Si las líneas vienen con fin de línea Windows (\r\n), se elimina el '\r'
 *    final por compatibilidad.
 */
bool Map::loadFromFile(const std::string& path, int tileSize) {
    // Limpia estado previo por si se reutiliza la instancia.
    _grid.clear();
    _enemies.clear();
    _player = { -1, -1 };
    _keys.clear();

    // 1) Apertura del archivo
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open map: " + path);

    // 2) Lectura línea a línea (cada línea es una fila del grid)
    std::string line;
    while (std::getline(in, line)) {
        // Compatibilidad CRLF: elimina '\r' final si existe
        if (!line.empty() && line.back() == '\r') line.pop_back();
        _grid.push_back(line);
    }
    if (_grid.empty()) throw std::runtime_error("Empty map file");

    // 3) Validación de rectangularidad
    _h = static_cast<int>(_grid.size());
    _w = static_cast<int>(_grid[0].size());
    for (const auto& row : _grid) {
        if (static_cast<int>(row.size()) != _w)
            throw std::runtime_error("Non-rectangular map");
    }

    // 4) Aplica tamaño de tile (usado luego para render)
    _tile = tileSize;

    // 5) Escaneo para localizar 'P' (jugador) y 'E' (enemigos)
    //    Nota: si hay varias 'P', la última sobrescribiría a la anterior; el
    //    diseño actual asume exactamente una 'P'. En futuras iteraciones
    //    podemos comprobar y fallar si hay != 1.
    //   También localiza llaves 'K'.
    for (int y = 0; y < _h; ++y) {
        for (int x = 0; x < _w; ++x) {
            const char c = _grid[y][x];
            if (c == 'P') {
                _player = { x, y };
            } else if (c == 'E') {
                _enemies.push_back({ x, y });
            } else if (c == 'K') {
                _keys.push_back({ x, y });
            } else if (c == '^') {
                _spikes.push_back({x, y});
            }
        }
    }

    // 6) Post-condición: debe existir un spawn de jugador
    if (_player.x < 0) throw std::runtime_error("Missing player start 'P'");

    return true;
}

/**
 * Acceso seguro al contenido de la celda (x,y).
 * Lanza std::out_of_range si los índices están fuera del rango válido.
 *
 * @param x Columna (0 ≤ x < width()).
 * @param y Fila (0 ≤ y < height()).
 * @return Carácter ASCII que representa el tipo de celda.
 *
 * Ejemplo de retorno:
 *   '#' → pared
 *   '.' → suelo transitable
 *   'P' → jugador
 *   'E' → enemigo
 *   'K' → llave
 *   'X' → salida
 */
char Map::at(int x, int y) const {
    // 1) Validación de coordenadas
    if (x < 0 || y < 0 || x >= _w || y >= _h) {
        std::ostringstream oss;
        oss << "Map::at(" << x << ", " << y << ") fuera de rango "
            << "[width=" << _w << ", height=" << _h << "]";
        throw std::out_of_range(oss.str());
    }

    // 2) Retornar el carácter correspondiente
    return _grid[y][x];
}

/**
 * isWalkable
 *  - Devuelve false si (x,y) está fuera de rango o si es pared ('#').
 *  - Devuelve true para celdas transitables: '.', 'P', 'E', 'X', etc.
 *
 * @note Esta función incluye la comprobación de límites para evitar accesos fuera de rango.
 */
bool Map::isWalkable(int x, int y) const {
    // 1) Comprobación de límites (importante antes de indexar _grid)
    if (x < 0 || y < 0 || x >= _w || y >= _h) return false;

    // 2) Paredes no transitables
    return _grid[y][x] != '#';
}

/**
 * clearCell
 *  - Reemplaza la celda (x,y) por 'replacement' (por defecto, suelo '.').
 *  - Si la celda contenía una llave 'K', también la elimina del vector _keys.
 *
 * @param x Columna de la celda a modificar.
 * @param y Fila de la celda a modificar.
 * @param replacement Carácter que reemplaza el contenido actual de la celda.
 * @return true si la celda estaba dentro de rango y se actualizó; false en caso contrario.
 */
bool Map::clearCell(int x, int y, char replacement) {
    if (x < 0 || y < 0 || x >= _w || y >= _h) return false;
    char &cell = _grid[y][x];

    // Si había una 'K', retírala también del vector _keys
    if (cell == 'K') {
        auto it = std::remove_if(_keys.begin(), _keys.end(),
            [&](const IVec2& k){ return k.x == x && k.y == y; });
        _keys.erase(it, _keys.end());
    }

    cell = replacement;
    return true;
}