#include "Include.h"
#include "WavLoader.h"
#include "Window.h"
#include "Navigation.h"
#include "Books.h"
Window* globalWindow;
uint64_t last = 0, now = 0;

bool PointRectColl(Vec2 point, Rect rect) {
	if (point.x >= rect.x && point.y >= rect.y && point.x < rect.x + rect.w && point.y < rect.y + rect.h)
		return true;
	else
		return false;
}

float Lerp(float a, float b, float t) {
	return a + t * (b - a);
}

void InitTimer(Timer* timer, float duration) {
	timer->duration = duration;
	timer->timePlayed = 0;
	timer->start = timer->end = 0;
}

int main() {
	Window window;
	if (InitCores(&window, 1280, 720)) return 1;
	last = SDL_GetTicks64();
	float magnifier = 1;
	int origFontSize = 25;
	TTF_Font* font = TTF_OpenFont("fonts/Cardo-Regular.ttf", origFontSize);
	if (!font) {
		fprintf(stderr, "Error: couldn't find font!");
		return 0;
	}

	cJSON* jsonBooks = GetRoot("books/Books.json");
	if (!jsonBooks) {
		fprintf(stderr, "Error: couldn't find main file!");
		return 0;
	}

	Book books[66];
	int usedBook = 42;
	int wrapWidth = 900;

	RenderBookAndBooksBeside(font, origFontSize, wrapWidth, books, usedBook, jsonBooks);

	int chapter = 0;

	TTF_SetFontSize(font, 20);
	SDL_Surface* surf = TTF_RenderUTF8_Blended(font, "Enter Bible verse:  ", (SDL_Colour){0, 0, 0, 255});
	TTF_SetFontSize(font, origFontSize);
	SDL_Texture* lookupTex = SDL_CreateTextureFromSurface(globalWindow->renderer, surf);
	int lookupTexWidth, lookupTexHeight;
	SDL_QueryTexture(lookupTex, NULL, NULL, &lookupTexWidth, &lookupTexHeight);
	SDL_FreeSurface(surf);

	float scrollAmount = 0;
	Timer text;
	SDL_Texture* inputTex;
	InitTimer(&text, 0.4);
	Timer textTransition;
	InitTimer(&textTransition, 0.7);
	char inputBook[30];
	float textOffset = 0;
	bool lookup = false;

	while (WindowIsOpen()) {
		now = SDL_GetTicks64();
		globalWindow->deltaTime = (double)((now - last) / 1000.0);
		last = SDL_GetTicks64();

		ScrollAndZoom(books, &magnifier, usedBook, chapter, &scrollAmount, font, origFontSize, wrapWidth, jsonBooks, &text);
		ChangeChapter(books, &usedBook, &chapter, &textOffset, &scrollAmount, font, origFontSize, wrapWidth, jsonBooks, &text, &textTransition);

		if (window.keys[SDL_SCANCODE_LCTRL] && (window.keys[SDL_SCANCODE_L] && !window.lastKeys[SDL_SCANCODE_L])) {
			for (int i = 0; i < 500; i++)
				window.textInput[i] = 0;
			if (lookup) lookup = false; else lookup = true;
		}


		if (scrollAmount != text.end) {
			scrollAmount = Lerp(text.start, text.end, 1 - pow(1 - (text.timePlayed / text.duration), 2));
			text.timePlayed += window.deltaTime;
		} if (text.timePlayed >= text.duration) {
			text.timePlayed = 0;
			scrollAmount = text.end;
		}

		if (textOffset != textTransition.end) {
			textOffset = Lerp(textTransition.start, textTransition.end, 1 - pow(1 - (textTransition.timePlayed / textTransition.duration), 2));
			textTransition.timePlayed += window.deltaTime;
		} if (textTransition.timePlayed >= textTransition.duration) {
			textTransition.timePlayed = 0;
			textOffset = textTransition.end;
		}

		if (scrollAmount > window.height / 2) {
			scrollAmount = text.end = window.height / 2;
			text.timePlayed = 0;
		} if (scrollAmount < -books[usedBook].chapters[chapter].tex.height + window.height / 2) {
			scrollAmount = text.end = -books[usedBook].chapters[chapter].tex.height + window.height / 2;
		}

		SDL_SetRenderDrawColor(window.renderer, 0xff, 0xfd, 0xd0, 0xff);
		SDL_RenderClear(window.renderer);

		if (lookup) {
			TTF_SetFontSize(font, 20);
			TTF_SetFontStyle(font, TTF_STYLE_ITALIC);
			surf = TTF_RenderUTF8_Blended(font, window.textInput, (SDL_Colour){0, 0, 0, 255});
			inputTex = SDL_CreateTextureFromSurface(globalWindow->renderer, surf);
			TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
			int inputTexWidth, inputTexHeight;
			SDL_QueryTexture(inputTex, NULL, NULL, &inputTexWidth, &inputTexHeight);
			SDL_RenderCopyF(window.renderer, lookupTex, NULL, &(SDL_FRect){5, 40 / 2 - lookupTexHeight / 2, lookupTexWidth, lookupTexHeight});
			SDL_RenderCopyF(window.renderer, inputTex, NULL, &(SDL_FRect){lookupTexWidth, 40 / 2 - inputTexHeight / 2, inputTexWidth, inputTexHeight});
			if (window.keys[SDL_SCANCODE_BACKSPACE]) {
				for (int i = 0; i < 200; i++) {
					if (window.textInput[i] != 0 && window.textInput[i + 1] == 0) {
						window.textInput[i] = 0;
					}
				}
			}

			if (window.keys[SDL_SCANCODE_RETURN]) {
				lookup = false;
				char* token;
				token = strtok(window.textInput, " ");
				bool firstToken = true;
				while (token) {
					if (firstToken) {
						snprintf(inputBook, 30, "%s", token);
						firstToken = false;
					} else {
						snprintf(inputBook, 30, "%s %s", inputBook, token);
					}
					token = strtok(NULL, " ");
				}
				for (int i = 0; i < 66; i++) {
					if (strcmp(inputBook, cJSON_GetArrayItem(jsonBooks, i)->valuestring) == 0) {
						CloseBook(&books[usedBook]);
						if (usedBook != 0) CloseBook(&books[usedBook - 1]);
						if (usedBook != 65) CloseBook(&books[usedBook + 1]);
						usedBook = i;
						chapter = 0;
						RenderBookAndBooksBeside(font, origFontSize, wrapWidth, books, usedBook, jsonBooks);
					}
				}
			}
		}

		SDL_RenderCopyF(window.renderer, books[usedBook].chapters[chapter].tex.data, NULL, &(SDL_FRect){window.width / 2 - (books[usedBook].chapters[chapter].tex.width) / 2 - (textOffset), scrollAmount, books[usedBook].chapters[chapter].tex.width, books[usedBook].chapters[chapter].tex.height});
		if (textOffset < textTransition.end) {
			if (chapter - 1 < 0) {
				SDL_RenderCopyF(
					window.renderer,
					books[usedBook - 1].chapters[books[usedBook - 1].numChapters - 1].tex.data,
					NULL,
					&(SDL_FRect){window.width / 2 - (books[usedBook - 1].chapters[books[usedBook - 1].numChapters - 1].tex.width) / 2 - (textOffset + window.width), scrollAmount, books[usedBook - 1].chapters[books[usedBook - 1].numChapters - 1].tex.width, books[usedBook - 1].chapters[books[usedBook - 1].numChapters - 1].tex.height}
				);
			} else {
				SDL_RenderCopyF(
					window.renderer,
					books[usedBook].chapters[chapter - 1].tex.data,
					NULL,
					&(SDL_FRect){window.width / 2 - (books[usedBook].chapters[chapter - 1].tex.width) / 2 - (textOffset + window.width), scrollAmount, books[usedBook].chapters[chapter - 1].tex.width, books[usedBook].chapters[chapter - 1].tex.height}
				);
			}
		}
		if (textOffset > textTransition.end) {
			if (chapter + 1 >= books[usedBook].numChapters) {
				SDL_RenderCopyF(
					window.renderer,
					books[usedBook + 1].chapters[0].tex.data,
					NULL,
					&(SDL_FRect){window.width / 2 - (books[usedBook + 1].chapters[0].tex.width) / 2 - (textOffset - window.width), scrollAmount, books[usedBook + 1].chapters[0].tex.width, books[usedBook + 1].chapters[0].tex.height}
				);
			} else {
				SDL_RenderCopyF(
					window.renderer,
					books[usedBook].chapters[chapter + 1].tex.data,
					NULL,
					&(SDL_FRect){window.width / 2 - (books[usedBook].chapters[chapter + 1].tex.width) / 2 - (textOffset - window.width), scrollAmount, books[usedBook].chapters[chapter + 1].tex.width, books[usedBook].chapters[chapter + 1].tex.height}
				);
			}
		}

		SDL_RenderPresent(window.renderer);
	}

	SDL_DestroyTexture(lookupTex);
	CloseBook(&books[usedBook]);
	if (usedBook != 0) CloseBook(&books[usedBook - 1]);
	if (usedBook != 65) CloseBook(&books[usedBook + 1]);

	SDL_DestroyWindow(window.window);
	TTF_CloseFont(font);
	SDL_Quit();
	TTF_Init();
}
