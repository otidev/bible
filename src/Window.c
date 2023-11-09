#include "Window.h"

bool WindowIsOpen() {
	for (int i = 0; i < 256; i++) {
		globalWindow->lastKeys[i] = globalWindow->keys[i];
	}

	for (int i = 0; i < 5; i++) {
		globalWindow->lastMouseButtons[i] = globalWindow->mouseButtons[i];
	}

	globalWindow->mouseScroll.x = globalWindow->mouseScroll.y = 0;

	while (SDL_PollEvent(&globalWindow->event) != 0) {
		switch (globalWindow->event.type) {
			case SDL_QUIT:
				globalWindow->running = false;
				break;
			case SDL_MOUSEMOTION:
				globalWindow->mousePos.x = globalWindow->event.motion.x;
				globalWindow->mousePos.y = globalWindow->event.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				globalWindow->mouseButtons[globalWindow->event.button.button - 1] = true;
				break;
			case SDL_MOUSEBUTTONUP:
				globalWindow->mouseButtons[globalWindow->event.button.button - 1] = false;
				break;
			case SDL_KEYDOWN:
				globalWindow->keys[globalWindow->event.key.keysym.scancode] = true;
				break;
			case SDL_KEYUP:
				globalWindow->keys[globalWindow->event.key.keysym.scancode] = false;
				break;
			case SDL_MOUSEWHEEL:
				globalWindow->mouseScroll.y = globalWindow->event.wheel.y;
				globalWindow->mouseScroll.x = globalWindow->event.wheel.x;
				break;
			case SDL_TEXTINPUT:
				strcat(globalWindow->textInput, globalWindow->event.text.text);
				break;
			case SDL_WINDOWEVENT:
				if (globalWindow->event.window.windowID == SDL_GetWindowID(globalWindow->window)) {
					switch (globalWindow->event.window.event) {
						case SDL_WINDOWEVENT_SIZE_CHANGED:
							globalWindow->width = globalWindow->event.window.data1;
							globalWindow->height = globalWindow->event.window.data2;
							break;
					}
				}
				break;
		}
	}

	return globalWindow->running;
}

int InitCores(Window* window, int width, int height) {
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
		fprintf(stderr, "Error: Could not init SDL.");
		return 1;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)) {
		fprintf(stderr, "Error: Could not init SDL_Mixer.");
		return 1;
	}

	TTF_Init();

	window->window = SDL_CreateWindow("Bible", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!window->window) {
		fprintf(stderr, "Error: Could not create an SDL Window.");
		return 1;
	}

	window->width = width;
	window->height = height;

	window->renderer = SDL_CreateRenderer(window->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!window->renderer) {
		fprintf(stderr, "Error: Could not create an SDL Renderer.");
		return 1;
	}

	for (int i = 0; i < 256; i++) {
		window->keys[i] = 0;
	}

	for (int i = 0; i < 5; i++) {
		window->mouseButtons[i] = 0;
	}

	window->deltaTime = 0;

	window->running = true;
	globalWindow = window;
	return 0;
}
