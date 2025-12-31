cmake_minimum_required(VERSION 3.10)

# Este subproyecto contiene únicamente la infraestructura mínima de tests.
# Vamos a usar primero un "dummy test" (sin framework) para comprobar que:
# - CTest detecta y registra un test.
# - La cadena de build funciona.

# Ejecutable de test mínimo (sin dependencias adicionales).
add_executable(game_tests
    dummy_test.cpp
)

# Los tests enlazan con el núcleo del juego para reutilizar la lógica.
# En el paso anterior esto no era necesario, pero lo dejamos preparado para los siguientes hitos.
target_link_libraries(game_tests PRIVATE game_core)

# Registrar el test en CTest.
# Si el ejecutable devuelve 0, el test pasa; cualquier otro valor falla.
add_test(NAME game_tests COMMAND game_tests)
