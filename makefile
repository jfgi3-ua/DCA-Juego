CXX := g++
CXXFLAGS := -std=c++17 -I src -I vendor/include
LDFLAGS  := -L vendor/lib
LDLIBS   := -lraylib -lGL -lm -lpthread -lrt -lX11
TARGET ?= game

.PHONY: all run clean
all:
	$(CXX) $(CXXFLAGS) -o $(TARGET) src/*.cpp $(LDFLAGS) $(LDLIBS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
