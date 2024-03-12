all: project

project: proj.cpp
	g++ -o project proj.cpp -lGL -lGLU -lglut 

clean:
	rm -f project

