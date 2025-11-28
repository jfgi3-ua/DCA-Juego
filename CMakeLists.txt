cmake_minimum_required(VERSION 3.10)

project(game
    VERSION 0.2
    DESCRIPTION "Laberinto"
)

add_executable(game)

target_sources(game PRIVATE
    src/core/main.cpp
    src/core/GameOverState.cpp
    src/core/MainGameState.cpp
    src/core/StartGameState.cpp
    src/core/StateMachine.cpp
    src/core/GameState.cpp
    src/objects/Player.cpp
    src/objects/Enemy.cpp
    src/objects/Map.cpp
    src/objects/Mechanisim.cpp
    src/objects/Spikes.cpp
)

# Exponer el directorio src para que las includes como "objects/Map.hpp" funcionen
target_include_directories(game PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/vendor/include)

# Colocar binarios en build/bin para que queden localizados dentro del árbol de build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)