CC = gcc
CPPFLAGS ?= -Iinclude
CFLAGS ?= -Wall -Wextra -Wpedantic -std=c11
LDFLAGS ?=
LDLIBS ?=

TARGET := build/sunfish-c
SOURCES := src/main.c
OBJECTS := $(SOURCES:src/%.c=build/%.o)

.PHONY: all debug release clean

all: debug

debug: CFLAGS += -O0 -g3
debug: $(TARGET)

release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

$(TARGET): $(OBJECTS)
	@cmd /C "if not exist $(@D) mkdir $(@D)"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

build/%.o: src/%.c
	@cmd /C "if not exist $(@D) mkdir $(@D)"
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(OBJECTS:.o=.d)

clean:
	cmd /C "if exist build rmdir /s /q build"
