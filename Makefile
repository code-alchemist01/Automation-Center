# ========================================
# Sistem Otomasyon Merkezi - Makefile
# ========================================

# Compiler ve flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
DEBUG_FLAGS = -g -DDEBUG
INCLUDES = -Iinclude -Ilib/cJSON -Ilib/pthread-win32
LIBS = -lm -Llib/cJSON -lcjson

# Platform bağımlı ayarlar
ifeq ($(OS),Windows_NT)
    LIBS += -lws2_32 -lpdh -liphlpapi -licmp -lpsapi -lnetapi32
    EXECUTABLE = automation_center.exe
    RM = del /Q
    MKDIR = mkdir
    PATH_SEP = \\
else
    LIBS += -lpthread -lcurl
    EXECUTABLE = automation_center
    RM = rm -f
    MKDIR = mkdir -p
    PATH_SEP = /
endif

# Dizinler
SRC_DIR = src
BUILD_DIR = build
CORE_DIR = $(SRC_DIR)/core
MODULES_DIR = $(SRC_DIR)/modules
UTILS_DIR = $(SRC_DIR)/utils
GUI_DIR = $(SRC_DIR)/gui

# Kaynak dosyaları
CORE_SOURCES = $(wildcard $(SRC_DIR)/core/*.c)
MODULE_SOURCES = $(wildcard $(SRC_DIR)/modules/*.c)
UTIL_SOURCES = $(wildcard $(SRC_DIR)/utils/*.c)
GUI_SOURCES = $(wildcard $(SRC_DIR)/gui/*.c)
MAIN_SOURCE = $(SRC_DIR)/main.c

# Object dosyaları
CORE_OBJECTS = $(CORE_SOURCES:$(SRC_DIR)/core/%.c=$(BUILD_DIR)/core/%.o)
MODULE_OBJECTS = $(MODULE_SOURCES:$(SRC_DIR)/modules/%.c=$(BUILD_DIR)/modules/%.o)
UTIL_OBJECTS = $(UTIL_SOURCES:$(SRC_DIR)/utils/%.c=$(BUILD_DIR)/utils/%.o)
GUI_OBJECTS = $(GUI_SOURCES:$(SRC_DIR)/gui/%.c=$(BUILD_DIR)/gui/%.o)
MAIN_OBJECT = $(BUILD_DIR)/main.o

ALL_OBJECTS = $(CORE_OBJECTS) $(MODULE_OBJECTS) $(UTIL_OBJECTS) $(GUI_OBJECTS) $(MAIN_OBJECT)

# Ana hedef
all: directories $(EXECUTABLE)

# Dizinleri oluştur
directories:
ifeq ($(OS),Windows_NT)
	@if not exist "$(BUILD_DIR)" $(MKDIR) "$(BUILD_DIR)"
	@if not exist "$(BUILD_DIR)\\core" $(MKDIR) "$(BUILD_DIR)\\core"
	@if not exist "$(BUILD_DIR)\\modules" $(MKDIR) "$(BUILD_DIR)\\modules"
	@if not exist "$(BUILD_DIR)\\utils" $(MKDIR) "$(BUILD_DIR)\\utils"
	@if not exist "$(BUILD_DIR)\\gui" $(MKDIR) "$(BUILD_DIR)\\gui"
else
	@$(MKDIR) $(BUILD_DIR)/core $(BUILD_DIR)/modules $(BUILD_DIR)/utils $(BUILD_DIR)/gui
endif

# Ana program
$(EXECUTABLE): $(ALL_OBJECTS)
	@echo "Linking $(EXECUTABLE)..."
	$(CC) $(ALL_OBJECTS) -o $(EXECUTABLE) $(LIBS)
	@echo "Build completed successfully!"

# Ana dosya
$(MAIN_OBJECT): $(MAIN_SOURCE)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Core dosyalar
$(BUILD_DIR)/core/%.o: $(CORE_DIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Modül dosyalar
$(BUILD_DIR)/modules/%.o: $(MODULES_DIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Utility dosyalar
$(BUILD_DIR)/utils/%.o: $(UTILS_DIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# GUI dosyalar
$(BUILD_DIR)/gui/%.o: $(GUI_DIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: directories $(EXECUTABLE)

# Temizlik
clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)"
	@if exist "$(EXECUTABLE)" $(RM) "$(EXECUTABLE)"
else
	$(RM) -r $(BUILD_DIR)
	$(RM) $(EXECUTABLE)
endif
	@echo "Clean completed!"

# Yeniden build
rebuild: clean all

# Test build (sadece derleme kontrolü)
test-compile: directories
	@echo "Testing compilation..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $(MAIN_SOURCE) -o $(BUILD_DIR)/test_main.o
	@for %%f in ($(CORE_SOURCES)) do $(CC) $(CFLAGS) $(INCLUDES) -c %%f -o $(BUILD_DIR)/test_core_%%~nf.o
	@echo "Compilation test passed!"
ifeq ($(OS),Windows_NT)
	@$(RM) "$(BUILD_DIR)\\test_*.o"
else
	@$(RM) $(BUILD_DIR)/test_*.o
endif

# Kurulum
install: $(EXECUTABLE)
ifeq ($(OS),Windows_NT)
	@echo "Windows installation not implemented yet"
else
	cp $(EXECUTABLE) /usr/local/bin/
	@echo "Installation completed!"
endif

# Kaldırma
uninstall:
ifeq ($(OS),Windows_NT)
	@echo "Windows uninstallation not implemented yet"
else
	rm -f /usr/local/bin/$(EXECUTABLE)
	@echo "Uninstallation completed!"
endif

# Çalıştır
run: $(EXECUTABLE)
	@echo "Running $(EXECUTABLE)..."
	./$(EXECUTABLE)

# Yardım
help:
	@echo "Available targets:"
	@echo "  all       - Build the project (default)"
	@echo "  debug     - Build with debug information"
	@echo "  clean     - Remove build files"
	@echo "  rebuild   - Clean and build"
	@echo "  test-compile - Test compilation only"
	@echo "  run       - Build and run the program"
	@echo "  install   - Install the program (Linux/Mac)"
	@echo "  uninstall - Uninstall the program (Linux/Mac)"
	@echo "  help      - Show this help message"

# Phony targets
.PHONY: all directories debug clean rebuild test-compile install uninstall run help

# Bağımlılıklar
$(MAIN_OBJECT): include/core.h include/logger.h include/config.h include/menu.h
$(CORE_OBJECTS): include/core.h include/logger.h include/config.h