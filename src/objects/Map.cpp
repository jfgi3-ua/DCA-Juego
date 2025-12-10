#include "Map.hpp"
#include <fstream>
#include <sstream>   // para construir mensajes de error detallados
extern "C" {
    #include <raylib.h>
}
#include <iostream>

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
    _spikes.clear();

    // 1) Apertura del archivo
    std::ifstream in(path);
    std::cout << "Cargando mapa desde: " << path << std::endl;
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
    //   También localiza llaves 'K', Spikes '^' y mecanismos 'D','T','B', 'U'

    //guardamos las posiciones de los mecanismos y el caracter que los identifica para juntarlos despues
    std::unordered_map<char, IVec2> triggers;
    std::unordered_map<char, IVec2> targets;

    for (int y = 0; y < _h; ++y) {
        for (int x = 0; x < _w; ++x) {
            const char c = _grid[y][x];
            if (c == 'P') {
                _player = { x, y };
            } else if (c == 'E') {
                _enemies.push_back({ x, y });
            } else if (c == 'K') {
                _keys.push_back({ x, y });
            } else if (c == 'X') {
                // Salida, no se necesita almacenar posición aquí?
            } else if (c == '^') {
                _spikes.push_back({x, y});
            } else if (c == 'X') {
                // Salida, no se necesita almacenar posición aquí?
            }else if (std::islower(c)) {
                triggers[c] = {x, y};
            } else if (std::isupper(c)) {
                targets[c] = {x, y};
            }
        }
    }

    // 6) Post-condición: debe existir un spawn de jugador
    if (_player.x < 0) throw std::runtime_error("Missing player start 'P'");

    //7 Mecanismos: emparejamos triggers y targets
    pairMechanisms(triggers, targets);

    //8 cargamos texturas SE HACE EN MAINGAMESTATE

    return true;
}

void Map::loadTextures() {
    auto& rm = ResourceManager::Get();
    
    _mapTexture = &rm.GetTexture("sprites/walls_floor.png");
    _floorSrc = { 176, 340, 32, 28 };
    _wallSrc  = { 10, 47, 32, 30 };
    _exitSrc  = { 48, 336, 32, 32 };  
    
    _keyTexture = &rm.GetTexture("sprites/icons/Icons.png");
    _keySrc = { 64, 0, 16, 16 }; 

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
 * isWalkableForEnemy
 *  - Versión específica para enemigos con restricciones adicionales.
 *  - Devuelve false si (x,y) está fuera de rango o si es pared ('#').
 *  - Devuelve false para salidas ('X') - los enemigos no pueden escapar.
 *  - Devuelve false para mecanismos en mayúsculas (puertas bloqueadas: 'D', 'B', etc.).
 *  - Devuelve true para mecanismos en minúsculas (botones/triggers: 'd', 'b', etc.) - pueden atravesarlos.
 *  - Devuelve true para celdas transitables: '.', 'P', 'E', 'K', etc.
 *
 * @note Esta función incluye la comprobación de límites para evitar accesos fuera de rango.
 */
bool Map::isWalkableForEnemy(int x, int y) const {
    // 1) Comprobación de límites (importante antes de indexar _grid)
    if (x < 0 || y < 0 || x >= _w || y >= _h) return false;

    char cell = _grid[y][x];

    // 2) Paredes no transitables
    if (cell == '#') return false;

    // 3) Salida no transitable para enemigos
    if (cell == 'X') return false;

    // 4) Mecanismos en mayúsculas (puertas) no transitables
    if (std::isupper(cell) && cell != 'P' && cell != 'E' && cell != 'K' && cell != 'X') {
        return false;
    }

    // 5) Todo lo demás es transitable (incluye minúsculas/botones)
    return true;
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

void Map::pairMechanisms(std::unordered_map<char, IVec2>& triggers, std::unordered_map<char, IVec2>& targets) {
    _mechanisms.clear();

    //recorremos los triggers para emparejarlos con sus targets
    for(auto it = triggers.begin(); it != triggers.end(); ++it) {
        //pasamos la letra a mayusculas para buscar su target
        char targetChar = std::toupper(it->first);
        auto targetIt = targets.find(targetChar);

        //si no encontramos el target lanzamos error
        if(targetIt == targets.end()) {
            throw std::runtime_error("Missing target for mechanism '" + std::string(1, targetChar) + "'");
        }

        //guardamos el mecanismo emparejado
        _mechanisms.push_back({targetChar, it->second, targetIt->second});

        //borramos el target ya emparejado para comprobar luego si quedan targets sin emparejar
        targets.erase(targetIt);
    }

    //comprobamos si quedan targets sin emparejar
    if(!targets.empty()) {
        throw std::runtime_error("Missing trigger for mechanism '" + std::string(1, targets.begin()->first) + "'");
    }
}

void Map::render(int ox, int oy) const {
    if (_grid.empty()) return;


    for (int y = 0; y < _h; ++y) {
        for (int x = 0; x < _w; ++x) {
            const char c = _grid[y][x];

            Rectangle destRect{
                (float)(ox + x * _tile),
                (float)(oy + y * _tile),
                (float)_tile,
                (float)_tile
            };

            // 1) Dibujar suelo base
            DrawTexturePro(*_mapTexture, _floorSrc, destRect, {0,0}, 0.0f, WHITE);
            
            // 2) Dibujar paredes
            if (c == '#') {
                DrawTexturePro(*_mapTexture, _wallSrc, destRect, {0,0}, 0.0f, WHITE);
            } 
            // 3) Dibujar salida
            else if (c == 'X') {
                DrawTexturePro(*_mapTexture, _exitSrc, destRect, {0,0}, 0.0f, WHITE);
            }
            // 4) Dibujar llave
            else if (c == 'K') {
                DrawTexturePro(*_keyTexture, _keySrc, destRect, {0,0}, 0.0f, WHITE);
    
            }
        }
    }
}
