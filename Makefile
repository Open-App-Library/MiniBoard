all: compile run

compile:
	gcc app.c `pkg-config gtk+-3.0 --cflags --libs` -o MiniBoard

run:
	./MiniBoard
