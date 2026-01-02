#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <string>
#include "core/ResourceManager.hpp"
#include "raylib_stubs.hpp"

TEST_CASE("ResourceManager: GetAssetPath resuelve assets existentes", "[resources][io]") {
    ResourceManager& rm = ResourceManager::Get();

    const std::string path = rm.GetAssetPath("maps/map_1.txt");
    REQUIRE(std::filesystem::exists(path));

    // Debe resolver dentro de la carpeta assets del repo.
    REQUIRE(path.find("assets/maps/map_1.txt") != std::string::npos);
}

TEST_CASE("ResourceManager: GetAssetPath devuelve entrada si no existe", "[resources][io]") {
    ResourceManager& rm = ResourceManager::Get();

    const std::string filename = "no_such_asset_123.txt";
    const std::string path = rm.GetAssetPath(filename);

    REQUIRE(path == filename);
}

TEST_CASE("ResourceManager: GetTexture usa cache y evita cargas duplicadas", "[resources][cache]") {
    ResourceManager& rm = ResourceManager::Get();

    // Aseguramos estado limpio antes de medir.
    rm.UnloadAll();
    RaylibStub_ResetCounters();

    const std::string texturePath = "sprites/player/Knight/Idle.png";

    const Texture2D& t1 = rm.GetTexture(texturePath);
    const Texture2D& t2 = rm.GetTexture(texturePath);

    // La misma entrada debe devolver la misma referencia y cargar solo una vez.
    REQUIRE(&t1 == &t2);
    REQUIRE(RaylibStub_GetLoadTextureCalls() == 1);
}
