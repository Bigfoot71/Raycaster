OBJS   = main.o window.o clock.o raycast.o map.o textures.o text.o
SOURCE = src/main.c src/window.c src/clock.c src/raycast.c src/map.c src/textures.c src/text.c
HEADER = src/window.h src/clock.h src/raycast.h src/map.h src/textures.h src/text.h src/color.h

CC      = gcc
EXEC    = Raycaster
CFLAGS  = -c -W -Werror -Wall  -Wextra
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm

all: $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

main.o: src/main.c
	$(CC) $(CFLAGS) src/main.c

window.o: src/window.c
	$(CC) $(CFLAGS) src/window.c

clock.o: src/clock.c
	$(CC) $(CFLAGS) src/clock.c

raycast.o: src/raycast.c
	$(CC) $(CFLAGS) src/raycast.c

map.o: src/map.c
	$(CC) $(CFLAGS) src/map.c

textures.o: src/textures.c
	$(CC) $(CFLAGS) src/textures.c

text.o: src/text.c
	$(CC) $(CFLAGS) src/text.c

clean:
	rm -rf $(OBJS)

mrproper: clean
	rm -rf $(EXEC)

run: $(EXEC)
	./$(EXEC)
