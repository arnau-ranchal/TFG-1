# Makefile simplificado y compatible

# Compilador y flags
CXX := g++
CXXFLAGS := -c -fPIC
LDFLAGS := -shared

# Directorios
BUILD_DIR := build

# Fuentes y objetos
SOURCES := exponents/exponents.cpp graphics/graphics.cpp
OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(SOURCES)))

# Objetivo principal
all: $(BUILD_DIR)/libfunctions.so

# Regla para la biblioteca
$(BUILD_DIR)/libfunctions.so: $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(LDFLAGS) -o $@ $^

# Regla genérica para objetos
$(BUILD_DIR)/%.o: exponents/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/%.o: graphics/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

# Limpieza
clean:
	rm -rf $(BUILD_DIR)