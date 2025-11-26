cmake_minimum_required(3.10)

project("game")

add_executable(game)
add_sources(game
    "src/main.cpp"
    "src/Game.cpp"
    "src/Player.cpp"
    "src/Enemy.cpp"
)