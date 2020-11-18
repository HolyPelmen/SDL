#include <SDL.h>
#include <iostream>
#include <vector>
#include <time.h>
#include "gen.h"
#define N 20

using namespace std;

//-------------------------------------------------------------------------------------------------------------------------------------
bool init(SDL_Window* win, SDL_Surface* screen, int width, int height, const char* name);
bool loadMedia(SDL_Surface* image, const char* path);
void close(SDL_Window *win);
void close(SDL_Surface *sur);
//-------------------------------------------------------------------------------------------------------------------------------------
SDL_Window* startWindow = NULL;
SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;
SDL_Surface* backgroud = NULL;
//-------------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char* args[]) {
	srand(time(NULL));

	vector<vector<int>> map(N, vector<int>(N, 0));
	gen(N, map);

	if (init(startWindow, backgroud, 500, 500, "Start window") == 0) { cout << "Failed to initialize\n"; } // Initialize start window

	if (!loadMedia(backgroud, "images/3.bmp")) {
		cout << "Failed to load media!\n";
	}
	else {
		SDL_BlitSurface(backgroud, NULL, screenSurface, NULL);
		SDL_UpdateWindowSurface(startWindow);
	}

	SDL_Delay(5000);
	close(startWindow); close(backgroud);
	SDL_Quit();

	return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------
bool init(SDL_Window *win, SDL_Surface *screen, int width, int height, const char* name) {
	bool success = true;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}

	else {
		 win = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

		if (&win == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			success = false;
		}
		else {
			screenSurface = SDL_GetWindowSurface(win);
			SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
		}
	}
	return success;
}

bool loadMedia(SDL_Surface *image, const char* path) {
	bool success = 1;
	backgroud = SDL_LoadBMP(path);
	if (backgroud == NULL) {
		cout << "Unable to load image :" << path << "! SDL Error :\n" << SDL_GetError() << endl;
		success = false;
	}
	return success;
}

void close(SDL_Window *win) {
	SDL_DestroyWindow(win);
	win = NULL;
}

void close(SDL_Surface *sur) {
	SDL_FreeSurface(sur);
	sur = NULL;
}
//-------------------------------------------------------------------------------------------------------------------------------------
