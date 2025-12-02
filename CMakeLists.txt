cmake_minimum_required(VERSION 3.10)

# Si no se define el tipo de build, usar Release por defecto
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()


project(game
    VERSION 0.3
    DESCRIPTION "Laberinto"
    LANGUAGES CXX
)

# Configuración de raylib multiplataforma
include(FetchContent)

# Descargar raylib automáticamente si no está disponible
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG master   # versión estable actual
)

# Construir raylib dentro del proyecto
FetchContent_MakeAvailable(raylib)

# Ejecutable principal
add_executable(game)

# Fuentes del proyecto
target_sources(game PRIVATE
    src/core/main.cpp
    src/core/GameOverState.cpp
    src/core/DevModeState.cpp
    src/core/MainGameState.cpp
    src/core/StartGameState.cpp
    src/core/StateMachine.cpp
    src/core/GameState.cpp
    src/objects/Player.cpp
    src/objects/Enemy.cpp
    src/objects/Map.cpp
    src/objects/Mechanism.cpp
    src/objects/Spikes.cpp
)

# Para que busque los includes propios del proyecto y de vendor
target_include_directories(game PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor/include
)

# Enlazar raylib
target_link_libraries(game PRIVATE raylib)

# Configuración multiplataforma
if (WIN32)
    message(STATUS "Configurando para Windows")
    # En Windows, raylib trae soporte para WinMM, gdi32, etc.
    target_link_libraries(game PRIVATE winmm gdi32 opengl32 user32 shell32 kernel32)
elseif (UNIX)
    message(STATUS "Configurando para Linux")
    # En Linux, raylib necesita enlazar con librerías del sistema
    target_link_libraries(game PRIVATE m pthread GL X11)
endif()

# Organización de binarios
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
