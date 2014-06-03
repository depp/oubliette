main_exe	:= Oubliette

all: $(main_exe)

clean:
	rm -f $(main_exe) base/*.d base/*.o game/*.d game/*.o

.PHONY: all clean

-include $(wildcard base/*.d)
-include $(wildcard game/*.d)

sdl_libs	:= $(shell pkg-config --libs sdl2 SDL2_image SDL2_mixer)
sdl_cflags	:= $(shell pkg-config --cflags sdl2 SDL2_image SDL2_mixer)
glew_libs	:= $(shell pkg-config --libs glew)
glew_cflags	:= $(shell pkg-config --cflags glew)

depflags	= -MF $(patsubst %.o,%.d,$@) -MMD -MP
warning_flags	= -Wall -Wextra -Wpointer-arith -Wformat-nonliteral
CXXFLAGS	= -O0 -g
override CXXFLAGS += -I. -std=c++11 $(warning_flags) $(depflags) $(glew_cflags)

sources := base/file.cpp base/image.cpp base/main.cpp base/pack.cpp base/rand.cpp base/shader.cpp base/sprite_array.cpp base/sprite_orientation.cpp base/sprite_sheet.cpp base/surface.cpp base/vec.cpp game/audio.cpp game/camera.cpp game/color.cpp game/control.cpp game/editor.cpp game/entity.cpp game/graphics.cpp game/leveldata.cpp game/levelmap.cpp game/script.cpp game/sprite.cpp game/state.cpp game/stats.cpp

base/main.o base/sprite_sheet.o base/surface.o base/image.o game/audio.o: CXXFLAGS += $(sdl_cflags)

Oubliette: $(patsubst %.cpp,%.o,$(sources))
	$(CXX) $(LDFLAGS) -o $@ $^ $(sdl_libs) $(glew_libs) -lGL
