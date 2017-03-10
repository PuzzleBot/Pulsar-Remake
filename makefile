TARGET =
INCLUDES = -F/System/Library/Frameworks -framework OpenGL -framework GLUT -lm -Iinclude -Wno-deprecated-declarations

all: a3

a3: src/a1.c src/graphics.c src/visible.c src/wall.c src/projectile.c src/ui.c src/mobs.c src/world.c src/sight.c include/graphics.h
	gcc -g src/a1.c src/graphics.c src/visible.c src/wall.c src/projectile.c src/ui.c src/mobs.c src/world.c src/sight.c -o a3 $(INCLUDES) $(TARGET)

test:
	gcc -g src/wall.c tests/test.c src/graphics.c tests/oldMain.c src/visible.c $(INCLUDES) $(TARGET) -o tests/a1test

run:
	./a3

runTestworld:
	./a3 -testworld

testRun:
	./tests/a1test

clean:
	rm -r ./a3
