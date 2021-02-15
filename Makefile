CC=gcc
FLAGS=-Wall -Wno-variadic-macros -Wno-overlength-strings -pedantic
VERSION=0.1
NAME=ctagd
COMP=$(CC) $(FLAGS)

all: build

clean:
	rm -rf bin

init:
	mkdir -p bin

build: init ctagd.o

ctagd.o: src/ctagd.c
	$(COMP) -c $< -o bin/$@

lib: build
	ar rcs bin/ctagd.a bin/ctagd.o

# The following two units are just for testing

test: build server client
	./test.sh

server: ctagd.o
	$(COMP) -o bin/$@ src/server.c bin/ctagd.o

client: ctagd.o
	$(COMP) -o bin/$@ src/client.c bin/ctagd.o
