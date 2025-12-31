cmake_minimum_required(VERSION 3.10)

# Este subproyecto contiene la infraestructura de tests con Catch2 (espero que no nos haga cambiar a Boost.Test).
# Catch2 nos da macros de test, asserts y un runner estándar para CTest.

include(FetchContent)

# Dependencia Catch2 (se descarga en el configure).
# Nota: cuando se ejecute CMake con red restringida, será necesario permitir
# el acceso a la red para completar la descarga (por si acaso dejo esto por aqui).
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)

FetchContent_MakeAvailable(Catch2)

# Ejecutable de tests con Catch2.
add_executable(game_tests
    test_basic.cpp
)

# Los tests enlazan con el núcleo del juego para reutilizar la lógica.
# Catch2 provee el main automáticamente y simplifica el setup.
target_link_libraries(game_tests PRIVATE game_core Catch2::Catch2WithMain)

# Registro automático de tests en CTest.
# CTest detecta los tests sin añadirlos manualmente uno a uno.
include(Catch)
catch_discover_tests(game_tests)
