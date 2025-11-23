CC = gcc
CFLAGS = -I/usr/include/SDL2 -I/usr/include/SDL2_ttf -D_REENTRANT
LDFLAGS = -lSDL2 -lSDL2_ttf
object_files = $(patsubst %.o, $(BUILD_DIR)/%.o, main.o cartridge.o cpu.o emulator.o ram.o stack.o utilities.o)

BUILD_DIR = ./build

all: $(BUILD_DIR)/all

$(BUILD_DIR)/all: $(object_files)
	$(CC) $(CFLAGS) $(object_files) -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR)/*

