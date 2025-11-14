TARGET   := game

PREFIX ?= /usr	#por defecto para debian
#respetamos FHS
BINDIR  := $(PREFIX)/bin
DATADIR := $(PREFIX)/share/$(TARGET)
DESTDIR ?=

SRC_DIR    := src
ASSETS_DIR := assets

CXX := g++
CXXFLAGS := -std=c++17 -I $(SRC_DIR) -I vendor/include
LDFLAGS  := -L vendor/lib
LDLIBS   := -lraylib -lGL -lm -lpthread -lrt -lX11

.PHONY: all run clean install
all:
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC_DIR)/*.cpp $(LDFLAGS) $(LDLIBS)

run: all
	./$(TARGET)

install: all
	# Instalar ejecutable
	install -D -m 0755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

	# Instalar assets
	install -d $(DESTDIR)$(DATADIR)/assets
	cp -r $(ASSETS_DIR)/* $(DESTDIR)$(DATADIR)/assets/

clean:
	rm -f $(TARGET)

