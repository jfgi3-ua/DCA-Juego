
#include "GameOverState.hpp"
#include <libintl.h>
#define _(String) gettext(String)

extern "C" {
    #include <raylib.h>
}

// Definiciones de constantes estáticas
#include "Localization.hpp"

std::string GetButtonSprite(const std::string& base) {
    return "sprites/icons/" + base + GetButtonSpriteLangSuffix() + ".png";
}

const std::string GameOverState::TEX_BUTTON_NEXT = "sprites/icons/boton_siguiente_nivel.png";
const std::string GameOverState::TEX_BUTTON_EXIT = "sprites/icons/boton_salir.png";
const std::string GameOverState::TEX_BUTTON_RESTART = "sprites/icons/boton_reiniciar.png";

GameOverState::GameOverState(int nivel, bool die, float time, bool isVictory)
{
    isDead_ = die;
    remainingTime_ = time;
    isVictory_ = isVictory;
    backgroundColor_ = isVictory_ ? GOLD : (isDead_ ? RED : DARKGREEN); // Si añadimos sprites eliminar color de fondo
    currentLevel_ = nivel;
    // Guardar idioma actual para detectar cambios y recargar sprites si es necesario
    currentLang_ = GetCurrentLanguage();
}

void GameOverState::loadSprites_(const std::vector<std::string>& sprites) {
    auto& rm = ResourceManager::Get();
    if (!sprites.empty()) {
        std::string suf = GetButtonSpriteLangSuffix();
        std::string bg = sprites[0];
        if (bg.find("background_pasar_nivel") != std::string::npos) {
            // Si existe el archivo en inglés y el idioma es inglés, úsalo, si no, usa el español
            if (suf == "_en" && std::filesystem::exists(rm.GetAssetPath("sprites/menus/background_pasar_nivel_en.png"))) {
                bg = "sprites/menus/background_pasar_nivel_en.png";
            } else {
                bg = "sprites/menus/background_pasar_nivel.png";
            }
        } else if (bg.find("background_congratulations") != std::string::npos) {
            // Si existe el archivo en español, úsalo por defecto
            if (suf == "_en" && !std::filesystem::exists(rm.GetAssetPath("sprites/menus/background_congratulations_en.png"))) {
                bg = "sprites/menus/background_congratulations.png";
            } else {
                bg = "sprites/menus/background_congratulations" + suf + ".png";
            }
        }
        background_ = &rm.GetTexture(bg);
        // Ajustar backgroundDrawRect_ para que la versión _en tenga el mismo tamaño visual
        // que la versión en español (si existe). Calculamos la escala basada en el ancho
        // del sprite español para mantener la proporción visual.
        if (bg.find("_en") != std::string::npos) {
            // intentar obtener la ruta en español correspondiente
            std::string spanishBg = bg;
            size_t pos = spanishBg.rfind("_en");
            if (pos != std::string::npos) {
                spanishBg.erase(pos, 3); // quitar "_en"
            }
            if (std::filesystem::exists(rm.GetAssetPath(spanishBg))) {
                // Para la variante _en de "background_pasar_nivel" queremos que ocupe
                // toda la altura del juego y mantener la proporción.
                const Texture2D& texEn = rm.GetTexture(bg);
                const Texture2D& texEs = rm.GetTexture(spanishBg);
                if (bg.find("background_pasar_nivel_en") != std::string::npos) {
                    // Mostrar la variante _en con el mismo rectángulo que la versión
                    // en español: pantalla completa. Así evitar cambios de escala
                    // aparentes al cambiar entre idiomas.
                    backgroundDrawRect_ = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
                } else {
                    // Mantener el comportamiento previo (usar ancho de la versión española para conservar apariencia)
                    float scale = (float)WINDOW_WIDTH / (float)texEs.width; // usar la escala del español
                    float destW = texEn.width * scale;
                    float destH = texEn.height * scale;
                    backgroundDrawRect_.width = destW;
                    backgroundDrawRect_.height = destH;
                    backgroundDrawRect_.x = (WINDOW_WIDTH - destW) / 2.0f;
                    backgroundDrawRect_.y = (WINDOW_HEIGHT - destH) / 2.0f;
                }
            } else {
                // español no existe; usar fullscreen
                backgroundDrawRect_ = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
            }
        } else {
            // No es la variante _en: dibujar a pantalla completa
            backgroundDrawRect_ = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
        }

        for (size_t i = 1; i < sprites.size(); ++i) {
            std::string s = sprites[i];
            if (s.find("items_congratulations") != std::string::npos) {
                s = "sprites/menus/items_congratulations" + suf + ".png";
            }
            rm.GetTexture(s);
        }
    }
}

void GameOverState::init() {
    // Cargar sprites necesarios
    if (isVictory_) {
        loadSprites_(spritesPaths_.victorySprites);
    } else if (isDead_) {
        loadSprites_(spritesPaths_.deathSprites);
    } else {
        // Nivel completado - cargar background y botones
        loadSprites_(spritesPaths_.levelCompletedSprites);
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
            std::cout << _( "Clic en SIGUIENTE NIVEL" ) << std::endl;
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
            std::cout << "Abortado (`core' generado)" << std::endl;
            this->state_machine->set_game_ending(true);
        };

        // Manejar botones con la configuración de nivel completado
        handleConfig_(levelConfig_, action1, action2);
    } 
    else {
        // Muerte o victoria total

        auto action1 = [this]() {
            if (isVictory_) {
                std::cout << _( "Clic en EMPEZAR DE NUEVO" ) << std::endl;
                this->state_machine->add_state(std::make_unique<MainGameState>(1), true);
            } 
            else {
                std::cout << _( "Clic en REINTENTAR" ) << std::endl;
                this->state_machine->add_state(std::make_unique<MainGameState>(currentLevel_), true);
            }
        };

        // Salir del juego  
        auto action2 = [this]() {
            std::cout << "Abortado (`core' generado)" << std::endl;
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

void GameOverState::update(float)
{
    // Comprobar si cambió el idioma y recargar sprites/background si es necesario
    std::string lang = GetCurrentLanguage();
    if (lang != currentLang_) {
        currentLang_ = lang;
        if (isVictory_) {
            loadSprites_(spritesPaths_.victorySprites);
        } else if (isDead_) {
            loadSprites_(spritesPaths_.deathSprites);
        } else {
            loadSprites_(spritesPaths_.levelCompletedSprites);
        }
    }
}

void GameOverState::renderButtons_(const ButtonConfig& config, const std::string& tex1, const std::string& tex2, bool useHover) {
    auto& rm = ResourceManager::Get();

    const Texture2D& t1 = rm.GetTexture(tex1);
    const Texture2D& t2 = rm.GetTexture(tex2);

    float totalWidth = (config.buttonWidth * 2) + config.spacing;
    float startX = (WINDOW_WIDTH - totalWidth) / 2.0f;

    Rectangle button1Rect = {startX, config.yPos, config.buttonWidth, config.buttonHeight};
    Rectangle button2Rect = {startX + config.buttonWidth + config.spacing, config.yPos, config.buttonWidth, config.buttonHeight};

    Color color1 = (selectedOption_ == 0) ? WHITE : Color{180, 180, 180, 255};
    Color color2 = (selectedOption_ == 1) ? WHITE : Color{180, 180, 180, 255};

    if (useHover) {
        Vector2 mousePos = GetMousePosition();

        Rectangle click1 = {
            startX + config.clickPaddingX,
            config.yPos + config.clickPaddingY,
            config.buttonWidth - (config.clickPaddingX * 2),
            config.buttonHeight - (config.clickPaddingY * 2)
        };

        Rectangle click2 = {
            startX + config.buttonWidth + config.spacing + config.clickPaddingX,
            config.yPos + config.clickPaddingY,
            config.buttonWidth - (config.clickPaddingX * 2),
            config.buttonHeight - (config.clickPaddingY * 2)
        };

        if (CheckCollisionPointRec(mousePos, click1)) {
            color1 = WHITE;
        }

        if (CheckCollisionPointRec(mousePos, click2)) {
            color2 = WHITE;
        }
    }

    DrawTexturePro(t1, {0, 0, (float)t1.width, (float)t1.height}, button1Rect, {0, 0}, 0.0f, color1);
    DrawTexturePro(t2, {0, 0, (float)t2.width, (float)t2.height}, button2Rect, {0, 0}, 0.0f, color2);
}

void GameOverState::render()
{
    ClearBackground(BLACK);
    auto& rm = ResourceManager::Get();

    // Dibujar background si existe (victoria, muerte o nivel completado)
    if (background_) {
        DrawTexturePro(
            *background_,
            {0, 0, (float)background_->width, (float)background_->height},
            backgroundDrawRect_,
            {0, 0}, 0.0f, WHITE
        );
    }

    // Dibujar información según el estado
    if (!isDead_ && !isVictory_) {
        // Mostrar solo tiempo restante en amarillo con reborde negro
        int totalSeconds = (int)remainingTime_;
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        char timeText[64];
        sprintf(timeText, _( "Tiempo restante: %02d:%02d" ), minutes, seconds);
        int timeWidth = MeasureText(timeText, TIME_FONT_SIZE);
        int textX = (WINDOW_WIDTH - timeWidth) / 2;
        int textY = TIME_TEXT_Y;
        
        // Dibujar reborde negro (dibujando el texto desplazado en 8 direcciones)
        int offset = TEXT_OFFSET;
        for (int dx = -offset; dx <= offset; dx++) {
            for (int dy = -offset; dy <= offset; dy++) {
                if (dx != 0 || dy != 0) {
                    DrawText(timeText, textX + dx, textY + dy, TIME_FONT_SIZE, BLACK);
                }
            }
        }
        
        // Dibujar texto amarillo encima
        DrawText(timeText, textX, textY, TIME_FONT_SIZE, GOLD);
    } else if (isVictory_) {
        std::string suf = GetButtonSpriteLangSuffix();
        const Texture2D& congrats = rm.GetTexture("sprites/menus/items_congratulations" + suf + ".png");
        // Ajustar tamaño y posición para que coincida con el español
        float scale = CONGRATS_SCALE;
        float width = congrats.width * scale;
        float height = congrats.height * scale;
        float x = (WINDOW_WIDTH - width) / 2.0f;
        float y = CONGRATS_Y;
        // Si es inglés, forzar el mismo ancho y alto que el sprite español
        if (suf == "_en") {
            const Texture2D& congrats_es = rm.GetTexture("sprites/menus/items_congratulations.png");
            width = congrats_es.width * scale;
            height = congrats_es.height * scale;
            x = (WINDOW_WIDTH - width) / 2.0f;
            y = CONGRATS_Y;
        }
        DrawTexturePro(
            congrats,
            {0, 0, (float)congrats.width, (float)congrats.height},
            {x, y, width, height},
            {0, 0}, 0.0f, WHITE
        );
    } else {
        const Texture2D& gameOver = rm.GetTexture("sprites/menus/game_over.png");
        
        float scale = GAME_OVER_SCALE;
        float width = gameOver.width * scale;
        float height = gameOver.height * scale;
        float x = (WINDOW_WIDTH - width) / 2.0f;
        float y = GAME_OVER_Y;
        
        DrawTexturePro(
            gameOver,
            {0, 0, (float)gameOver.width, (float)gameOver.height},
            {x, y, width, height},
            {0, 0}, 0.0f, WHITE
        );
    }

    // Renderizar botones según el estado
    if (!isDead_ && !isVictory_) {
        renderButtons_(levelConfig_, GetButtonSprite("boton_siguiente_nivel"), GetButtonSprite("boton_salir"), false);
    } else if (isVictory_) {
        renderButtons_(otherConfig_, GetButtonSprite("boton_reiniciar"), GetButtonSprite("boton_salir"), true);
    } else {
        renderButtons_(otherConfig_, GetButtonSprite("boton_reiniciar"), GetButtonSprite("boton_salir"), true);
    }
}
