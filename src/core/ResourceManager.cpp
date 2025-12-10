#include "ResourceManager.hpp"
#include <iostream>


const std::string LOCAL_PATH = "assets/";

#ifdef __linux__
// Ruta estándar en Linux para recursos de aplicaciones empaquetadas
const std::string INSTALL_PATH = "/usr/share/game/assets/";
#else
// En Windows o cualquier otro sistema
const std::string INSTALL_PATH = "assets/";
#endif



ResourceManager& ResourceManager::Get() {
    // static garantiza que solo se crea una vez
    static ResourceManager instance;
    return instance;
}


// encontrar la ruta completa de un asset dentro del proyecto o el sistema de ficheros
// filename es un path relativo dentro de assets/, por ejemplo "sprites/player.png"
std::string ResourceManager::GetAssetPath(const std::string& filename) {

    //ruta local (modo desarrollo)
    {
        std::filesystem::path base = LOCAL_PATH;       // "assets/"
        std::filesystem::path candidate = base / filename;

        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
    }

    //ruta instalada del sistema (solo Linux)
    {
        std::filesystem::path base = INSTALL_PATH;
        std::filesystem::path candidate = base / filename;

        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
    }

    std::cerr << "[ERROR] Asset no encontrado: " << filename << std::endl;
    return filename;
}

//devuelve una textura, la carga desde el disco o cache si ya estaba cargada.
const Texture2D& ResourceManager::GetTexture(const std::string& filename) {

    // 1. Revisar si ya está cacheada
    auto it = textures.find(filename);
    if (it != textures.end()) {
        return it->second;
    }

    // 2. Resolver ruta final del archivo
    std::string path = GetAssetPath(filename);

    // 3. Cargar textura desde disco
    Texture2D tex = LoadTexture(path.c_str());

    // Comprobación de error mínima
    if (tex.id == 0) {
        std::cerr << "[ERROR] Fallo al cargar textura desde: " << path << std::endl;
    }

    // 4. Guardar en caché
    textures[filename] = tex;

    // 5. Devolver referencia a la textura cacheada
    return textures[filename];
}


void ResourceManager::UnloadTexture(const std::string& filename) {

    auto it = textures.find(filename);
    if (it != textures.end()) {
        // Liberar la textura de la GPU
        ::UnloadTexture(it->second);

        // Eliminar del mapa
        textures.erase(it);
    }
}


void ResourceManager::UnloadAll() {

    // Libero uno por uno
    for (auto& t : textures) {
        UnloadTexture(t.first);
    }

    // Limpio la tabla
    textures.clear();
}
