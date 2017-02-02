CC        := g++
INCLUDES  := -Iinclude/json/src -Iinclude/GSL/include -Iinclude/glm -I/usr/include/freetype2
LD_FLAGS  := -O2
LD_LIBS   := -lGLU -lGL -lGLEW -lglut -lSDL2 -lSDL2_image `freetype-config --libs` -lpthread
CC_FLAGS  := -std=c++14 $(INCLUDES)
CPP_FILES := $(wildcard src/*.cc)
OBJ_FILES := $(addprefix build/,$(notdir $(CPP_FILES:.cc=.o)))

videre: $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -o $@ $^ $(LD_LIBS)

build/%.o: src/%.cc
	$(CC) $(CC_FLAGS) -c -o $@ $<

clean:
	rm -rf build/*

