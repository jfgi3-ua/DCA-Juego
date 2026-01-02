#include "StartGameState.hpp"
#include "StateMachine.hpp"
#include "ResourceManager.hpp"

#include "PlayerSpriteCatalog.hpp"
#include "SelectPlayerState.hpp"
#include "Localization.hpp"
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
    // Cargar los botones según el idioma
    std::string suf = GetButtonSpriteLangSuffix();
    // Construir las rutas de forma segura para evitar ambigüedades de concatenación
    std::string playTex = std::string("sprites/icons/boton_jugar") + suf + ".png";
    std::string exitTex = std::string("sprites/icons/boton_salir") + suf + ".png";
    rm.GetTexture(playTex);
    rm.GetTexture(exitTex);

    // Registrar idioma actual como cargado
    lastLang_ = GetCurrentLanguage();

    // Discovery de sets de jugador (log temporal para validar).
    auto sets = DiscoverPlayerSpriteSets();
    auto defaultId = ResolveDefaultPlayerSpriteSetId(sets);
    LogPlayerSpriteSets(sets, defaultId);
}

void StartGameState::handleInput() {
    Vector2 mousePos = GetMousePosition();

    // Permitir cambio de idioma con tecla L/l
    // NOTE: main.cpp gestiona SwitchLocalization() globalmente. Aquí solo procesamos
    // el resto de entradas; la recarga de texturas se detecta en update().

    //añadimos boton localizacion
    Rectangle langButton = {
        WINDOW_WIDTH - 60.0f - 20.0f,
        20.0f,
        60.0f,
        30.0f
    };

    if (CheckCollisionPointRec(mousePos, langButton)) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            SwitchLocalization(); // Cambia EN <-> ES
            return;
        }
    }
    // Configurar rectángulos de botones - en horizontal
    float buttonWidth = 250;
    float buttonHeight = 80;
    float spacing = 50; // Espacio entre botones
    float totalWidth = (buttonWidth * 2) + spacing;
    float startX = (WINDOW_WIDTH - totalWidth) / 2.0f;
    float startY = WINDOW_HEIGHT / 2.0f + 100;

    // Área clickeable ajustada (compensando transparencias en el sprite)
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
        selectedOption = 0;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            std::cout << "Clic en JUGAR" << std::endl;
            this->state_machine->add_state(std::make_unique<SelectPlayerState>(), true);
            return;
        }
    }

    if (CheckCollisionPointRec(mousePos, exitButton)) {
        selectedOption = 1;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            std::cout << "Clic en SALIR" << std::endl;
            this->state_machine->set_game_ending(true);
            return;
        }
    }

    // Cambiar selección menu con teclado
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
        std::cout << "Tecla izquierda/derecha presionada." << std::endl;
        selectedOption = !selectedOption;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        std::cout << "Tecla enter presionada." << std::endl;
        if(selectedOption){
            this->state_machine->set_game_ending(true);
        }else{
            this->state_machine->add_state(std::make_unique<SelectPlayerState>(), true);
        }
    }
}

void StartGameState::update(float /*dt*/) {
    // Detectar cambio de idioma y recargar texturas de botones si procede
    std::string lang = GetCurrentLanguage();
    if (lang != lastLang_) {
        auto& rm = ResourceManager::Get();
        std::string basePlay = "sprites/icons/boton_jugar";
        std::string baseExit = "sprites/icons/boton_salir";

        // Descargar ambas versiones para forzar recarga
        rm.UnloadTexture(basePlay + ".png");
        rm.UnloadTexture(basePlay + "_en.png");
        rm.UnloadTexture(baseExit + ".png");
        rm.UnloadTexture(baseExit + "_en.png");

        std::string suf = GetButtonSpriteLangSuffix();
        std::string playTex = basePlay + suf + ".png";
        std::string exitTex = baseExit + suf + ".png";
        std::cout << "[Localization] detected change to " << lang << " loading: " << playTex << " , " << exitTex << std::endl;
        rm.GetTexture(playTex);
        rm.GetTexture(exitTex);

        lastLang_ = lang;
    }
}

void StartGameState::render() {
    ClearBackground(BLACK);
    Vector2 mousePos = GetMousePosition();

    // Obtener texturas del gestor de recursos
    auto& rm = ResourceManager::Get();
    const Texture2D& background = rm.GetTexture("sprites/menus/background_inicio.png");
    const Texture2D& title = rm.GetTexture("sprites/menus/title.png");
    std::string suf = GetButtonSpriteLangSuffix();
    std::string playTex = std::string("sprites/icons/boton_jugar") + suf + ".png";
    std::string exitTex = std::string("sprites/icons/boton_salir") + suf + ".png";
    const Texture2D& botonJugar = rm.GetTexture(playTex);
    const Texture2D& botonSalir = rm.GetTexture(exitTex);

    // Dibujar fondo (ajustado exactamente al tamaño de la ventana sin estirar)
    float bgAspect = (float)background.width / (float)background.height;
    float screenAspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    float bgWidth, bgHeight, bgX, bgY;

    if (bgAspect > screenAspect) {
        bgHeight = WINDOW_HEIGHT;
        bgWidth = bgHeight * bgAspect;
        bgX = -(bgWidth - WINDOW_WIDTH) / 2.0f;
        bgY = 0;
    } else {
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
    float titleScale = 0.3f;
    float titleWidth = title.width * titleScale;
    float titleX = (WINDOW_WIDTH - titleWidth) / 2.0f;
    float titleY = 50.0f;

    DrawTextureEx(title, {titleX, titleY}, 0.0f, titleScale, WHITE);

    // Configuración de botones - MISMAS dimensiones y posiciones que en handleInput()
    float buttonWidth = 250;
    float buttonHeight = 80;
    float spacing = 50;
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
        (selectedOption == 0 || playHover) ? WHITE : Color{180, 180, 180, 255}
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
        (selectedOption == 1 || exitHover) ? WHITE : Color{180, 180, 180, 255}
    );

    // Dibujar botón de cambio de idioma
    Rectangle langButton = {
        WINDOW_WIDTH - 60.0f - 20.0f,
        20.0f,
        60.0f,
        30.0f
    };

    bool langHover = CheckCollisionPointRec(mousePos, langButton);

    DrawRectangleRec(
        langButton,
        langHover ? Color{200, 200, 200, 255} : Color{160, 160, 160, 255}
    );

    std::string langText =
        (GetCurrentLanguage() == "en") ? "EN" : "ES";

    int fontSize = 20;
    int textWidth = MeasureText(langText.c_str(), fontSize);

    DrawText(
        langText.c_str(),
        langButton.x + (langButton.width - textWidth) / 2,
        langButton.y + (langButton.height - fontSize) / 2,
        fontSize,
        BLACK
    );
}
