#include <catch2/catch_test_macros.hpp>
#include <string>
#include "objects/Map.hpp"

namespace {
    // Helper para construir rutas de fixtures desde la macro TESTS_DIR.
    std::string FixturePath(const std::string& filename) {
        return std::string(TESTS_DIR) + "/fixtures/" + filename;
    }
} // namespace

TEST_CASE("Map: mecanismos validos se emparejan con su tipo", "[map][mechanisms]") {
    Map map;
    const std::string path = FixturePath("mechanisms_valid.txt");

    REQUIRE(map.loadFromFile(path, 16));

    const auto& mechs = map.getMechanisms();
    REQUIRE(mechs.size() == 4);

    // El orden no esta garantizado (unordered_map), comprobamos por presencia.
    bool hasDoor = false;
    bool hasTrap = false;
    bool hasBridge = false;
    bool hasLever = false;

    for (const auto& m : mechs) {
        switch (m.type) {
            case MechanismType::DOOR: hasDoor = true; break;
            case MechanismType::TRAP: hasTrap = true; break;
            case MechanismType::BRIDGE: hasBridge = true; break;
            case MechanismType::LEVER: hasLever = true; break;
        }
    }

    REQUIRE(hasDoor);
    REQUIRE(hasTrap);
    REQUIRE(hasBridge);
    REQUIRE(hasLever);
}

TEST_CASE("Map: trigger sin target lanza error", "[map][mechanisms]") {
    Map map;
    const std::string path = FixturePath("mechanisms_missing_target.txt");

    REQUIRE_THROWS_AS(map.loadFromFile(path, 16), std::runtime_error);
}

TEST_CASE("Map: target sin trigger lanza error", "[map][mechanisms]") {
    Map map;
    const std::string path = FixturePath("mechanisms_missing_trigger.txt");

    REQUIRE_THROWS_AS(map.loadFromFile(path, 16), std::runtime_error);
}
