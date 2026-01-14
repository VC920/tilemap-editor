all:
	g++ main.cpp -o tilemap-editor -l SDL2 -l SDL2_image -l SDL2_ttf
run:
	./tilemap-editor
