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

	ezxml_t xmlBible = ezxml_parse_file("books/KJV.xml");

	Book books[66];
	int usedBook = 42;
	int wrapWidth = 900;
	int chapter = 0;

	OpenBook(&books[usedBook], font, origFontSize, usedBook, wrapWidth, jsonBooks, xmlBible);


	TTF_SetFontSize(font, 20);
	SDL_Surface* surf = TTF_RenderUTF8_Blended(font, "Enter book of the Bible:  ", (SDL_Colour){255, 255, 255, 255});
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
	Timer darkLightTransition;
	InitTimer(&darkLightTransition, 0.7);
	bool darkMode = false;
	SDL_Colour textColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	SDL_Colour srcTextColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	SDL_Colour dstTextColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	SDL_Colour bgColour = (SDL_Colour){0xff, 0xfd, 0xd0, 0xff};
	SDL_Colour srcBgColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	SDL_Colour dstBgColour = (SDL_Colour){0xff, 0xfd, 0xd0, 0xff};
	char inputBook[30];
	int lastWindowWidth = window.width;
	int lastWindowHeight = window.height;
	float textOffset = 0;
	bool lookup = false;

	while (WindowIsOpen()) {
		now = SDL_GetTicks64();
		globalWindow->deltaTime = (double)((now - last) / 1000.0);
		last = SDL_GetTicks64();

		ScrollAndZoom(books, &magnifier, usedBook, chapter, &scrollAmount, font, origFontSize, wrapWidth, jsonBooks, xmlBible, &text);
		ChangeChapter(books, &usedBook, &chapter, &textOffset, &scrollAmount, font, origFontSize, wrapWidth, jsonBooks, xmlBible, &text, &textTransition);

		if (window.keys[SDL_SCANCODE_LCTRL] && (window.keys[SDL_SCANCODE_L] && !window.lastKeys[SDL_SCANCODE_L])) {
			for (int i = 0; i < 500; i++)
				window.textInput[i] = 0;
			if (lookup) lookup = false; else lookup = true;
		}

		if (window.keys[SDL_SCANCODE_LCTRL] && (window.keys[SDL_SCANCODE_D] && !window.lastKeys[SDL_SCANCODE_D])) {
			if (darkMode) darkMode = false; else darkMode = true;
			if (darkMode) {
				srcTextColour = textColour;
				dstTextColour = (SDL_Colour){0xff, 0xff, 0xff, 0xff};
				srcBgColour = bgColour;
				dstBgColour = (SDL_Colour){0x0b, 0x07, 0x05, 0xff};
			} else {
				srcTextColour = textColour;
				dstTextColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
				srcBgColour = bgColour;
				dstBgColour = (SDL_Colour){0xff, 0xfd, 0xd0, 0xff};
			}
			darkLightTransition.timePlayed = 0;
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

		if ((textColour.r != dstTextColour.r || textColour.g != dstTextColour.g || textColour.b != dstTextColour.b) || (bgColour.r != dstBgColour.r || bgColour.g != dstBgColour.g || bgColour.b != dstBgColour.b)) {
			textColour.r = Lerp(srcTextColour.r, dstTextColour.r, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			textColour.g = Lerp(srcTextColour.g, dstTextColour.g, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			textColour.b = Lerp(srcTextColour.b, dstTextColour.b, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			bgColour.r = Lerp(srcBgColour.r, dstBgColour.r, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			bgColour.g = Lerp(srcBgColour.g, dstBgColour.g, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			bgColour.b = Lerp(srcBgColour.b, dstBgColour.b, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			darkLightTransition.timePlayed += window.deltaTime;
		} if (darkLightTransition.timePlayed >= darkLightTransition.duration) {
			darkLightTransition.timePlayed = 0;
			textColour.r = dstTextColour.r; textColour.g = dstTextColour.g; textColour.b = dstTextColour.b;
			bgColour.r = dstBgColour.r; bgColour.g = dstBgColour.g; bgColour.b = dstBgColour.b;
		}

		if (scrollAmount > window.height / 2) {
			scrollAmount = text.end = window.height / 2;
			text.timePlayed = 0;
		} if (scrollAmount < -books[usedBook].chapters[chapter].tex.height + window.height / 2) {
			scrollAmount = text.end = -books[usedBook].chapters[chapter].tex.height + window.height / 2;
		}

		if ((lastWindowWidth != window.width) || (lastWindowHeight != window.height)) {
			CloseBook(&books[usedBook]);
			if (900 >= window.width)
				wrapWidth = window.width - 40;
			else
				wrapWidth = 900;
			OpenBook(&books[usedBook], font, origFontSize, usedBook, wrapWidth, jsonBooks, xmlBible);
			if (lastWindowWidth != window.width) {
				lastWindowWidth = window.width;
			} if (lastWindowHeight != window.height) {
				lastWindowHeight = window.height;
			}

		}

		SDL_SetRenderDrawColor(window.renderer, bgColour.r, bgColour.g, bgColour.b, 0xff);
		SDL_RenderClear(window.renderer);

		if (lookup) {
			TTF_SetFontSize(font, 20);
			TTF_SetFontStyle(font, TTF_STYLE_ITALIC);
			surf = TTF_RenderUTF8_Blended(font, window.textInput, (SDL_Colour){255, 255, 255, 255});
			inputTex = SDL_CreateTextureFromSurface(globalWindow->renderer, surf);
			TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
			int inputTexWidth, inputTexHeight;
			SDL_QueryTexture(inputTex, NULL, NULL, &inputTexWidth, &inputTexHeight);
			SDL_SetTextureColorMod(lookupTex, textColour.r, textColour.g, textColour.b);
			SDL_SetTextureColorMod(inputTex, textColour.r, textColour.g, textColour.b);
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
						usedBook = i;
						chapter = 0;
						OpenBook(&books[usedBook], font, origFontSize, usedBook, wrapWidth, jsonBooks, xmlBible);
					}
				}
			}
		}

		SDL_SetTextureColorMod(books[usedBook].chapters[chapter].tex.data, textColour.r, textColour.g, textColour.b);
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

		if (textOffset == textTransition.end && textTransition.start != textTransition.end) {
			if (textTransition.start > textTransition.end && chapter + 1 >= books[usedBook].numChapters) {
				CloseBook(&books[usedBook + 1]);
			} if (textTransition.start < textTransition.end && chapter - 1 < 0) {
				CloseBook(&books[usedBook - 1]);
			}
			textTransition.start = textTransition.end = 0;
		}

		SDL_RenderPresent(window.renderer);
	}

	SDL_DestroyTexture(lookupTex);
	CloseBook(&books[usedBook]);

	ezxml_free(xmlBible);

	SDL_DestroyWindow(window.window);
	TTF_CloseFont(font);
	SDL_Quit();
	TTF_Init();
}
