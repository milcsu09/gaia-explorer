
all:
	g++ -std=c++17 -Wall -Wextra -Wpedantic -Ofast -ffast-math -mfma -march=native main.cc -lsfml-graphics -lsfml-window -lsfml-system -lGL -lGLEW

