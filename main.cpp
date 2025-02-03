#define SDL_MAIN_HANDLED
#include <iostream>
#include <SDL.h>
#include <SDL_Syswm.h>

int width, height;
SDL_Window* window;
SDL_Event event;
bool run = true;

void InitGraphics(int w, int h, HWND hwnd);
void RenderGraphics(float timeDelta);
void CleanGraphics();

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow(
		"Application - Direct3D 9",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		800, 600,
		SDL_WINDOW_SHOWN// | SDL_WINDOW_FULLSCREEN_DESKTOP
	);

	SDL_GetWindowSize(window, &width, &height);

	SDL_SysWMinfo sysInfo;
	SDL_VERSION(&sysInfo.version);
	SDL_GetWindowWMInfo(window, &sysInfo);
	HWND hwnd = sysInfo.info.win.window;

	InitGraphics(width, height, hwnd);

	while (run)
	{
		float currentFrame = SDL_GetTicks() / 1000.0f;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		SDL_PollEvent(&event);

		if (event.type == SDL_QUIT)
		{
			run = false;
		}

		RenderGraphics(deltaTime);
	}

	CleanGraphics();
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

