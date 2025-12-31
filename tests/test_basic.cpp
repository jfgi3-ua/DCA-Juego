#include <catch2/catch_test_macros.hpp>

// Test básico para validar que Catch2 está integrado y CTest lo detecta.
// En los siguientes pasos este archivo se ampliará con tests reales
// puede que le cambie el nombre al archivo.
TEST_CASE("Catch2 is wired up", "[smoke]") {
    REQUIRE(1 + 1 == 2);
}
