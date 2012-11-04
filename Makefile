# Parameters

CC = g++
CCC = gcc
CCFLAGS = -std=c++0x

# Program

bin/raycraft: bin/main.o bin/world.o
	$(CC) $(CCFLAGS) bin/main.o bin/world.o -o bin/raycraft -lglfw

bin/main.o: bin src/main.cpp
	$(CC) $(CCFLAGS) -c -I include src/main.cpp -o bin/main.o

bin/world.o: bin src/world.cpp
	$(CC) $(CCFLAGS) -c -I include src/world.cpp -o bin/world.o

# Binaries folder

bin:
	mkdir bin

# Clean up

clean:
	rm -rf bin/*