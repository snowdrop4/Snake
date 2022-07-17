ODIR = obj

CFLAGS = -g -std=c99 -Wall -O0
SDL = -lSDL -lSDL_image -lSDL_ttf -lSDL_gfx -no-pie

_MAIN = globals.o main.o game.o highscores.o menu.o sdlhelperfuncs.o
MAIN = $(patsubst %,$(ODIR)/%,$(_MAIN))

$(ODIR)/%.o: %.c
	gcc $(CFLAGS) -c -o $@ $< $(SDL)

main: $(MAIN)
	gcc $(CFLAGS) -o ../main $^ $(SDL)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o
