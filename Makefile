all: clean build run

clean:
	rm -f a.out

build: 
	g++ Main.cpp -lglut -lOpenGL -lglfw -lGLEW -lGL

run:
	./a.out