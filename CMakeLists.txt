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

# --- Testing base (CTest) ----------------------------------------------------
# include(CTest) configura CTest y añade opciones estándar para tests.
# enable_testing() activa el registro de tests en este proyecto.
# Debe ir después de project() para que CMake tenga el proyecto definido.
include(CTest)
enable_testing()

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

# Fuentes del proyecto (núcleo compartible).
# Excluimos main.cpp en el siguiente paso para poder reutilizar el núcleo en tests sin arrancar el juego.
set(GAME_CORE_SOURCES
    src/core/main.cpp
    src/core/GameOverState.cpp
    src/core/DevModeState.cpp
    src/core/MainGameState.cpp
    src/core/StartGameState.cpp
    src/core/PlayerSpriteCatalog.cpp
    src/core/StateMachine.cpp
    src/core/GameState.cpp
    src/core/ResourceManager.cpp
    src/core/PlayerSelection.cpp
    src/core/SelectPlayerState.cpp
    src/objects/Player.cpp
    src/objects/Enemy.cpp
    src/objects/Map.cpp
    src/objects/Mechanism.cpp
    src/objects/Spikes.cpp
    src/ecs/systems/CollisionSystems.cpp
    src/ecs/systems/EnemySystems.cpp
    src/ecs/systems/LevelSetupSystem.cpp
    src/ecs/systems/PlayerSystems.cpp
    src/ecs/systems/RenderSystems.cpp
    src/ecs/systems/WorldSystems.cpp
)

# Quitamos main.cpp del núcleo para que el core sea "headless"
# y reutilizable en tests sin arrancar el juego.
list(REMOVE_ITEM GAME_CORE_SOURCES src/core/main.cpp)

# Librería con el conjunto de fuentes del núcleo del juego.
# Se usa tanto por el ejecutable principal como por los tests.
# Ahora el ejecutable y los tests comparten la misma lógica.
add_library(game_core
    ${GAME_CORE_SOURCES}
)

# Para que busque los includes propios del proyecto y de vendor
target_include_directories(game_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor/include
)

# Enlazar raylib desde el núcleo (consumidores heredan el enlace, por eso cambiamos a PUBLIC).
# CUalquier consumidor (game/tests) heredan includes y enlace.
target_link_libraries(game_core PUBLIC raylib)

# Ejecutable principal (solo contiene main.cpp).
add_executable(game
    src/core/main.cpp
)

# El juego depende del núcleo.
# El ejecutable queda limpio y usa el core como dependencia.
target_link_libraries(game PRIVATE game_core)

# Configuración multiplataforma
if (WIN32)
    message(STATUS "Configurando para Windows")
    # En Windows, raylib necesita WinMM, gdi32, etc. Lo ponemos en el core
    # para que también lo hereden los tests que enlazan con game_core.
    # Cambiado a PUBLIC para que tests hereden.
    target_link_libraries(game_core PUBLIC winmm gdi32 opengl32 user32)
elseif (UNIX)
    message(STATUS "Configurando para Linux")
    # En Linux, raylib necesita enlazar con librerías del sistema.
    # Se declaran en game_core para que game y tests hereden el enlace.
    # Cambiado a PUBLIC para que tests hereden.
    target_link_libraries(game_core PUBLIC m pthread GL X11)
endif()

# Enlace estático en MinGW para evitar DLLs externas
if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

# Organización de binarios
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Subdirectorio de tests (solo declara objetivos, no ejecuta nada. Se encontrará en la subcarpeta tests).
add_subdirectory(tests)
