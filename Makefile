C_FILES=src/_main.c src/brush.c src/canvas.c src/gui.c

all: compile run

compile:
	gcc $(C_FILES) `pkg-config gtk+-3.0 --cflags --libs` -o MiniBoard

run:
	./MiniBoard
