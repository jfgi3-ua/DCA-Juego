cmake_minimum_required(VERSION 3.10)

# Este subproyecto contiene únicamente la infraestructura mínima de tests.
# Vamos a usar primero un "dummy test" (sin framework) para comprobar que:
# - CTest detecta y registra un test.
# - La cadena de build funciona.

# Ejecutable de test mínimo (sin dependencias adicionales).
add_executable(game_tests
    dummy_test.cpp
)

# Registrar el test en CTest.
# Si el ejecutable devuelve 0, el test pasa; cualquier otro valor falla.
add_test(NAME game_tests COMMAND game_tests)
