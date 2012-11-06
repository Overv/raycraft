# Parameters

CC = g++
CCC = gcc
CCFLAGS = -O3 -std=c++0x

# Program

bin/raycraft: bin bin/main.o bin/world.o bin/renderer.o bin/gl3w.o bin/renderer.vert bin/renderer.frag bin/materials.png
	$(CC) $(CCFLAGS) bin/main.o bin/renderer.o bin/world.o bin/gl3w.o -o bin/raycraft -lglfw -lSOIL

bin/main.o: src/main.cpp
	$(CC) $(CCFLAGS) -c -I include -I extlibs src/main.cpp -o bin/main.o

bin/renderer.o: src/renderer.cpp
	$(CC) $(CCFLAGS) -c -I include -I extlibs src/renderer.cpp -o bin/renderer.o

bin/world.o: src/world.cpp
	$(CC) $(CCFLAGS) -c -I include -I extlibs src/world.cpp -o bin/world.o

# Resources

bin/renderer.vert: src/renderer.vert
	cp src/renderer.vert bin/renderer.vert

bin/renderer.frag: src/renderer.frag
	cp src/renderer.frag bin/renderer.frag

bin/materials.png: res/materials.png
	cp res/materials.png bin/materials.png

# External dependencies

bin/gl3w.o: extlibs/GL3/gl3w.c
	$(CCC) -c -I extlibs extlibs/GL3/gl3w.c -o bin/gl3w.o

# Binaries folder

bin:
	mkdir bin

# Clean up

clean:
	rm -rf bin