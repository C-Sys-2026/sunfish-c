
CC = gcc
CPPFLAGS ?= -Iinclude -Isrc
CFLAGS ?= -Wall -Wextra -Wpedantic -std=c11
LDFLAGS ?=

# ==========================================
# Operating system detection
# ==========================================

ifeq ($(OS),Windows_NT)
    EXE_EXT := .exe
    PYTHON := python
    LDLIBS ?= -lwinpthread
    MUNIT_FLAGS := -DPSNIP_CLOCK_NO_LIBRT

    MKDIR = cmd /C "if not exist build mkdir build"
    CLEAN = cmd /C "if exist build rmdir /s /q build"
    RUN_TESTS = .\build\tests.exe
else
    EXE_EXT :=
    PYTHON := python3
    LDLIBS ?= -pthread
    MUNIT_FLAGS := -pthread

    MKDIR = mkdir -p build
    CLEAN = rm -rf build
    RUN_TESTS = ./build/tests
endif

# ==========================================
# Engine
# ==========================================

TARGET := build/sunfish-c$(EXE_EXT)

SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:src/%.c=build/%.o)

# ==========================================
# Tests
# ==========================================

ENGINE_TEST_SOURCES := $(filter-out src/main.c,$(wildcard src/*.c))

TEST_GENERATOR := scripts/generate_tests.py
GENERATED_TEST_MAIN := tests/test_main.c

TEST_SOURCES := $(filter-out $(GENERATED_TEST_MAIN),$(wildcard tests/*.c))
TEST_SOURCES += $(GENERATED_TEST_MAIN)

TEST_OBJECTS := $(ENGINE_TEST_SOURCES:src/%.c=build/test-src-%.o)
TEST_OBJECTS += $(TEST_SOURCES:tests/%.c=build/test-%.o)
TEST_OBJECTS += build/test-munit.o

TEST_TARGET := build/tests$(EXE_EXT)

# ==========================================
# Build targets
# ==========================================

.PHONY: all debug release test generate-tests clean FORCE

all: debug

debug: CFLAGS += -O0 -g3
debug: $(TARGET)

release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

test: CFLAGS += -O0 -g3 -w
test: $(TEST_TARGET)
	$(RUN_TESTS)

# ==========================================
# Automatic test generation
# ==========================================

generate-tests: $(GENERATED_TEST_MAIN)

$(GENERATED_TEST_MAIN): FORCE
	@$(PYTHON) $(TEST_GENERATOR)

FORCE:

# ==========================================
# Test compilation
# ==========================================

$(TEST_TARGET): $(TEST_OBJECTS)
	@$(MKDIR)
	@$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

build/test-src-%.o: src/%.c
	@$(MKDIR)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -Isrc -MMD -MP -c $< -o $@

build/test-%.o: tests/%.c
	@$(MKDIR)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -Isrc -MMD -MP -c $< -o $@

build/test-munit.o: third_party/munit/munit.c
	@$(MKDIR)
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(MUNIT_FLAGS) -MMD -MP -c $< -o $@

# ==========================================
# Engine compilation
# ==========================================

$(TARGET): $(OBJECTS)
	@$(MKDIR)
	@$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

build/%.o: src/%.c
	@$(MKDIR)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

# ==========================================
# Dependencies and cleanup
# ==========================================

-include $(OBJECTS:.o=.d)
-include $(TEST_OBJECTS:.o=.d)

clean:
	@$(CLEAN)
