CC = gcc
CPPFLAGS ?= -Iinclude
CFLAGS ?= -Wall -Wextra -Wpedantic -std=c11
LDFLAGS ?=
LDLIBS ?=

TARGET := build/sunfish-c
SOURCES := src/main.c
OBJECTS := $(SOURCES:src/%.c=build/%.o)
ENGINE_TEST_SOURCES := $(filter-out src/main.c,$(wildcard src/*.c))
TEST_SOURCES := $(wildcard tests/*.c) third_party/munit/munit.c
TEST_OBJECTS := $(ENGINE_TEST_SOURCES:src/%.c=build/test-src-%.o) \\
                $(filter-out build/test-main.o,$(TEST_SOURCES:tests/%.c=build/test-%.o)) \\
                build/test-main.o build/test-munit.o
TEST_TARGET := build/tests

.PHONY: all debug release test clean

all: debug

debug: CFLAGS += -O0 -g3
debug: $(TARGET)

release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

test: CFLAGS += -O0 -g3
test: $(TEST_TARGET)
	$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	@cmd /C "if not exist $(@D) mkdir $(@D)"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

build/test-src-%.o: src/%.c
	@cmd /C "if not exist $(@D) mkdir $(@D)"
	$(CC) $(CPPFLAGS) $(CFLAGS) -Isrc -MMD -MP -c $< -o $@

build/test-%.o: tests/%.c
	@cmd /C "if not exist $(@D) mkdir $(@D)"
	$(CC) $(CPPFLAGS) $(CFLAGS) -Isrc -MMD -MP -c $< -o $@

build/test-munit.o: third_party/munit/munit.c
	@cmd /C "if not exist $(@D) mkdir $(@D)"
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(TARGET): $(OBJECTS)
	@cmd /C "if not exist $(@D) mkdir $(@D)"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

build/%.o: src/%.c
	@cmd /C "if not exist $(@D) mkdir $(@D)"
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(OBJECTS:.o=.d)

clean:
	cmd /C "if exist build rmdir /s /q build"
