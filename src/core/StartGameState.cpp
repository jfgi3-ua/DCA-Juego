#include "StartGameState.hpp"
#include "StateMachine.hpp"
#include "ResourceManager.hpp"
#include "PlayerSpriteCatalog.hpp"
#include "SelectPlayerState.hpp"
#include <iostream>

extern "C" {
    #include <raylib.h>
}

StartGameState::StartGameState() = default;

StartGameState::~StartGameState() {
}

void StartGameState::init() {
    auto& rm = ResourceManager::Get();
    // Pre-cargar las texturas necesarias usando el gestor de recursos
    rm.GetTexture("sprites/menus/background_inicio.png");
    rm.GetTexture("sprites/menus/title.png");
    rm.GetTexture("sprites/icons/boton_jugar.png");
    rm.GetTexture("sprites/icons/boton_salir.png");

    // Discovery de sets de jugador (log temporal para validar).
    auto sets = DiscoverPlayerSpriteSets();
    auto defaultId = ResolveDefaultPlayerSpriteSetId(sets);
    LogPlayerSpriteSets(sets, defaultId);
}

void StartGameState::handleInput() {
    Vector2 mousePos = GetMousePosition();

    // Configurar rectángulos de botones - en horizontal
    float buttonWidth = 250;
    float buttonHeight = 80;
    float spacing = 50; // Espacio entre botones
    float totalWidth = (buttonWidth * 2) + spacing;
    float startX = (WINDOW_WIDTH - totalWidth) / 2.0f;
    float startY = WINDOW_HEIGHT / 2.0f + 100;

    // Área clickeable ajustada (compensando transparencias en el sprite)
    // Los sprites tienen mucho espacio transparente, ajustamos al área visible central
    float clickPadding = 40; // Reducir área clickeable desde los bordes

    Rectangle playButton = {
        startX + clickPadding,
        startY + clickPadding/2,
        buttonWidth - (clickPadding * 2),
        buttonHeight - clickPadding
    };

    Rectangle exitButton = {
        startX + buttonWidth + spacing + clickPadding,
        startY + clickPadding/2,
        buttonWidth - (clickPadding * 2),
        buttonHeight - clickPadding
    };

    // Detectar hover con ratón
    if (CheckCollisionPointRec(mousePos, playButton)) {
        _selectedOption = 0;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            std::cout << "Clic en JUGAR" << std::endl;
            this->state_machine->add_state(std::make_unique<SelectPlayerState>(), true);
            return;
        }
    }

    if (CheckCollisionPointRec(mousePos, exitButton)) {
        _selectedOption = 1;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            std::cout << "Clic en SALIR" << std::endl;
            this->state_machine->set_game_ending(true);
            return;
        }
    }

    // Cambiar selección menu con teclado
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
        std::cout << "Tecla izquierda/derecha presionada." << std::endl;
        _selectedOption = !_selectedOption;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        std::cout << "Tecla enter presionada." << std::endl;
        if(_selectedOption){
            this->state_machine->set_game_ending(true);
        }else{
            this->state_machine->add_state(std::make_unique<SelectPlayerState>(), true);
        }
    }
}

void StartGameState::update(float) {
}

void StartGameState::render() {
    ClearBackground(BLACK);
    Vector2 mousePos = GetMousePosition();

    // Obtener texturas del gestor de recursos
    auto& rm = ResourceManager::Get();
    const Texture2D& background = rm.GetTexture("sprites/menus/background_inicio.png");
    const Texture2D& title = rm.GetTexture("sprites/menus/title.png");
    const Texture2D& botonJugar = rm.GetTexture("sprites/icons/boton_jugar.png");
    const Texture2D& botonSalir = rm.GetTexture("sprites/icons/boton_salir.png");

    // Dibujar fondo (ajustado exactamente al tamaño de la ventana sin estirar)
    // Calculamos el ratio para cubrir toda la pantalla manteniendo aspecto
    float bgAspect = (float)background.width / (float)background.height;
    float screenAspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    float bgWidth, bgHeight, bgX, bgY;

    if (bgAspect > screenAspect) {
        // Background más ancho proporcionalmente
        bgHeight = WINDOW_HEIGHT;
        bgWidth = bgHeight * bgAspect;
        bgX = -(bgWidth - WINDOW_WIDTH) / 2.0f;
        bgY = 0;
    } else {
        // Background más alto proporcionalmente
        bgWidth = WINDOW_WIDTH;
        bgHeight = bgWidth / bgAspect;
        bgX = 0;
        bgY = -(bgHeight - WINDOW_HEIGHT) / 2.0f;
    }

    DrawTexturePro(
        background,
        {0, 0, (float)background.width, (float)background.height},
        {bgX, bgY, bgWidth, bgHeight},
        {0, 0},
        0.0f,
        WHITE
    );

    // Dibujar título (escalado para caber en la pantalla)
    float titleScale = 0.3f; // Reducir el título más
    float titleWidth = title.width * titleScale;
    float titleX = (WINDOW_WIDTH - titleWidth) / 2.0f;
    float titleY = 50.0f; // Posición fija desde arriba

    DrawTextureEx(title, {titleX, titleY}, 0.0f, titleScale, WHITE);

    // Configuración de botones - MISMAS dimensiones y posiciones que en handleInput()
    float buttonWidth = 250;
    float buttonHeight = 80;
    float spacing = 50; // Espacio entre botones
    float totalWidth = (buttonWidth * 2) + spacing;
    float startX = (WINDOW_WIDTH - totalWidth) / 2.0f;
    float startY = WINDOW_HEIGHT / 2.0f + 100;

    // Botón JUGAR
    Rectangle playButton = {startX, startY, buttonWidth, buttonHeight};
    bool playHover = CheckCollisionPointRec(mousePos, playButton);

    DrawTexturePro(
        botonJugar,
        {0, 0, (float)botonJugar.width, (float)botonJugar.height},
        playButton,
        {0, 0},
        0.0f,
        (_selectedOption == 0 || playHover) ? WHITE : Color{180, 180, 180, 255}
    );

    // Botón SALIR
    Rectangle exitButton = {startX + buttonWidth + spacing, startY, buttonWidth, buttonHeight};
    bool exitHover = CheckCollisionPointRec(mousePos, exitButton);

    DrawTexturePro(
        botonSalir,
        {0, 0, (float)botonSalir.width, (float)botonSalir.height},
        exitButton,
        {0, 0},
        0.0f,
        (_selectedOption == 1 || exitHover) ? WHITE : Color{180, 180, 180, 255}
    );
}
