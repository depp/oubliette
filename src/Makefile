main_exe	:= Oubliette

all: $(main_exe)

clean:
	rm -f $(main_exe) *.d *.o game/*.d game/*.o

.PHONY: all clean

-include $(wildcard *.d)
-include $(wildcard game/*.d)

sdl_libs	:= $(shell pkg-config --libs sdl2 SDL2_image)
sdl_cflags	:= $(shell pkg-config --cflags sdl2 SDL2_image)
glew_libs	:= $(shell pkg-config --libs glew)
glew_cflags	:= $(shell pkg-config --cflags glew)

depflags	= -MF $(patsubst %.o,%.d,$@) -MMD -MP
warning_flags	= -Wall -Wextra -Wpointer-arith -Wformat-nonliteral
CXXFLAGS	= -O0 -g -std=c++11 $(warning_flags) $(depflags) $(glew_cflags)

sources := file.cpp image.cpp main.cpp pack.cpp rand.cpp shader.cpp sprite_array.cpp sprite_orientation.cpp sprite_sheet.cpp surface.cpp game/control.cpp game/entity.cpp game/graphics.cpp game/levelmap.cpp game/state.cpp game/vec.cpp

main.o sprite_sheet.o surface.o image.o: CXXFLAGS += $(sdl_cflags)

Oubliette: $(patsubst %.cpp,%.o,$(sources))
	$(CXX) $(LDFLAGS) -o $@ $^ $(sdl_libs) $(glew_libs) -lGL
