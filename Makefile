CXX = g++
LD = g++
MAKE = make
CFLAGS = -g -Wall --std=c++11
LDFLAGS = 
LIBS = `pkg-config --static --libs x11 xrandr xi xxf86vm glew glfw3`

INCLUDE = 
BIN_DIR = bin
OBJ_DIR = obj
SHADER_DIR = shader

OUT_BIN = mandelbrot

ifeq ($(OS),Windows_NT)
	LIBS := -lmingw32 -lglfw3 -lglew32 -lopengl32
	OUT_BIN := $(OUT_BIN).exe
	LDFLAGS += -mwindows
endif

DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.Td

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(patsubst src/%,$(OBJ_DIR)/%.o,$(basename $(SOURCES)))

SHADERS = $(wildcard $(SHADER_DIR)/*.glsl)
SHADERS_LD = -Wl,--format=binary $(SHADERS) -Wl,--format=default

all: $(BIN_DIR)/$(OUT_BIN)

clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)

$(BIN_DIR)/$(OUT_BIN): $(OBJECTS) $(SHADERS)
	@mkdir -p $(BIN_DIR)
	$(LD) -o $(BIN_DIR)/$(OUT_BIN) $(OBJECTS) $(SHADERS_LD) $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o : src/%.cpp
$(OBJ_DIR)/%.o : src/%.cpp $(OBJ_DIR)/%.d
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(DEPFLAGS) $(CFLAGS) -c $(addprefix -I,$(INCLUDE)) -o $@ $<
	@mv -f $(OBJ_DIR)/$*.Td $(OBJ_DIR)/$*.d

$(OBJ_DIR)/%.d: ;
.PRECIOUS: $(OBJ_DIR)/%.d

-include $(patsubst src/%,$(OBJ_DIR)/%.d,$(basename $(SOURCES)))