CXX = g++
LD = g++
MAKE = make
CFLAGS = -g -Wall --std=c++11

LDFLAGS =
LIBS = `pkg-config --static --libs x11 xrandr xi xxf86vm glew glfw3`

INCLUDE = -Iinclude -I$(RESPACK_DIR)/include
BIN_DIR = ../bin
OBJ_DIR = obj

OUT_BIN = mandelbrot

ifeq ($(OS),Windows_NT)
	LIBS := -lmingw32 -lglfw3 -lglew32 -lopengl32
	OUT_BIN := $(OUT_BIN).exe
	LDFLAGS += -mwindows
endif

$(shell mkdir -p $(BIN_DIR) >/dev/null)
$(shell mkdir -p $(OBJ_DIR) >/dev/null)

DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.Td

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(patsubst src/%,$(OBJ_DIR)/%.o,$(basename $(SOURCES))) $(RESLOAD)

all: $(BIN_DIR)/$(OUT_BIN)

clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)
	$(MAKE) -C $(RESPACK_DIR) clean

$(RESLOAD): $(RESPACK)
$(RESPACK):
	$(MAKE) -C $(RESPACK_DIR)

$(BIN_DIR)/$(OUT_BIN): $(OBJECTS)
	$(LD) -o $(BIN_DIR)/$(OUT_BIN) $(OBJECTS) $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o : src/%.cpp
$(OBJ_DIR)/%.o : src/%.cpp $(OBJ_DIR)/%.d
	$(CXX) $(DEPFLAGS) $(CFLAGS) -c $(INCLUDE) -o $@ $<
	@mv -f $(OBJ_DIR)/$*.Td $(OBJ_DIR)/$*.d

$(OBJ_DIR)/%.d: ;
.PRECIOUS: $(OBJ_DIR)/%.d

-include $(patsubst src/%,$(OBJ_DIR)/%.d,$(basename $(SOURCES)))