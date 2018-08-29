all: compile run

compile:
	gcc `pkg-config gtk+-3.0 --cflags --libs` app.c -o MiniBoard

run:
	./MiniBoard
