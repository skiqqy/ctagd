CC=gcc
FLAGS=-Wall -Wno-variadic-macros -Wno-overlength-strings -pedantic
VERSION=0.1
NAME=ctagd

all: build

clean:
	rm -rf bin

build:
	mkdir -p bin
	$(CC) $(FLAGS) -o bin/$(NAME) src/ctagd.c
