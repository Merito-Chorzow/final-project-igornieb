CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -Idrivers
LDFLAGS = -lm

BUILD_DIR = build
EXECUTABLE = $(BUILD_DIR)/thermostat

SOURCES = app/main.c app/thermostat.c app/control_pi.c app/plant_sim.c app/fsm.c drivers/ringbuf.c
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run build test

all: build

build: $(EXECUTABLE)
	@echo "✓ Build successful: $(EXECUTABLE)"

$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: build
	./$(EXECUTABLE)

test: build
	./$(EXECUTABLE) | tee tests/output.log

clean:
	rm -rf $(BUILD_DIR)
	@echo "✓ Cleaned"

help:
	@echo "Usage:"
	@echo "  make build  - Compile the project"
	@echo "  make run    - Compile and run"
	@echo "  make clean  - Remove build artifacts"
	@echo ""
	@echo "Detected OS: $(UNAME)"
	@echo "Build target: $(EXECUTABLE)"
