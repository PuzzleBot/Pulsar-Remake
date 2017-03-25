TARGET =
INCLUDES = -F/System/Library/Frameworks -framework OpenGL -framework GLUT -lm -Iinclude -Wno-deprecated-declarations
FILES = src/a1.c src/graphics.c src/visible.c src/wall.c src/projectile.c src/ui.c src/mobs.c src/world.c src/sight.c src/gameplay.c src/parabola.c

all: a3

a3: $(FILES) include/graphics.h
	gcc $(FILES) -g -o a4 $(INCLUDES) $(TARGET)

test:
	gcc -g src/wall.c tests/test.c src/graphics.c tests/oldMain.c src/visible.c $(INCLUDES) $(TARGET) -o tests/a1test

run:
	./a4

runTestworld:
	./a4 -testworld

testRun:
	./tests/a1test

clean:
	rm -r ./a4
