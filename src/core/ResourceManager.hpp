#pragma once
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <filesystem>

class ResourceManager {
public:
    // patron singleton: devuelve la instancia única del gestor
    static ResourceManager& Get();

    // Devuelve una textura, la carga desde el disco o cache si ya estaba cargada.
    const Texture2D& GetTexture(const std::string& filename);

    // Libera una textura concreta del gestor
    void UnloadTexture(const std::string& filename);

    // Libera todos los recursos cargados por el gesto
    void UnloadAll();

    // encontrar la ruta completa de un asset dentro del proyecto o el sistema de ficheros
    std::string GetAssetPath(const std::string& filename);

private:
    ResourceManager() = default;
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

private:
    //cache texturas
    std::unordered_map<std::string, Texture2D> textures;
};
