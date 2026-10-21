CC      ?= cc
CFLAGS  ?= -std=c99 -O2 -Wall -Wextra -Werror

SRC = src/smartbeacon.c src/nmea_rmc.c test/test_main.c

test: build/test
	./build/test

build/test: $(SRC) src/*.h
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $@

clean:
	rm -rf build

.PHONY: test clean
