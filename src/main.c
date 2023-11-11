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

	BibleData d;
	d.usedBook = 42;
	d.wrapWidth = 900;
	d.chapter = 0;
	d.magnifier = 1;
	d.origFontSize = 25;
	d.darkMode = false;
	d.scrollAmount = 0;
	d.textOffset = 0;
	d.textColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	d.srcTextColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	d.dstTextColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	d.bgColour = (SDL_Colour){0xff, 0xfd, 0xd0, 0xff};
	d.srcBgColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	d.dstBgColour = (SDL_Colour){0xff, 0xfd, 0xd0, 0xff};

	TTF_Font* font = TTF_OpenFont("../fonts/Cardo-Regular.ttf", d.origFontSize);
	if (!font) {
		fprintf(stderr, "Error: couldn't find font!");
		return 0;
	}

	SDL_Surface* icon = IMG_Load("../icons/biblelogo.ico");
	SDL_SetWindowIcon(window.window, icon);
	SDL_FreeSurface(icon);

	cJSON* jsonBooks = GetRoot("../books/Books.json");
	if (!jsonBooks) {
		fprintf(stderr, "Error: couldn't find main file!");
		return 0;
	}

	ezxml_t xmlBible = ezxml_parse_file("../books/KJV.xml");

	Book books[66];

	OpenBook(&books[d.usedBook], font, &d, jsonBooks, xmlBible);

	TTF_SetFontSize(font, 20);
	SDL_Surface* surf = TTF_RenderUTF8_Blended_Wrapped(font, "Enter Bible verse (e.g. John 3 16):  ", (SDL_Colour){255, 255, 255, 255}, window.width);
	TTF_SetFontSize(font, d.origFontSize);
	SDL_Texture* lookupTex = SDL_CreateTextureFromSurface(globalWindow->renderer, surf);
	int lookupTexWidth, lookupTexHeight;
	SDL_QueryTexture(lookupTex, NULL, NULL, &lookupTexWidth, &lookupTexHeight);
	SDL_FreeSurface(surf);

	Timer text;
	InitTimer(&text, 0.4);
	Timer textTransition;
	InitTimer(&textTransition, 0.7);
	Timer darkLightTransition;
	InitTimer(&darkLightTransition, 0.7);
	int lastWindowWidth = window.width;
	int lastWindowHeight = window.height;
	bool lookup = false;

	while (WindowIsOpen()) {
		now = SDL_GetTicks64();
		globalWindow->deltaTime = (double)((now - last) / 1000.0);
		last = SDL_GetTicks64();

		ScrollAndZoom(books, &d, font, jsonBooks, xmlBible, &text);
		ChangeChapter(books, &d, font, jsonBooks, xmlBible, &text, &textTransition);

		if (window.keys[SDL_SCANCODE_LCTRL] && (window.keys[SDL_SCANCODE_F] && !window.lastKeys[SDL_SCANCODE_F])) {
			for (int i = 0; i < 500; i++)
				window.textInput[i] = 0;
			if (lookup) lookup = false; else lookup = true;
		}

		if (window.keys[SDL_SCANCODE_LCTRL] && (window.keys[SDL_SCANCODE_D] && !window.lastKeys[SDL_SCANCODE_D])) {
			if (d.darkMode) d.darkMode = false; else d.darkMode = true;
			if (d.darkMode) {
				d.srcTextColour = d.textColour;
				d.dstTextColour = (SDL_Colour){0xff, 0xff, 0xff, 0xff};
				d.srcBgColour = d.bgColour;
				d.dstBgColour = (SDL_Colour){0x20, 0x20, 0x20, 0xff};
			} else {
				d.srcTextColour = d.textColour;
				d.dstTextColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
				d.srcBgColour = d.bgColour;
				d.dstBgColour = (SDL_Colour){0xff, 0xfd, 0xd0, 0xff};
			}
			darkLightTransition.timePlayed = 0;
		}


		if (d.scrollAmount != text.end) {
			d.scrollAmount = Lerp(text.start, text.end, 1 - pow(1 - (text.timePlayed / text.duration), 2));
			text.timePlayed += window.deltaTime;
		} if (text.timePlayed >= text.duration) {
			text.timePlayed = 0;
			d.scrollAmount = text.end;
		}

		if (d.textOffset != textTransition.end) {
			d.textOffset = Lerp(textTransition.start, textTransition.end, 1 - pow(1 - (textTransition.timePlayed / textTransition.duration), 2));
			textTransition.timePlayed += window.deltaTime;
		} if (textTransition.timePlayed >= textTransition.duration) {
			textTransition.timePlayed = 0;
			d.textOffset = textTransition.end;
		}

		if ((d.textColour.r != d.dstTextColour.r || d.textColour.g != d.dstTextColour.g || d.textColour.b != d.dstTextColour.b) || (d.bgColour.r != d.dstBgColour.r || d.bgColour.g != d.dstBgColour.g || d.bgColour.b != d.dstBgColour.b)) {
			d.textColour.r = Lerp(d.srcTextColour.r, d.dstTextColour.r, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			d.textColour.g = Lerp(d.srcTextColour.g, d.dstTextColour.g, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			d.textColour.b = Lerp(d.srcTextColour.b, d.dstTextColour.b, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));

			d.bgColour.r = Lerp(d.srcBgColour.r, d.dstBgColour.r, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			d.bgColour.g = Lerp(d.srcBgColour.g, d.dstBgColour.g, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			d.bgColour.b = Lerp(d.srcBgColour.b, d.dstBgColour.b, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			darkLightTransition.timePlayed += window.deltaTime;
		} if (darkLightTransition.timePlayed >= darkLightTransition.duration) {
			darkLightTransition.timePlayed = 0;
			d.textColour.r = d.dstTextColour.r; d.textColour.g = d.dstTextColour.g; d.textColour.b = d.dstTextColour.b;
			d.bgColour.r = d.dstBgColour.r; d.bgColour.g = d.dstBgColour.g; d.bgColour.b = d.dstBgColour.b;
		}

		if (d.scrollAmount > window.height / 2) {
			d.scrollAmount = text.end = window.height / 2;
			text.timePlayed = 0;
		} if (d.scrollAmount < -books[d.usedBook].chapters[d.chapter].tex.height + window.height / 2) {
			d.scrollAmount = text.end = -books[d.usedBook].chapters[d.chapter].tex.height + window.height / 2;
		}

		if ((lastWindowWidth != window.width) || (lastWindowHeight != window.height)) {
			CloseBook(&books[d.usedBook]);
			if (900 >= window.width)
				d.wrapWidth = window.width - 60;
			else
				d.wrapWidth = 900;
			OpenBook(&books[d.usedBook], font, &d, jsonBooks, xmlBible);
			if (lastWindowWidth != window.width) {
				lastWindowWidth = window.width;
			} if (lastWindowHeight != window.height) {
				lastWindowHeight = window.height;
			}

		}

		SDL_SetRenderDrawColor(window.renderer, d.bgColour.r, d.bgColour.g, d.bgColour.b, 0xff);
		SDL_RenderClear(window.renderer);

		SDL_SetTextureColorMod(books[d.usedBook].chapters[d.chapter].tex.data, d.textColour.r, d.textColour.g, d.textColour.b);
		SDL_RenderCopyF(window.renderer, books[d.usedBook].chapters[d.chapter].tex.data, NULL, &(SDL_FRect){window.width / 2 - (books[d.usedBook].chapters[d.chapter].tex.width) / 2 - (d.textOffset), d.scrollAmount, books[d.usedBook].chapters[d.chapter].tex.width, books[d.usedBook].chapters[d.chapter].tex.height});

		if (d.textOffset < textTransition.end) {
			if (d.chapter - 1 < 0) {
				SDL_RenderCopyF(
					window.renderer,
					books[d.usedBook - 1].chapters[books[d.usedBook - 1].numChapters - 1].tex.data,
					NULL,
					&(SDL_FRect){window.width / 2 - (books[d.usedBook - 1].chapters[books[d.usedBook - 1].numChapters - 1].tex.width) / 2 - (d.textOffset + window.width), d.scrollAmount, books[d.usedBook - 1].chapters[books[d.usedBook - 1].numChapters - 1].tex.width, books[d.usedBook - 1].chapters[books[d.usedBook - 1].numChapters - 1].tex.height}
				);
			} else {
				SDL_RenderCopyF(
					window.renderer,
					books[d.usedBook].chapters[d.chapter - 1].tex.data,
					NULL,
					&(SDL_FRect){window.width / 2 - (books[d.usedBook].chapters[d.chapter - 1].tex.width) / 2 - (d.textOffset + window.width), d.scrollAmount, books[d.usedBook].chapters[d.chapter - 1].tex.width, books[d.usedBook].chapters[d.chapter - 1].tex.height}
				);
			}
		}
		if (d.textOffset > textTransition.end) {
			if (d.chapter + 1 >= books[d.usedBook].numChapters) {
				SDL_RenderCopyF(
					window.renderer,
					books[d.usedBook + 1].chapters[0].tex.data,
					NULL,
					&(SDL_FRect){window.width / 2 - (books[d.usedBook + 1].chapters[0].tex.width) / 2 - (d.textOffset - window.width), d.scrollAmount, books[d.usedBook + 1].chapters[0].tex.width, books[d.usedBook + 1].chapters[0].tex.height}
				);
			} else {
				SDL_RenderCopyF(
					window.renderer,
					books[d.usedBook].chapters[d.chapter + 1].tex.data,
					NULL,
					&(SDL_FRect){window.width / 2 - (books[d.usedBook].chapters[d.chapter + 1].tex.width) / 2 - (d.textOffset - window.width), d.scrollAmount, books[d.usedBook].chapters[d.chapter + 1].tex.width, books[d.usedBook].chapters[d.chapter + 1].tex.height}
				);
			}
		}

		if (d.textOffset == textTransition.end && textTransition.start != textTransition.end) {
			if (textTransition.start > textTransition.end && d.chapter + 1 >= books[d.usedBook].numChapters) {
				CloseBook(&books[d.usedBook + 1]);
			} if (textTransition.start < textTransition.end && d.chapter - 1 < 0) {
				CloseBook(&books[d.usedBook - 1]);
			}
			textTransition.start = textTransition.end = 0;
		}

		if (lookup) {
			SearchVerse(&lookup, books, font, &d, jsonBooks, xmlBible, &text, lookupTexWidth, lookupTexHeight, lookupTex);
		}

		SDL_RenderPresent(window.renderer);
	}

	SDL_DestroyTexture(lookupTex);
	CloseBook(&books[d.usedBook]);

	ezxml_free(xmlBible);

	SDL_DestroyWindow(window.window);
	TTF_CloseFont(font);
	SDL_Quit();
	TTF_Init();
}
