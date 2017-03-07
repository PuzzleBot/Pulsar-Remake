TARGET = -mmacosx-version-min=10.8
INCLUDES = -F/System/Library/Frameworks -framework OpenGL -framework GLUT -lm -Iinclude
CODEFILES = src/a1.c src/graphics.c src/visible.c src/wall.c src/projectile.c src/ui.c src/mobs.c

all: a1

a1: $(CODEFILES) include/graphics.h
	gcc $(CODEFILES) -o a1 $(INCLUDES) $(TARGET)

test:
	gcc -g src/wall.c tests/test.c src/graphics.c tests/oldMain.c src/visible.c $(INCLUDES) $(TARGET) -o tests/a1test

run:
	./a1

runTestworld:
	./a1 -testworld

testRun:
	./tests/a1test
