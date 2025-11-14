# =========================
# Colores para mensajes
# =========================
GREEN  := \033[1;32m
YELLOW := \033[1;33m
BLUE   := \033[1;34m
RED    := \033[1;31m
RESET  := \033[0m

# =========================
# Configuración básica
# =========================
CXX       := g++
APP_NAME  := game

# Rutas del proyecto
SRC_DIR   := src
OBJ_DIR   := obj
BIN_DIR   := bin
LIB_DIR   := vendor/lib
VENDOR_INC_DIR := vendor/include

# =========================
# Descubrir fuentes e includes
# =========================

# Buscar todos los .cpp recursivamente dentro de src/
SRC  := $(shell find $(SRC_DIR) -type f -name '*.cpp')

# Generar los .o correspondientes en obj/ con la misma estructura
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

# Incluir recursivamente todos los subdirectorios de src/ y vendor/include/
INC_DIRS    := $(shell find $(SRC_DIR) -type d)
INC_VENDORS := $(shell find $(VENDOR_INC_DIR) -type d 2>/dev/null)
INC_FLAGS   := $(addprefix -I,$(INC_DIRS)) $(addprefix -I,$(INC_VENDORS))

# =========================
# Flags de compilación y enlace
# =========================
CXXSTD   := -std=c++17
WARNINGS := -Wall -Wextra -Wpedantic
OPT      := -O2
CXXFLAGS := $(CXXSTD) $(WARNINGS) $(OPT) $(INC_FLAGS)

LIB_DIRS := -L$(LIB_DIR)
LDFLAGS  := $(LIB_DIRS)

# Librerías (Linux + Raylib)
LDLIBS   := -lraylib -lGL -lm -lpthread -lrt -lX11

# =========================
# Raylib como dependencia
# =========================
RAYLIB     := libraylib.a
RAYLIB_DEP := $(LIB_DIR)/$(RAYLIB)

# =========================
# Objetivos phony
# =========================
.PHONY: all run clean distclean debug release help info raylib

# Regla por defecto: compilar en modo release
all: release

# =========================
# Build (release / debug)
# =========================
release: CXXFLAGS := $(CXXSTD) $(WARNINGS) -O2 -DNDEBUG $(INC_FLAGS)
release: $(BIN_DIR)/$(APP_NAME)

debug: CXXFLAGS := $(CXXSTD) $(WARNINGS) -O0 -g -DDEBUG $(INC_FLAGS)
debug: $(BIN_DIR)/$(APP_NAME)

# Enlace final
$(BIN_DIR)/$(APP_NAME): $(RAYLIB_DEP) $(OBJS)
	@echo "$(BLUE)[LD] Enlazando $(APP_NAME)...$(RESET)"
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
	@echo "$(GREEN)Ejecutable generado: $(BIN_DIR)/$(APP_NAME)$(RESET)"

# Compilación de cada .cpp a .o (crea obj/ y subcarpetas si no existen)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "$(YELLOW)[CXX] $< → $@$(RESET)"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# =========================
# Ejecutar el juego
# =========================
run: all
	@echo "$(BLUE)Ejecutando $(BIN_DIR)/$(APP_NAME)...$(RESET)"
	@./$(BIN_DIR)/$(APP_NAME)

# =========================
# Limpieza
# =========================
clean:
	@echo "$(RED)[CLEAN] Borrando objetos y binarios...$(RESET)"
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

distclean: clean
	@echo "$(RED)[CLEAN] Borrando dependencias descargadas...$(RESET)"
	@rm -rf dist

# =========================
# Info de depuración
# =========================
info:
	$(info SRC = $(SRC))
	$(info OBJS = $(OBJS))
	$(info INC_FLAGS = $(INC_FLAGS))
	$(info LIB_DIRS = $(LIB_DIRS))
	$(info RAYLIB_DEP = $(RAYLIB_DEP))

# =========================
# Descarga/compilación Raylib
# =========================
raylib: $(RAYLIB_DEP)

$(RAYLIB_DEP):
	@if [ ! -f "$(RAYLIB_DEP)" ]; then \
		echo "$(YELLOW)Raylib no está en $(RAYLIB_DEP). Descargando...$(RESET)"; \
		git clone --depth 1 https://github.com/raysan5/raylib.git; \
		$(MAKE) -C raylib/src/ PLATFORM=PLATFORM_DESKTOP; \
		mkdir -p $(LIB_DIR); \
		mv raylib/src/libraylib.a "$(LIB_DIR)/"; \
		rm -rf raylib; \
		echo "$(GREEN)Raylib instalada correctamente en $(RAYLIB_DEP).$(RESET)"; \
	else \
		echo "$(GREEN)Raylib ya instalada en $(RAYLIB_DEP).$(RESET)"; \
	fi

# =========================
# Ayuda rápida
# =========================
help:
	@echo "Comandos disponibles:"
	@echo "  make / make release  -> Compila en modo release"
	@echo "  make debug           -> Compila en modo debug"
	@echo "  make run             -> Compila (release) y ejecuta"
	@echo "  make clean           -> Borra obj/ y bin/"
	@echo "  make distclean       -> clean + borra dist/"
	@echo "  make info            -> Muestra fuentes, objetos e includes"
	@echo "  make raylib          -> Descarga/compila libraylib.a si falta"
