TARGET = -mmacosx-version-min=10.8
INCLUDES = -F/System/Library/Frameworks -framework OpenGL -framework GLUT -lm -Iinclude

all: a3

a1: src/a1.c src/graphics.c src/visible.c src/wall.c src/projectile.c src/ui.c include/graphics.h
	gcc src/a1.c src/graphics.c src/visible.c src/wall.c src/projectile.c src/ui.c -o a3 $(INCLUDES) $(TARGET)

test:
	gcc -g src/wall.c tests/test.c src/graphics.c tests/oldMain.c src/visible.c $(INCLUDES) $(TARGET) -o tests/a1test

run:
	./a3

runTestworld:
	./a3 -testworld

testRun:
	./tests/a1test