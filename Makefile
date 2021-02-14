CC=gcc
FLAGS=-Wall -Wno-variadic-macros -Wno-overlength-strings -pedantic
VERSION=0.1
NAME=ctagd
COMP=$(CC) $(FLAGS)

all: build ctagd.o server client

clean:
	rm -rf bin

build:
	mkdir -p bin

ctagd.o: src/ctagd.c
	$(COMP) -c $< -o bin/$@

# The following two units are just for testing

server: ctagd.o
	$(COMP) -o bin/$@ src/server.c bin/ctagd.o

client: ctagd.o
	$(COMP) -o bin/$@ src/client.c bin/ctagd.o
