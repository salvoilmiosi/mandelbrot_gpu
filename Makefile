CXX = g++
LD = g++
MAKE = make
CFLAGS = -Wall --std=c++1z
LDFLAGS = -s -O2
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
SHADER_OBJ = $(patsubst $(SHADER_DIR)/%,$(OBJ_DIR)/%.o,$(basename $(SHADERS)))

all: $(BIN_DIR)/$(OUT_BIN)

clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)

$(BIN_DIR)/$(OUT_BIN): $(OBJECTS) $(SHADER_OBJ)
	@mkdir -p $(BIN_DIR)
	$(LD) -o $(BIN_DIR)/$(OUT_BIN) $(OBJECTS) $(SHADER_OBJ) $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o : src/%.cpp
$(OBJ_DIR)/%.o : src/%.cpp $(OBJ_DIR)/%.d
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(DEPFLAGS) $(CFLAGS) -c $(addprefix -I,$(INCLUDE)) -o $@ $<
	@mv -f $(OBJ_DIR)/$*.Td $(OBJ_DIR)/$*.d

$(OBJ_DIR)/%.o : shader/%.glsl
	@mkdir -p $(OBJ_DIR)
	objcopy -I binary -O elf64-x86-64 -B i386 $< $@

$(OBJ_DIR)/%.d: ;
.PRECIOUS: $(OBJ_DIR)/%.d

-include $(patsubst src/%,$(OBJ_DIR)/%.d,$(basename $(SOURCES)))