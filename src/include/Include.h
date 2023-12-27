#ifndef INCLUDE_H
#define INCLUDE_H

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <wchar.h>
#include <ctype.h>
#include "WavLoader.h"
#include "cJSON.h"
#include "ezxml.h"


#define PI2 M_PI * 2

typedef struct Vec2 {
	float x, y;
} Vec2;

typedef struct Rect {
	float x, y, w, h;
} Rect;

typedef struct Window {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event event;
	int width, height;
	int mouseButtons[5], lastMouseButtons[5];
	int MIDIppqn;
	Vec2 mouseScroll;
	Vec2 mousePos;
	double deltaTime;
	bool running;
	bool fullscreen;
	bool keys[256], lastKeys[256];
	bool typeBackspace, typeV;
	char textInput[500];
	char droppedFile[500];
} Window;

typedef struct Waveform {
	SDL_Texture* surf;
	Wave wav;
	float** samples;
	Mix_Chunk* chunk;
} Waveform;

typedef struct Timer {
	float duration;
	float timePlayed;
	float start, end;
} Timer;

typedef struct Texture {
	SDL_Texture* data;
	int width, height;
} Texture;

typedef struct Chapter {
	Texture tex;
} Chapter;

typedef struct Book {
	int numChapters;
	Chapter* chapters;
} Book;

typedef struct BibleData {
	char lang[7];
	bool darkMode;
	int usedBook;
	int wrapWidth;
	int chapter;
	int origFontSize;
	int numBooks;
	float magnifier;
	float wrapWidthMult;
	float scrollAmount;
	float textOffset;
	bool versePerLine;
	Texture bgImg;
	SDL_Colour textColour;
	SDL_Colour srcTextColour; // Used for timing
	SDL_Colour dstTextColour; // Used for timing
	SDL_Colour bgColour;
	SDL_Colour srcBgColour; // Used for timing
	SDL_Colour dstBgColour; // Used for timing
	SDL_Colour bgImgColour;
	SDL_Colour bgImgSrcColour; // Used for timing
	SDL_Colour bgImgDstColour; // Used for timing
} BibleData;

typedef struct Highlight {
	int verse;
	int book;
	int chapter;
	int offset;
	int width;
	SDL_Colour colour;
} Highlight;

extern Window* globalWindow;

#endif
