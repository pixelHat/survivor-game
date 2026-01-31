# Variables
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra $(shell sdl2-config --cflags --libs) $(shell sdl2-config --libs) -lSDL2_tff -lSDL2_image
LIBS = $(shell sdl2-config --cflags --libs) $(shell sdl2-config --libs) -lSDL2_ttf -lSDL2_image
SRC = main.cpp Player.cpp Enemy.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = game

# The default rule
all: $(TARGET)

# Link the object files into the final executable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LIBS) && ./game

# Compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJ) $(TARGET)
