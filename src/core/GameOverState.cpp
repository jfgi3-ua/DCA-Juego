#include "GameOverState.hpp"

extern "C" {
    #include <raylib.h>
}

GameOverState::GameOverState(int nivel, bool die, float time, bool isVictory)
{
    isDead_ = die;
    remainingTime_ = time;
    isVictory_ = isVictory;
    backgroundColor_ = isVictory_ ? GOLD : (isDead_ ? RED : DARKGREEN); // Si añadimos sprites eliminar color de fondo
    currentLevel_ = nivel;
}

void GameOverState::loadSprites_(const std::vector<std::string>& sprites) {
    auto& rm = ResourceManager::Get();
    if (!sprites.empty()) {
        background_ = &rm.GetTexture(sprites[0]);
        for (size_t i = 1; i < sprites.size(); ++i) {
            rm.GetTexture(sprites[i]);
        }
    }
}

void GameOverState::init() {
    // Cargar sprites necesarios
    if (isVictory_) {
        loadSprites_(spritesPaths_.victorySprites_);
    } else if (isDead_) {
        loadSprites_(spritesPaths_.deathSprites_);
    } else {
        // Nivel completado - cargar background y botones
        loadSprites_(spritesPaths_.levelCompletedSprites_);
    }
}

// Maneja la lógica de los botones y la selección (Ratón y teclado)
void GameOverState::handleButtons_(const Rectangle& button1Area, const Rectangle& button2Area, std::function<void()> action1, std::function<void()> action2) {
    Vector2 mousePos = GetMousePosition();

    if (CheckCollisionPointRec(mousePos, button1Area)) {
        selectedOption_ = 0;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            action1();
            return;
        }
    }

    if (CheckCollisionPointRec(mousePos, button2Area)) {
        selectedOption_ = 1;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            action2();
            return;
        }
    }

    // Navegación con teclado horizontal
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
        selectedOption_ = !selectedOption_;
    }
}

// Configura las áreas de los botones basándose en la configuración dada
void GameOverState::handleConfig_(const ButtonConfig& config, std::function<void()> action1, std::function<void()> action2) {
    float totalWidth = (config.buttonWidth * 2) + config.spacing;
    float startX = (WINDOW_WIDTH - totalWidth) / 2.0f;

    // Definir áreas de los botones
    Rectangle button1Area = {
        startX + config.clickPaddingX,
        config.yPos + config.clickPaddingY,
        config.buttonWidth - (config.clickPaddingX * 2),
        config.buttonHeight - (config.clickPaddingY * 2)
    };

    Rectangle button2Area = {
        startX + config.buttonWidth + config.spacing + config.clickPaddingX,
        config.yPos + config.clickPaddingY,
        config.buttonWidth - (config.clickPaddingX * 2),
        config.buttonHeight - (config.clickPaddingY * 2)
    };

    // Llamar a la función que maneja los botones con las áreas definidas
    handleButtons_(button1Area, button2Area, action1, action2);
}

void GameOverState::handleInput() {
    if (!isDead_ && !isVictory_) {

        // Juego completado o siguiente nivel 
        auto action1 = [this]() {

            std::cout << "Clic en SIGUIENTE NIVEL" << std::endl;

            // Si es el nivel 6, ir a pantalla de victoria total
            if (currentLevel_ >= 6) {
                this->state_machine->add_state(std::make_unique<GameOverState>(6, false, remainingTime_, true), true);
            }
            // Si no, cargar siguiente nivel
            else {
                this->state_machine->add_state(std::make_unique<MainGameState>(currentLevel_ + 1), true);
            }
        };

        // Salir del juego
        auto action2 = [this]() {
            std::cout << "Clic en SALIR" << std::endl;
            this->state_machine->set_game_ending(true);
        };

        // Manejar botones con la configuración de nivel completado
        handleConfig_(levelConfig_, action1, action2);
    } 
    else {
        // Muerte o victoria total
        auto action1 = [this]() {

            if (isVictory_) {
                std::cout << "Clic en EMPEZAR DE NUEVO" << std::endl;
                this->state_machine->add_state(std::make_unique<MainGameState>(1), true);
            } 
            else {
                std::cout << "Clic en REINTENTAR" << std::endl;
                this->state_machine->add_state(std::make_unique<MainGameState>(currentLevel_), true);
            }
        };

        // Salir del juego  
        auto action2 = [this]() {
            std::cout << "Clic en SALIR" << std::endl;
            this->state_machine->set_game_ending(true);
        };

        // Manejar botones con la configuración de muerte/victoria
        handleConfig_(otherConfig_, action1, action2);
    }

    if (IsKeyPressed(KEY_ENTER)) {
        if (selectedOption_) {
            this->state_machine->set_game_ending(true);
        } 
        else {
            if (isVictory_) {
                this->state_machine->add_state(std::make_unique<MainGameState>(1), true);
            } 
            else if (isDead_) {
                this->state_machine->add_state(std::make_unique<MainGameState>(currentLevel_), true);
            } 
            else if (currentLevel_ >= 6) {
                this->state_machine->add_state(std::make_unique<GameOverState>(6, false, remainingTime_, true), true);
            } 
            else {
                this->state_machine->add_state(std::make_unique<MainGameState>(currentLevel_ + 1), true);
            }
        }
    }
}

void GameOverState::update(float) {}

void GameOverState::render()
{
    ClearBackground(BLACK);
    Vector2 mousePos = GetMousePosition();
    auto& rm = ResourceManager::Get();

    // Dibujar background si existe (victoria, muerte o nivel completado)
    if (background_) {
        DrawTexturePro(
            *background_,
            {0, 0, (float)background_->width, (float)background_->height},
            {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT},
            {0, 0}, 0.0f, WHITE
        );
    }

    // --- RENDERIZADO ESPECÍFICO SEGÚN EL ESTADO ---
    if (!isDead_ && !isVictory_) {
        // Mostrar solo tiempo restante en amarillo con reborde negro
        int totalSeconds = (int)remainingTime_;
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        char timeText[32];
        sprintf(timeText, "Tiempo restante: %02d:%02d", minutes, seconds);
        int timeWidth = MeasureText(timeText, 35);
        int textX = (WINDOW_WIDTH - timeWidth) / 2;
        int textY = 250;
        
        // Dibujar reborde negro (dibujando el texto desplazado en 8 direcciones)
        int offset = 2;
        for (int dx = -offset; dx <= offset; dx++) {
            for (int dy = -offset; dy <= offset; dy++) {
                if (dx != 0 || dy != 0) {
                    DrawText(timeText, textX + dx, textY + dy, 35, BLACK);
                }
            }
        }
        
        // Dibujar texto amarillo encima
        DrawText(timeText, textX, textY, 35, GOLD);
    } else if (isVictory_) {
        // VICTORIA - Dibujar sprite de congratulations encima del background
        const Texture2D& congrats = rm.GetTexture("sprites/menus/items_congratulations.png");
        
        float scale = 0.35f;
        float width = congrats.width * scale;
        float height = congrats.height * scale;
        float x = (WINDOW_WIDTH - width) / 2.0f;
        float y = 100;
        
        DrawTexturePro(
            congrats,
            {0, 0, (float)congrats.width, (float)congrats.height},
            {x, y, width, height},
            {0, 0}, 0.0f, WHITE
        );
    } else {
        // GAME OVER - Dibujar sprite con huesos encima del background
        const Texture2D& gameOver = rm.GetTexture("sprites/menus/game_over.png");
        
        float scale = 0.45f;
        float width = gameOver.width * scale;
        float height = gameOver.height * scale;
        float x = (WINDOW_WIDTH - width) / 2.0f;
        float y = 80;
        
        DrawTexturePro(
            gameOver,
            {0, 0, (float)gameOver.width, (float)gameOver.height},
            {x, y, width, height},
            {0, 0}, 0.0f, WHITE
        );
    }

    // --- Botones ---
    if (!isDead_ && !isVictory_) {
        // Botones con sprites para nivel completado
        const Texture2D& botonSiguiente = rm.GetTexture("sprites/icons/boton_siguiente_nivel.png");
        const Texture2D& botonSalir = rm.GetTexture("sprites/icons/boton_salir.png");
        
        float buttonWidth = 400;
        float buttonHeight = 120;
        float buttonSpacing = 100;
        float totalWidth = (buttonWidth * 2) + buttonSpacing;
        float startX = (WINDOW_WIDTH - totalWidth) / 2.0f;
        float buttonY = 380;
        
        // Dibujar botón Siguiente Nivel
        Rectangle siguienteButton = {startX, buttonY, buttonWidth, buttonHeight};
        DrawTexturePro(
            botonSiguiente,
            {0, 0, (float)botonSiguiente.width, (float)botonSiguiente.height},
            siguienteButton,
            {0, 0}, 0.0f,
            (selectedOption_ == 0) ? WHITE : Color{180, 180, 180, 255}
        );
        
        // Dibujar botón Salir
        Rectangle salirButton = {startX + buttonWidth + buttonSpacing, buttonY, buttonWidth, buttonHeight};
        DrawTexturePro(
            botonSalir,
            {0, 0, (float)botonSalir.width, (float)botonSalir.height},
            salirButton,
            {0, 0}, 0.0f,
            (selectedOption_ == 1) ? WHITE : Color{180, 180, 180, 255}
        );
    } else if (isVictory_) {
        // Botones con sprites para victoria - horizontal como en la imagen
        const Texture2D& botonEmpezar = rm.GetTexture("sprites/icons/boton_reiniciar.png");
        const Texture2D& botonSalir = rm.GetTexture("sprites/icons/boton_salir.png");
        
        float buttonWidth = 350;
        float buttonHeight = 100;
        float spacing = 80;
        float totalWidth = (buttonWidth * 2) + spacing;
        float startX = (WINDOW_WIDTH - totalWidth) / 2.0f;
        float startY = WINDOW_HEIGHT - 200;
        
        float clickPadding = 40;
        
        // Botón EMPEZAR DE NUEVO
        Rectangle empezarButton = {startX, startY, buttonWidth, buttonHeight};
        Rectangle empezarClickArea = {
            startX + clickPadding,
            startY + clickPadding/2,
            buttonWidth - (clickPadding * 2),
            buttonHeight - clickPadding
        };
        bool empezarHover = CheckCollisionPointRec(mousePos, empezarClickArea);
        
        DrawTexturePro(
            botonEmpezar,
            {0, 0, (float)botonEmpezar.width, (float)botonEmpezar.height},
            empezarButton,
            {0, 0}, 0.0f,
            (selectedOption_ == 0 || empezarHover) ? WHITE : Color{180, 180, 180, 255}
        );
        
        // Botón SALIR
        Rectangle salirButton = {startX + buttonWidth + spacing, startY, buttonWidth, buttonHeight};
        Rectangle salirClickArea = {
            startX + buttonWidth + spacing + clickPadding,
            startY + clickPadding/2,
            buttonWidth - (clickPadding * 2),
            buttonHeight - clickPadding
        };
        bool salirHover = CheckCollisionPointRec(mousePos, salirClickArea);
        
        DrawTexturePro(
            botonSalir,
            {0, 0, (float)botonSalir.width, (float)botonSalir.height},
            salirButton,
            {0, 0}, 0.0f,
            (selectedOption_ == 1 || salirHover) ? WHITE : Color{180, 180, 180, 255}
        );
    } else {
        // GAME OVER - Botones horizontales con sprites
        const Texture2D& botonReiniciar = rm.GetTexture("sprites/icons/boton_reiniciar.png");
        const Texture2D& botonSalir = rm.GetTexture("sprites/icons/boton_salir.png");
        
        float buttonWidth = 350;
        float buttonHeight = 100;
        float spacing = 80;
        float totalWidth = (buttonWidth * 2) + spacing;
        float startX = (WINDOW_WIDTH - totalWidth) / 2.0f;
        float startY = WINDOW_HEIGHT - 200;
        float clickPadding = 40;
        
        // Botón REINICIAR
        Rectangle reiniciarButton = {startX, startY, buttonWidth, buttonHeight};
        Rectangle reiniciarClickArea = {
            startX + clickPadding,
            startY + clickPadding/2,
            buttonWidth - (clickPadding * 2),
            buttonHeight - clickPadding
        };
        bool reiniciarHover = CheckCollisionPointRec(mousePos, reiniciarClickArea);
        
        DrawTexturePro(
            botonReiniciar,
            {0, 0, (float)botonReiniciar.width, (float)botonReiniciar.height},
            reiniciarButton,
            {0, 0}, 0.0f,
            (selectedOption_ == 0 || reiniciarHover) ? WHITE : Color{180, 180, 180, 255}
        );
        
        // Botón SALIR
        Rectangle salirButton = {startX + buttonWidth + spacing, startY, buttonWidth, buttonHeight};
        Rectangle salirClickArea = {
            startX + buttonWidth + spacing + clickPadding,
            startY + clickPadding/2,
            buttonWidth - (clickPadding * 2),
            buttonHeight - clickPadding
        };
        bool salirHover = CheckCollisionPointRec(mousePos, salirClickArea);
        
        DrawTexturePro(
            botonSalir,
            {0, 0, (float)botonSalir.width, (float)botonSalir.height},
            salirButton,
            {0, 0}, 0.0f,
            (selectedOption_ == 1 || salirHover) ? WHITE : Color{180, 180, 180, 255}
        );
    }
}
