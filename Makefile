CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -Idrivers
LDFLAGS = -lm

BUILD_DIR = build
EXECUTABLE = $(BUILD_DIR)/thermostat
TEST_EXECUTABLE = $(BUILD_DIR)/thermostat_test

APP_SOURCES = app/main.c app/thermostat.c app/control_pi.c app/plant_sim.c app/fsm.c drivers/ringbuf.c
TEST_SOURCES = tests/run_tests.c app/thermostat.c app/control_pi.c app/plant_sim.c app/fsm.c drivers/ringbuf.c

APP_OBJECTS = $(APP_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_OBJECTS = $(TEST_SOURCES:%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run build test

all: build

build: $(EXECUTABLE)
	@echo "✓ Build successful: $(EXECUTABLE)"

$(EXECUTABLE): $(APP_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(APP_OBJECTS) -o $@ $(LDFLAGS)

run: build
	./$(EXECUTABLE)

test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): $(TEST_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(TEST_OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

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
