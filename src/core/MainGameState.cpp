#include "MainGameState.hpp"
#include "GameOverState.hpp"
#include "StateMachine.hpp"
#include "objects/Enemy.hpp"
#include <iostream>
extern "C" {
  #include <raylib.h>
}

static std::vector<Enemy> enemies;

MainGameState::MainGameState(int level)
{
    level_ = level > 0 ? level : 1;
}

void MainGameState::init()
{
    std::cout << "You are in the Main Game State, level " << level_ << std::endl;

    std::string path = "assets/maps/map_" + std::to_string(level_) + ".txt";
    map_.loadFromFile(path, 32);
    tile_ = map_.tile();

    IVec2 p = map_.playerStart();
    Vector2 startPos = { p.x * (float)tile_ + tile_ / 2.0f,
                         p.y * (float)tile_ + tile_ / 2.0f };
    player_.init(startPos, tile_ * 0.35f, 5);

    enemiesPos_.clear();
    enemies.clear();
    for (auto e : map_.enemyStarts()) {
        enemiesPos_.push_back({ e.x * (float)tile_ + tile_ / 2.0f,
                                e.y * (float)tile_ + tile_ / 2.0f });
        // crear un único Enemy usando el constructor que necesita tile_
        enemies.emplace_back(e.x, e.y, tile_);
    }

    for (auto s : map_.spikesStarts()) {
        spikes_.addSpike(s.x, s.y);
    }

    for (auto m : map_.getMechanisms()) {
        //m es un MechanismPair
        mechanisms_.emplace_back(m.id, m.trigger, m.target);
    }

    // Inicializar temporizador: 30s base + 30s por cada nivel adicional
    levelTime_ = 30.0f + (level_ - 1) * 30.0f;
}

void MainGameState::handleInput()
{
    // Si el menú dev está abierto o esperando contraseña, manejar solo eso
    if (awaitingPassword_ || devMenuOpen_) {
        handleDevModeInput();
        return;
    }
    
    // Activar menú de desarrollador con CTRL+D (solo si no está ya activo)
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D)) {
        awaitingPassword_ = true;
        passwordInput_ = "";
        return;
    }
}

void MainGameState::update(float deltaTime)
{
    activeMechanisms(); //actualizamos un vector con todos los mecanismos activos
    
    // Si el menú dev está abierto, pausar el juego
    if (devMenuOpen_ || awaitingPassword_) {
        return;
    }
    
    // Reducir temporizador de nivel (excepto si está en modo tiempo infinito)
    if (!infiniteTime_) {
        levelTime_ -= deltaTime;
        if (levelTime_ <= 0.0f) {
            levelTime_ = 0.0f;
            // Tiempo agotado -> Game Over (dead = true)
            this->state_machine->add_state(std::make_unique<GameOverState>(level_, true, 0.0f), true);
            return;
        }
    }

    // 1) Actualizar (movimiento) jugador con colisiones de mapa
    player_.update(deltaTime, map_, activeMechanisms_);

    // 2) Celda actual del jugador
    int cellX = (int)(player_.getPosition().x) / tile_;
    int cellY = (int)(player_.getPosition().y) / tile_;

    // 3) ¿Está sobre una llave 'K'? -> Recogerla
    try {
        if (map_.at(cellX, cellY) == 'K') {
            player_.setHasKey(true);
            map_.clearCell(cellX, cellY); // Reemplaza 'K' por '.', es decir, retira la llave del mapa.
            std::cout << "Llave recogida!" << std::endl; // Para debug
            // TODO:Aquí podríamos reproducir un sonido o mostrar un mensaje en pantalla si se desea.
        }
    } catch (const std::out_of_range& e) {
        // Ignorar fuera de rango (no debería ocurrir aquí). Fuera de rango no debería ocurrir si los cálculos de celda son correctos.
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // 4) ¿Está sobre la salida 'X' y tiene la llave? -> Nivel completado
    if (player_.isOnExit(map_) && player_.hasKey()) {
        std::cout << "Nivel completado" << std::endl;
        // Pasar a pantalla de nivel completado (dead = false)
        this->state_machine->add_state(std::make_unique<GameOverState>(level_, false, levelTime_), true);
        return;
    }

    // 5) IA enemigos y colisiones con jugador
    // Pasar la posición del jugador a los enemigos para el árbol de decisiones
    Vector2 playerPos = player_.getPosition();
    
    // Solo actualizar enemigos si no están congelados
    if (!freezeEnemies_) {
        for (auto &e : enemies) {
            e.update(map_, deltaTime, tile_, playerPos.x, playerPos.y);
        }
    }

    enemiesPos_.clear();
    for (auto &e : enemies) {
        enemiesPos_.push_back({ e.x * (float)tile_ + tile_ / 2.0f,
                                e.y * (float)tile_ + tile_ / 2.0f });
    }

    for (auto &e : enemies) {
        if (e.collidesWithPlayer(playerPos.x, playerPos.y, player_.getRadius()) && !player_.isInvulnerable()) {
            // Si colisiona, quitar una vida y empujar al jugador a la casilla previa
            player_.onHit(map_);
            // Notificar al enemigo que golpeó (solo los que persiguen se alejarán)
            e.onHitPlayer();
            std::cout << "El jugador ha sido golpeado por un enemigo. Vidas: " << player_.getLives() << std::endl;
        }
    }


    // 6) Pinchos y colisiones si activo
    spikes_.update(deltaTime);
    // Si el jugador está sobre un pincho activo
    if (spikes_.isActiveAt(cellX, cellY) && !player_.isInvulnerable()) {
    // Si colisiona, quitar una vida y empujar al jugador a la casilla previa
    player_.onHit(map_);
        std::cout << "El jugador ha sido golpeado por pinchos. " << player_.getLives() << std::endl;
    }

    // 7) Si el jugador no tiene vidas, cambiar al estado de Game Over
    if (player_.getLives() <= 0) {
        std::cout << "Game Over: El jugador no tiene más vidas." << std::endl;
        // Game Over por muerte (dead = true)
        this->state_machine->add_state(std::make_unique<GameOverState>(level_, true, levelTime_), true);
        return;
    }

    //7 Mecanismos, cmprobar si el jugador está sobre un trigger q no este activo
    for (auto& mech : mechanisms_) {
        IVec2 trigPos = mech.getTriggerPos();

         if (mech.isActive() && cellX == trigPos.x && cellY == trigPos.y) {
            mech.deactivate();
        }
    }
}

void MainGameState::render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Dimensiones
    const int mapWpx = map_.width()  * tile_;
    const int mapHpx = map_.height() * tile_;
    const int viewW  = GetScreenWidth();
    const int viewH  = GetScreenHeight() - HUD_HEIGHT; // Espacio disponible sin el HUD

    // Offset centrado (clamp >= 0) - El mapa queda centrado en el espacio disponible
    const int ox = std::max(0, (viewW - mapWpx) / 2);
    const int oy = std::max(0, (viewH - mapHpx) / 2);

    // 1) Mapa (dibujado en la zona superior, desde y=0 hasta y=MAP_H_PX)
    for (int y = 0; y < map_.height(); ++y) {
        for (int x = 0; x < map_.width(); ++x) {
            const char c = map_.at(x, y);
            Rectangle r{ (float)(ox + x * tile_), (float)(oy + y * tile_),(float)tile_, (float)tile_ };

            // Suelo + paredes
            DrawRectangleRec(r, (c == '#') ? LIGHTGRAY : WHITE);

            if (c == '#') DrawRectangleLinesEx(r, 1.0f, DARKGRAY);  // Paredes
            else if (c == 'X') DrawRectangleRec(r, LIME);           // Salida
            else if (c == 'K') {                                    // Llave
                // Pequeño icono dorado centrado en la celda
                Rectangle keyRect{
                    r.x + r.width * 0.25f,
                    r.y + r.height * 0.35f,
                    r.width * 0.5f,
                    r.height * 0.3f
                };
                DrawRectangleRec(keyRect, GOLD);
                DrawRectangleLinesEx(keyRect, 1.5f, BROWN);
            }
        }
    }

    // Mecanismos
    for (const auto& mech : mechanisms_) {
        mech.render(ox, oy);
    }
    // 2) Jugador
    player_.render(ox, oy);

    // 3) Enemigos (cuadrados)
    for (auto &e : enemies) {
        e.draw(tile_, ox, oy, RED);
    }

    spikes_.render(ox, oy);


    // 4) HUD inferior - se coloca en la parte inferior de la ventana
    const float baseY = (float)(GetScreenHeight() - HUD_HEIGHT); // HUD siempre abajo
    // Fondo del HUD a lo ancho de la ventana
    Rectangle hudBg{ 0.0f, baseY, (float)GetScreenWidth(), (float)HUD_HEIGHT };
    DrawRectangleRec(hudBg, Fade(BLACK, 0.06f));
    // sombreado sutil encima del HUD
    for (int i=0; i<6; ++i) {
        Color c = Fade(BLACK, 0.05f * (6 - i));
        DrawLine(0, (int)baseY - i, GetScreenWidth(), (int)baseY - i, c);
    }

    // Panel de mochila (alineado a la izquierda dentro del HUD)
    const int pad = 10;
    Rectangle hud{ (float)pad, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad };
    DrawRectangleRounded(hud, 0.25f, 6, Fade(BLACK, 0.10f));
    DrawRectangleRoundedLinesEx(hud, 0.25f, 6, 1.0f, DARKGRAY);
    DrawText("Mochila", (int)hud.x + 10, (int)hud.y + 6, 16, DARKGRAY);

    // Slot de llave
    Rectangle slot{ hud.x + 12, hud.y + 28, 28.0f, 28.0f };
    DrawRectangleLinesEx(slot, 1.0f, GRAY);
    DrawText("Llave", (int)hud.x + 50, (int)hud.y + 30, 16, GRAY);

    if (player_.hasKey()) {
        Rectangle keyIcon{ slot.x + 4, slot.y + 8, slot.width - 8, slot.height - 12 };
        DrawRectangleRec(keyIcon, GOLD);
        DrawRectangleLinesEx(keyIcon, 1.2f, BROWN);
    }

    // Panel de vidas (alineado a la derecha dentro del HUD)
    DrawRectangleRounded(Rectangle{ (float)GetScreenWidth() - 190.0f, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad }, 0.25f, 6, Fade(BLACK, 0.10f));
    DrawRectangleRoundedLinesEx(Rectangle{ (float)GetScreenWidth() - 190.0f, baseY + pad, 180.0f, HUD_HEIGHT - 2*pad }, 0.25f, 6, 1.0f, DARKGRAY);
    DrawText("Vidas", (int)((float)GetScreenWidth() - 190.0f) + 10, (int)(baseY + pad) + 6, 16, DARKGRAY);

    if (player_.getLives() > 0) {
        for (int i = 0; i < player_.getLives(); ++i) {
            DrawCircleV(Vector2{ (float)GetScreenWidth() - 190.0f + 12.0f + i * 20.0f + 6.0f, baseY + pad + 28.0f + 6.0f }, 5.0f, RED);
        }
    }

    // Mostrar temporizador centrado encima del HUD (formato mm:ss)
    int timerFont = 22;
    int minutes = (int)levelTime_ / 60;
    int seconds = (int)levelTime_ % 60;
    std::string timeText = "Tiempo: " + std::to_string(minutes) + ":" + 
                          (seconds < 10 ? "0" : "") + std::to_string(seconds);
    int textW = MeasureText(timeText.c_str(), timerFont);
    DrawText(timeText.c_str(), (GetScreenWidth() - textW) / 2, (int)baseY + 8, timerFont, DARKGRAY);

    // Mostrar nivel actual arriba a la izquierda
    std::string levelText = "Nivel: " + std::to_string(level_);
    DrawText(levelText.c_str(), 10, 10, 24, DARKGRAY);

    // 5) Mensaje contextual por encima del HUD
    const int cx = (int)(player_.getPosition().x / tile_);
    const int cy = (int)(player_.getPosition().y / tile_);
    if (cx >= 0 && cy >= 0 && cx < map_.width() && cy < map_.height()) {
        if (map_.at(cx, cy) == 'X' && !player_.hasKey()) {
            const char* msg = "Necesitas la llave para salir";
            const int font = 18;
            const int textW = MeasureText(msg, font);
            // 10 px de margen por encima del HUD
            const int textY = (int)baseY - font - 10;
            DrawText(msg, (GetScreenWidth()-textW)/2, textY, font, MAROON);
        }
    }

    // Renderizar menú de desarrollador si está activo
    if (awaitingPassword_ || devMenuOpen_) {
        renderDevMenu();
    }

    EndDrawing();
}

void MainGameState::activeMechanisms() {
    activeMechanisms_.clear();
    for (const auto& mech : mechanisms_) {
        if (mech.isActive()) {
            Vector2 target = { (float)mech.getTargetPos().x, (float)mech.getTargetPos().y };
            activeMechanisms_.push_back(target);
        }
    }
}

// ==================== DEVELOPER MODE ====================

void MainGameState::handleDevModeInput() {
    Vector2 mousePos = GetMousePosition();
    
    // Si estamos esperando contraseña
    if (awaitingPassword_) {
        int key = GetCharPressed();
        
        // Capturar caracteres
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && passwordInput_.length() < 20) {
                passwordInput_ += (char)key;
            }
            key = GetCharPressed();
        }
        
        // Borrar con backspace
        if (IsKeyPressed(KEY_BACKSPACE) && passwordInput_.length() > 0) {
            passwordInput_.pop_back();
        }
        
        // Enter para confirmar o clic en botón
        int menuX = GetScreenWidth() / 2 - 250;
        int menuY = 100;
        Rectangle confirmBtn = { (float)(menuX + 150), (float)(menuY + 240), 200.0f, 40.0f };
        Rectangle cancelBtn = { (float)(menuX + 150), (float)(menuY + 290), 200.0f, 40.0f };
        
        bool confirmClicked = CheckCollisionPointRec(mousePos, confirmBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        bool cancelClicked = CheckCollisionPointRec(mousePos, cancelBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        
        if (IsKeyPressed(KEY_ENTER) || confirmClicked) {
            if (passwordInput_ == "developer") {
                devModeActive_ = true;
                devMenuOpen_ = true;
                awaitingPassword_ = false;
                selectedDevOption_ = -1; // -1 = ninguno seleccionado
                passwordInput_.clear();
                std::cout << "Developer Mode activado!" << std::endl;
            } else {
                awaitingPassword_ = false;
                passwordInput_.clear();
                std::cout << "Contrasena incorrecta" << std::endl;
            }
        }
        
        // ESC para cancelar o clic en botón cancelar
        if (IsKeyPressed(KEY_ESCAPE) || cancelClicked) {
            awaitingPassword_ = false;
            passwordInput_.clear();
        }
        return;
    }
    
    // Si el menú está abierto
    if (devMenuOpen_) {
        int menuX = GetScreenWidth() / 2 - 250;
        int menuY = 100;
        
        // Detectar hover y clic en opciones
        for (int i = 0; i < 10; i++) {
            Rectangle optionRect = { 
                (float)(menuX + 30), 
                (float)(menuY + 100 + i * 38), 
                440.0f, 
                35.0f 
            };
            
            // Hover del ratón
            if (CheckCollisionPointRec(mousePos, optionRect)) {
                selectedDevOption_ = i;
                
                // Clic del ratón
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    applyDevCheat(i);
                }
            }
        }
        
        // Navegación por teclado (alternativa)
        if (IsKeyPressed(KEY_UP)) {
            selectedDevOption_--;
            if (selectedDevOption_ < 0) selectedDevOption_ = 9;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            selectedDevOption_++;
            if (selectedDevOption_ > 9) selectedDevOption_ = 0;
        }
        
        // Enter para aplicar cheat seleccionado
        if (IsKeyPressed(KEY_ENTER) && selectedDevOption_ >= 0) {
            applyDevCheat(selectedDevOption_);
        }
        
        // ESC o clic en botón cerrar para cerrar menú
        Rectangle closeBtn = { (float)(menuX + 150), (float)(menuY + 490), 200.0f, 25.0f };
        bool closeBtnClicked = CheckCollisionPointRec(mousePos, closeBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        
        if (IsKeyPressed(KEY_ESCAPE) || closeBtnClicked) {
            devMenuOpen_ = false;
            selectedDevOption_ = -1;
            std::cout << "Menu de desarrollador cerrado" << std::endl;
        }
    }
}

void MainGameState::applyDevCheat(int option) {
    switch (option) {
        case 0: // God Mode
            godMode_ = !godMode_;
            player_.setGodMode(godMode_);
            std::cout << "God Mode: " << (godMode_ ? "ON" : "OFF") << std::endl;
            break;
            
        case 1: // Congelar Enemigos
            freezeEnemies_ = !freezeEnemies_;
            std::cout << "Freeze Enemies: " << (freezeEnemies_ ? "ON" : "OFF") << std::endl;
            break;
            
        case 2: // Tiempo Infinito
            infiniteTime_ = !infiniteTime_;
            std::cout << "Infinite Time: " << (infiniteTime_ ? "ON" : "OFF") << std::endl;
            break;
            
        case 3: // NoClip (atravesar paredes)
            noClip_ = !noClip_;
            player_.setNoClip(noClip_);
            std::cout << "NoClip: " << (noClip_ ? "ON" : "OFF") << std::endl;
            break;
            
        case 4: // Añadir vida
            player_.addLife();
            std::cout << "Vida anadida. Vidas: " << player_.getLives() << std::endl;
            break;
            
        case 5: // Vidas máximas
            player_.setMaxLives();
            std::cout << "Vidas al maximo (10)" << std::endl;
            break;
            
        case 6: // Dar llave
            player_.setHasKey(true);
            std::cout << "Llave obtenida" << std::endl;
            break;
            
        case 7: // Añadir 30 segundos
            levelTime_ += 30.0f;
            std::cout << "30 segundos anadidos" << std::endl;
            break;
            
        case 8: // Saltar nivel
            std::cout << "Saltando al siguiente nivel..." << std::endl;
            devMenuOpen_ = false;
            selectedDevOption_ = -1;
            // Pasar al siguiente nivel directamente
            this->state_machine->add_state(std::make_unique<MainGameState>(level_ + 1), true);
            break;
            
        case 9: // Resetear cheats
            godMode_ = false;
            freezeEnemies_ = false;
            infiniteTime_ = false;
            noClip_ = false;
            player_.setGodMode(false);
            player_.setNoClip(false);
            player_.resetLives();
            player_.setHasKey(false);
            std::cout << "Todos los cheats reseteados" << std::endl;
            break;
    }
}

void MainGameState::renderDevMenu() {
    Vector2 mousePos = GetMousePosition();
    
    // Fondo semitransparente
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.7f));
    
    int menuX = GetScreenWidth() / 2 - 250;
    int menuY = 100;
    int menuWidth = 500;
    int menuHeight = 520;
    
    // Panel del menú
    DrawRectangle(menuX, menuY, menuWidth, menuHeight, DARKGRAY);
    DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, GOLD);
    
    if (awaitingPassword_) {
        // Pantalla de contraseña
        DrawText("DEVELOPER MODE", menuX + 100, menuY + 20, 30, GOLD);
        DrawText("Introduce la contrasena:", menuX + 80, menuY + 80, 20, WHITE);
        
        // Campo de contraseña (ocultar con asteriscos)
        char masked[32] = {0};
        for (size_t i = 0; i < passwordInput_.length() && i < 31; i++) {
            masked[i] = '*';
        }
        DrawRectangle(menuX + 50, menuY + 120, 400, 40, LIGHTGRAY);
        DrawRectangleLines(menuX + 50, menuY + 120, 400, 40, DARKGRAY);
        DrawText(masked, menuX + 60, menuY + 128, 25, BLACK);
        
        // Botones clickeables
        Rectangle confirmBtn = { (float)(menuX + 150), (float)(menuY + 240), 200.0f, 40.0f };
        Rectangle cancelBtn = { (float)(menuX + 150), (float)(menuY + 290), 200.0f, 40.0f };
        
        bool confirmHover = CheckCollisionPointRec(mousePos, confirmBtn);
        bool cancelHover = CheckCollisionPointRec(mousePos, cancelBtn);
        
        // Botón Confirmar
        DrawRectangleRec(confirmBtn, confirmHover ? DARKGREEN : GREEN);
        DrawRectangleLinesEx(confirmBtn, 2.0f, BLACK);
        DrawText("CONFIRMAR", menuX + 175, menuY + 248, 20, WHITE);
        
        // Botón Cancelar
        DrawRectangleRec(cancelBtn, cancelHover ? MAROON : RED);
        DrawRectangleLinesEx(cancelBtn, 2.0f, BLACK);
        DrawText("CANCELAR", menuX + 180, menuY + 298, 20, WHITE);
        
        DrawText("Usa teclado o haz clic en los botones", menuX + 90, menuY + 350, 16, LIGHTGRAY);
        
    } else if (devMenuOpen_) {
        // Menú de opciones
        DrawText("DEVELOPER MENU", menuX + 120, menuY + 20, 30, GOLD);
        DrawText("Haz clic o usa UP/DOWN y ENTER", menuX + 80, menuY + 60, 16, LIGHTGRAY);
        
        const char* options[] = {
            "1. God Mode (Invulnerabilidad)",
            "2. Congelar Enemigos",
            "3. Tiempo Infinito",
            "4. NoClip (Atravesar Paredes)",
            "5. Anadir +1 Vida",
            "6. Vidas Maximas (10)",
            "7. Obtener Llave",
            "8. Anadir +30 Segundos",
            "9. Saltar Nivel",
            "0. Resetear Todos los Cheats"
        };
        
        // Obtener estado de llave una sola vez antes del loop
        bool hasKey = player_.hasKey();
        
        for (int i = 0; i < 10; i++) {
            Rectangle optionRect = { 
                (float)(menuX + 30), 
                (float)(menuY + 100 + i * 38), 
                440.0f, 
                35.0f 
            };
            
            bool isHovered = CheckCollisionPointRec(mousePos, optionRect);
            bool isSelected = (i == selectedDevOption_);
            
            // Fondo de la opción
            if (isHovered || isSelected) {
                DrawRectangleRec(optionRect, Fade(YELLOW, 0.2f));
            }
            
            Color textColor = (isHovered || isSelected) ? YELLOW : WHITE;
            Color stateColor = (isHovered || isSelected) ? GOLD : LIGHTGRAY;
            
            DrawText(options[i], menuX + 35, menuY + 102 + i * 38, 20, textColor);
            
            // Mostrar estados actuales
            const char* state = "";
            if (i == 0) state = godMode_ ? "[ON]" : "[OFF]";
            else if (i == 1) state = freezeEnemies_ ? "[ON]" : "[OFF]";
            else if (i == 2) state = infiniteTime_ ? "[ON]" : "[OFF]";
            else if (i == 3) state = noClip_ ? "[ON]" : "[OFF]";
            else if (i == 6) state = hasKey ? "[SI]" : "[NO]";
            
            DrawText(state, menuX + 400, menuY + 102 + i * 38, 20, stateColor);
        }
        
        // Botón cerrar clickeable
        Rectangle closeBtn = { (float)(menuX + 150), (float)(menuY + 490), 200.0f, 25.0f };
        bool closeHover = CheckCollisionPointRec(mousePos, closeBtn);
        
        DrawRectangleRec(closeBtn, closeHover ? Fade(RED, 0.3f) : Fade(DARKGRAY, 0.3f));
        DrawRectangleLines(menuX + 150, menuY + 490, 200, 25, closeHover ? RED : LIGHTGRAY);
        DrawText("CERRAR (ESC)", menuX + 170, menuY + 493, 18, closeHover ? RED : LIGHTGRAY);
    }
}
