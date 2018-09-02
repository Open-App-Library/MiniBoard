all: compile-ui cmake compile-project

compile-ui:
	glib-compile-resources miniboard.gresource.xml --target=src/resources.c --generate-source

cmake:
	mkdir -p build
	cd build; cmake ..

compile-project:
	cd build; make

run:
	./build/src/MiniBoard

compile-and-run: compile-project run

compile-everything-and-run: all run
