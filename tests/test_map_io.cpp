#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "objects/Map.hpp"

namespace {
    /* Helper para construir rutas de fixtures desde la macro TESTS_DIR. Esto evita
       depender del directorio de ejecución del test.
     *   Una fixture es un conjunto de datos o recursos controlados que se preparan
     *   para que un test sea reproducible y predecible. En vuestro caso, las fixtures
     *   son archivos de mapa en tests/fixtures/ que representan entradas concretas
     *   (válidas o inválidas) para probar el parser de mapas. La idea es que el test no
     *   dependa del entorno ni de datos cambiantes: siempre carga la misma entrada y
     *   espera el mismo resultado.
    */
    std::string FixturePath(const std::string& filename) {
        return std::string(TESTS_DIR) + "/fixtures/" + filename;
    }

    // Helper RAII para crear y borrar un archivo temporal.
    // Se usa para validar CRLF sin modificar archivos del repositorio.
    struct TempFile {
        std::string path;

        explicit TempFile(const std::string& content) {
            const std::filesystem::path base = std::filesystem::temp_directory_path();
            const std::filesystem::path file = base / "dca_test_map_crlf.txt";
            path = file.string();

            std::ofstream out(path, std::ios::binary);
            out << content;
        }

        ~TempFile() {
            if (!path.empty()) {
                std::remove(path.c_str());
            }
        }
    };
}

TEST_CASE("Map: cargar mapa valido desde archivo", "[map][io]") {
    Map map;
    const std::string path = FixturePath("valid_map.txt");

    REQUIRE(map.loadFromFile(path, 16));
    REQUIRE(map.width() == 5);
    REQUIRE(map.height() == 4);
    REQUIRE(map.playerStart().x >= 0);
    REQUIRE(map.playerStart().y >= 0);
}

TEST_CASE("Map: archivo inexistente lanza error", "[map][io]") {
    Map map;
    const std::string path = FixturePath("does_not_exist.txt");

    REQUIRE_THROWS_AS(map.loadFromFile(path, 16), std::runtime_error);
}

TEST_CASE("Map: archivo vacio lanza error", "[map][io]") {
    Map map;
    const std::string path = FixturePath("empty_map.txt");

    REQUIRE_THROWS_AS(map.loadFromFile(path, 16), std::runtime_error);
}

TEST_CASE("Map: mapa no rectangular lanza error", "[map][io]") {
    Map map;
    const std::string path = FixturePath("non_rect_map.txt");

    REQUIRE_THROWS_AS(map.loadFromFile(path, 16), std::runtime_error);
}

TEST_CASE("Map: mapa sin jugador lanza error", "[map][io]") {
    Map map;
    const std::string path = FixturePath("no_player_map.txt");

    REQUIRE_THROWS_AS(map.loadFromFile(path, 16), std::runtime_error);
}

/* Test para validar que los finales de línea CRLF no contaminan los caracteres del mapa.
 *   CRLF significa Carriage Return + Line Feed (\r\n). Es el fin de línea típico de Windows.
 *   En Linux/macOS el fin de línea suele ser solo \n (LF). Si no se gestiona correctamente,
 *   al leer un archivo con CRLF, el \r puede quedarse al final de la línea y contaminar los
 *   caracteres del mapa (por ejemplo, la última columna podría ser '\r' en vez de '#').
 */
TEST_CASE("Map: soporta CRLF sin contaminar caracteres", "[map][io]") {
    Map map;
    TempFile tmp("#####\r\n#P..#\r\n#..X#\r\n#####\r\n");

    REQUIRE(map.loadFromFile(tmp.path, 16));
    REQUIRE(map.at(1, 1) == 'P');
    REQUIRE(map.at(3, 2) == 'X');
}
