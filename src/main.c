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

void HighlightVerse(Highlight* highlighter, BibleData* data, ezxml_t xmlBible, TTF_Font* font, Book* books) {
	int textHeight;
	TTF_SizeUTF8(font, "", NULL, &textHeight);
	if (highlighter->width < 0) {
		ezxml_t xmlBook = ezxml_idx(ezxml_child(xmlBible, "b"), highlighter->book);
		if (!xmlBook) {
			fprintf(stderr, "Error: No book %d!", highlighter->book);
			return;
		}

		ezxml_t xmlChapter = ezxml_idx(ezxml_child(xmlBook, "c"), highlighter->chapter);
		if (!xmlChapter) {
			fprintf(stderr, "Error: No chapter %d!", highlighter->chapter);
			return;
		}

		char* dText = NULL;
		int dTextSize = 0;

		for (int i = 0; i < highlighter->verse; i++) {
			ezxml_t xmlVerse = ezxml_idx(ezxml_child(xmlChapter, "v"), i);
			int verseNumber = atoi(ezxml_attr(xmlVerse, "n"));
			char digits[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
			Superscript(verseNumber, digits);

			dText = (char*)realloc(dText, sizeof(char) * (strlen(xmlVerse->txt) + dTextSize + strlen(digits) + 2));
			dTextSize += (strlen(xmlVerse->txt) + strlen(digits) + 2);
			if (i == 0) memset(dText, 0, dTextSize);
			if (!dText) {
				fprintf(stderr, "Error: allocating Bible text failed");
				return;
			}
			snprintf(dText, dTextSize, "%s%s%s ", dText, digits, xmlVerse->txt);
		}
		int height;

		highlighter->offset = 0;
		if (highlighter->verse) {
			TTF_SetFontSize(font, (int)round(data->origFontSize * data->magnifier));
			TTF_SizeText(font, dText, &highlighter->offset, &height);
		}

		ezxml_t xmlVerse = ezxml_idx(ezxml_child(xmlChapter, "v"), highlighter->verse);
		int verseNumber = atoi(ezxml_attr(xmlVerse, "n"));
		char digits[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
		Superscript(verseNumber, digits);
		free(dText);

		dText = (char*)realloc(NULL, sizeof(char) * (strlen(xmlVerse->txt) + dTextSize + strlen(digits) + 2));
		dTextSize += (strlen(xmlVerse->txt) + strlen(digits) + 2);
		memset(dText, 0, dTextSize);
		if (!dText) {
			fprintf(stderr, "Error: allocating Bible text failed");
			return;
		}
		snprintf(dText, dTextSize, "%s%s%s ", dText, digits, xmlVerse->txt);

		TTF_SetFontSize(font, (int)round(data->origFontSize * data->magnifier));
		TTF_SizeText(font, dText, &highlighter->width, &height);
	}

	int counter = 0, lines = 0, pixels = 0, linesToSkip = 0, pixelsToSkip = 0;
	while (counter != highlighter->width) {
		pixels++;
		if (pixels == data->wrapWidth) {
			lines++;
			pixels = 0;
		}

		counter++;
	}

	counter = 0;
	while (counter != highlighter->offset) {
		pixelsToSkip++;
		if (pixelsToSkip == data->wrapWidth) {
			linesToSkip++;
			pixelsToSkip = 0;
		}

		counter++;
	}
	SDL_SetRenderDrawBlendMode(globalWindow->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(globalWindow->renderer, highlighter->colour.r, highlighter->colour.g, highlighter->colour.b, highlighter->colour.a);
	for (int i = 0; i < lines; i++) {
		SDL_RenderFillRect(
			globalWindow->renderer,
			&(SDL_Rect){
				globalWindow->width / 2 - (books[data->usedBook].chapters[data->chapter].tex.width) / 2 - (data->textOffset),
				data->scrollAmount + (textHeight * 2) + (textHeight * i),
				data->wrapWidth,
				textHeight
			}
		);
	}
	SDL_RenderFillRect(
		globalWindow->renderer,
		&(SDL_Rect){
			globalWindow->width / 2 - (books[data->usedBook].chapters[data->chapter].tex.width) / 2 - (data->textOffset) + pixelsToSkip,
			data->scrollAmount + textHeight * 2 + (textHeight * (lines + linesToSkip)),
			pixels,
			textHeight
		}
	);
	SDL_SetRenderDrawBlendMode(globalWindow->renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(globalWindow->renderer, data->bgColour.r, data->bgColour.g, data->bgColour.b, 0xff);
}

void ChangeBibleVersion(char* filename, BibleData* data, ezxml_t* xmlBible, cJSON* jsonBooks) {
	*xmlBible = ezxml_parse_file(filename);
	if (!ezxml_attr(*xmlBible, "lang")) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "FATAL ERROR: The file is not a formatted version!", globalWindow->window);
		fprintf(stderr, "FATAL ERROR: %s is not a formatted bible version!", filename);
		exit(1);
	}
	snprintf(data->lang, 7, "%s", ezxml_attr(*xmlBible, "lang"));
	data->numBooks = cJSON_GetArraySize(cJSON_GetObjectItem(jsonBooks, data->lang));
}

int main(int argc, char** argv) {
	Window window;
	if (InitCores(&window, 1280, 720)) return 1;
	last = SDL_GetTicks64();
	char* bibleVersion = 0;
	int bibleVersionSize = 0;

	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
			if (i + 1 < argc) {
				bibleVersionSize = strlen(argv[i + 1]) + strlen("../books/.xml") + 2;
				bibleVersion = realloc(NULL, bibleVersionSize);
				memset(bibleVersion, 0, bibleVersionSize);
				snprintf(bibleVersion, bibleVersionSize, "../books/%s.xml", argv[i + 1]);
			}
		}
	}

	if (bibleVersion == 0) {
		bibleVersionSize = strlen("../books/KJV.xml") + 1;
		bibleVersion = realloc(NULL, bibleVersionSize);
		memset(bibleVersion, 0, bibleVersionSize);
		snprintf(bibleVersion, bibleVersionSize, "../books/KJV.xml");
	}

	BibleData d;
	d.usedBook = 42;
	d.magnifier = 1;
	d.wrapWidth = 900;
	d.wrapWidthMult = 1;
	d.chapter = 0;
	d.origFontSize = 25;
	d.darkMode = false;
	d.scrollAmount = 0;
	d.textOffset = 0;
	d.versePerLine = false;
	d.textColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	d.srcTextColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	d.dstTextColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	d.bgColour = (SDL_Colour){0xff, 0xfd, 0xd0, 0xff};
	d.srcBgColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	d.dstBgColour = (SDL_Colour){0xff, 0xfd, 0xd0, 0xff};
	d.bgImg.data = NULL;
	d.bgImgColour = (SDL_Colour){0xf6, 0xea, 0xe2, 0x1f};
	d.bgImgSrcColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
	d.bgImgDstColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};

	cJSON* jsonBooks = GetRoot("../books/Books.json");
	if (!jsonBooks) {
		fprintf(stderr, "Error: couldn't find main file!");
		return 0;
	}

	SDL_Surface* surfPtr = IMG_Load("../icons/biblelogo.ico");
	SDL_SetWindowIcon(window.window, surfPtr);
	SDL_FreeSurface(surfPtr);

	TTF_Font* font = TTF_OpenFont("../fonts/Cardo-Regular.ttf", d.origFontSize);
	if (!font) {
		fprintf(stderr, "Error: couldn't find font!");
		return 0;
	}

	ezxml_t xmlBible = NULL;
	ChangeBibleVersion(bibleVersion, &d, &xmlBible, jsonBooks);
	Book* books = malloc(sizeof(Book) * (d.numBooks));
	if (d.usedBook >= d.numBooks) d.usedBook = 0;
	OpenBook(&books[d.usedBook], font, &d, jsonBooks, xmlBible);
	free(bibleVersion);
	LoadBibleIcon(&d, jsonBooks);

	Highlight highlighter;
	highlighter.width = -1;
	highlighter.book = 42;
	highlighter.chapter = 0;
	highlighter.verse = 0;
	highlighter.colour = (SDL_Colour){0x00, 0x00, 0x00, 0x3f};

	TTF_SetFontSize(font, d.origFontSize + 5);
	SDL_Surface* surf = TTF_RenderUTF8_Blended_Wrapped(font, "Enter Bible verse (format: book chapter verse):  ", (SDL_Colour){255, 255, 255, 255}, window.width);
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
	bool highlightVerse = false;
	bool fullscreen = true;

	while (WindowIsOpen()) {
		now = SDL_GetTicks64();
		globalWindow->deltaTime = (double)((now - last) / 1000.0);
		last = SDL_GetTicks64();

		ScrollAndZoom(books, &d, font, jsonBooks, xmlBible, &text);
		ChangeChapter(books, &d, font, jsonBooks, xmlBible, &text, &textTransition);

		if (globalWindow->droppedFile[0] != 0) {
			CloseBook(&books[d.usedBook]);
			ChangeBibleVersion(globalWindow->droppedFile, &d, &xmlBible, jsonBooks);
			if (d.usedBook >= d.numBooks) d.usedBook = 0;
			OpenBook(&books[d.usedBook], font, &d, jsonBooks, xmlBible);
			for (int i = 0; i < 500; i++) globalWindow->droppedFile[i] = 0;
		}

		if (window.keys[SDL_SCANCODE_EQUALS] && !window.lastKeys[SDL_SCANCODE_EQUALS]) {
			CloseBook(&books[d.usedBook]);
			if ((int)round(d.wrapWidth * d.wrapWidthMult) < window.width - 100)
				d.wrapWidthMult += 0.1;
			OpenBook(&books[d.usedBook], font, &d, jsonBooks, xmlBible);
		}

		if (window.keys[SDL_SCANCODE_MINUS] && !window.lastKeys[SDL_SCANCODE_MINUS]) {
			CloseBook(&books[d.usedBook]);
			if ((int)round(d.wrapWidth * d.wrapWidthMult) > 200)
				d.wrapWidthMult -= 0.1;
			OpenBook(&books[d.usedBook], font, &d, jsonBooks, xmlBible);
		}

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
				d.bgImgSrcColour = d.bgImgColour;
				d.bgImgDstColour = (SDL_Colour){0xff, 0xff, 0xff, 0x1f};
			} else {
				d.srcTextColour = d.textColour;
				d.dstTextColour = (SDL_Colour){0x00, 0x00, 0x00, 0xff};
				d.srcBgColour = d.bgColour;
				d.dstBgColour = (SDL_Colour){0xff, 0xfd, 0xd0, 0xff};
				d.bgImgSrcColour = d.bgImgColour;
				d.bgImgDstColour = (SDL_Colour){0xf6, 0xea, 0xe2, 0x1f};
			}
			darkLightTransition.timePlayed = 0;
		}

		if (window.keys[SDL_SCANCODE_LCTRL] && (window.keys[SDL_SCANCODE_L] && !window.lastKeys[SDL_SCANCODE_L])) {
			CloseBook(&books[d.usedBook]);
			if (d.versePerLine) {
				d.versePerLine = false;
			} else {
				d.versePerLine = true;
			}
			OpenBook(&books[d.usedBook], font, &d, jsonBooks, xmlBible);
		}

		if (window.keys[SDL_SCANCODE_F11] && !window.lastKeys[SDL_SCANCODE_F11]) {
			if (!fullscreen) {
				fullscreen = true;
				SDL_SetWindowFullscreen(window.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
			} else {
				fullscreen = false;
				SDL_SetWindowFullscreen(window.window, 0);
			}
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
			d.bgImgColour.r = Lerp(d.bgImgSrcColour.r, d.bgImgDstColour.r, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			d.bgImgColour.g = Lerp(d.bgImgSrcColour.g, d.bgImgDstColour.g, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			d.bgImgColour.b = Lerp(d.bgImgSrcColour.b, d.bgImgDstColour.b, 1 - pow(1 - (darkLightTransition.timePlayed / darkLightTransition.duration), 2));
			darkLightTransition.timePlayed += window.deltaTime;
		} if (darkLightTransition.timePlayed >= darkLightTransition.duration) {
			darkLightTransition.timePlayed = 0;
			d.textColour.r = d.dstTextColour.r; d.textColour.g = d.dstTextColour.g; d.textColour.b = d.dstTextColour.b;
			d.bgColour.r = d.dstBgColour.r; d.bgColour.g = d.dstBgColour.g; d.bgColour.b = d.dstBgColour.b;
			d.bgImgColour.r = d.bgImgDstColour.r; d.bgImgColour.g = d.bgImgDstColour.g; d.bgImgColour.b = d.bgImgDstColour.b;

		}

		if (d.scrollAmount > window.height / 2) {
			d.scrollAmount = text.end = window.height / 2;
			text.timePlayed = 0;
		} if (d.scrollAmount < -books[d.usedBook].chapters[d.chapter].tex.height + window.height / 2) {
			d.scrollAmount = text.end = -books[d.usedBook].chapters[d.chapter].tex.height + window.height / 2;
		}

		if ((lastWindowWidth != window.width) || (lastWindowHeight != window.height)) {
			CloseBook(&books[d.usedBook]);
			if ((int)round(d.wrapWidth * d.wrapWidthMult) >= window.width) {
				d.wrapWidth = window.width - 100;
				d.wrapWidthMult = 1;
			} else
				d.wrapWidth = 900;
			OpenBook(&books[d.usedBook], font, &d, jsonBooks, xmlBible);
			if (lastWindowWidth != window.width) {
				lastWindowWidth = window.width;
			} if (lastWindowHeight != window.height) {
				lastWindowHeight = window.height;
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

		SDL_SetRenderDrawColor(window.renderer, d.bgColour.r, d.bgColour.g, d.bgColour.b, 0xff);
		SDL_RenderClear(window.renderer);

		float moveDistance = (float)(window.width - d.bgImg.width * d.magnifier) / (books[d.usedBook].numChapters > 1 ? (books[d.usedBook].numChapters - 1) : 1);

		SDL_SetTextureColorMod(d.bgImg.data, d.bgImgColour.r, d.bgImgColour.g, d.bgImgColour.b);
		SDL_SetTextureAlphaMod(d.bgImg.data, d.bgImgColour.a);
		SDL_RenderCopyF(window.renderer, d.bgImg.data, NULL, &(SDL_FRect){
			(moveDistance * d.chapter) + (float)(moveDistance * (d.textOffset / globalWindow->width)),
			window.height / 2 - (d.bgImg.height * d.magnifier) / 2 + (d.scrollAmount * 0.1),
			d.bgImg.width * d.magnifier,
			d.bgImg.height * d.magnifier
		});

		SDL_SetTextureColorMod(books[d.usedBook].chapters[d.chapter].tex.data, d.textColour.r, d.textColour.g, d.textColour.b);
		SDL_RenderCopyF(window.renderer, books[d.usedBook].chapters[d.chapter].tex.data, NULL, &(SDL_FRect){window.width / 2 - (books[d.usedBook].chapters[d.chapter].tex.width) / 2 - (d.textOffset), d.scrollAmount, books[d.usedBook].chapters[d.chapter].tex.width, books[d.usedBook].chapters[d.chapter].tex.height});

		if (lookup) {
			SearchVerse(&highlighter, &lookup, books, font, &d, jsonBooks, xmlBible, &text, lookupTexWidth, lookupTexHeight, lookupTex);
		}

		if (highlightVerse) {
			HighlightVerse(&highlighter, &d, xmlBible, font, books);
		}

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
