CC ?= cc
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null || echo -I/opt/homebrew/opt/raylib/include)
RAYLIB_LIBS := $(shell pkg-config --libs raylib 2>/dev/null || echo -L/opt/homebrew/opt/raylib/lib -lraylib)
CFLAGS ?= -Wall -Wextra -Wpedantic -std=c11 -Isrc $(RAYLIB_CFLAGS)

.PHONY: all run test clean

all: main

SOURCES := $(shell find src -name '*.c')

main: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o main $(RAYLIB_LIBS) -lm

run: main
	./main

tests/building_network_test: tests/building_network_test.c src/build/building.c src/build/building.h src/economy/economy.c src/economy/economy.h src/render/game_visuals.c src/render/game_visuals.h
	$(CC) $(CFLAGS) tests/building_network_test.c src/build/building.c src/economy/economy.c src/render/game_visuals.c -o tests/building_network_test $(RAYLIB_LIBS) -lm

test: tests/building_network_test
	./tests/building_network_test

clean:
	rm -f main tests/building_network_test
