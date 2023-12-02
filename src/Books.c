#include "Books.h"

void Superscript(int number, char* string) {
	// Dude NONE OF THIS CODE MAKES SENSE.
	int advance = 0;

	// Reverse the number
	int reverse = 0;
	int remainder, leadingZeros = 0;
    while (number) {
		remainder = number % 10;
		if (remainder == 0) leadingZeros++;
		reverse = reverse * 10 + remainder;
		number = number / 10;
    }
	number = reverse; // im lazy, ok?

	for (int i = 0; i < 6; i += 2) {
		if (number) {
			uint16_t writeValue;
			// Get the last two bytes of the Unicode superscript things
			switch (number % 10) {
				case 0: writeValue = 0x70; break;
				case 1: writeValue = 0xB9; break;
				case 2: writeValue = 0xB2; break;
				case 3: writeValue = 0xB3; break;
				case 4: writeValue = 0x74; break;
				case 5: writeValue = 0x75; break;
				case 6: writeValue = 0x76; break;
				case 7: writeValue = 0x77; break;
				case 8: writeValue = 0x78; break;
				case 9: writeValue = 0x79; break;
			}

			// Try and mash it in a way C understands
			if (number % 10 >= 1 && number % 10 <= 3) {
				string[0 + advance] = 0xC2;
				string[1 + advance] = writeValue;
				advance += 2;
			} else {
				string[0 + advance] = 0xE2;
				string[1 + advance] = 0x81;
				string[2 + advance] = 0x40 + writeValue;
				advance += 3;
			}
			number /= 10;
		} else if (leadingZeros) {
			string[0 + advance] = 0xE2;
			string[1 + advance] = 0x81;
			string[2 + advance] = (uint16_t)0x40 + 0x70;
			advance += 3;
			leadingZeros--;
		}
	}
}

cJSON* GetRoot(char* filename) {
	// Load file
	int fileSize;

	FILE* file = fopen(filename, "rb");

	if (!file) {
		fprintf(stderr, "Error: no file called %s", filename);
		return NULL;
	}

	// Getting file size
	fseek(file, 0l, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0l, SEEK_SET);

	// Put it in a buffer.
	char fileBuf[fileSize];
	memset(fileBuf, 0, fileSize);
	fread(fileBuf, sizeof(char), fileSize, file);

	fclose(file);

	// Start parsing!
	cJSON* root = cJSON_Parse(fileBuf);
	return root;
}

SDL_Texture* RenderChapter(TTF_Font* font, BibleData* data, cJSON* books, ezxml_t xmlBible, int chapter) {
	char bookChapterName[50];
	ezxml_t xmlBook = ezxml_idx(ezxml_child(xmlBible, "b"), data->usedBook);
	ezxml_t xmlChapter = ezxml_idx(ezxml_child(xmlBook, "c"), chapter - 1);
	snprintf(bookChapterName, 50, "%s %d", cJSON_GetArrayItem(cJSON_GetObjectItem(books, data->lang), data->usedBook)->valuestring, atoi(ezxml_attr(xmlChapter, "n")));

	float fontSize = (int)round(data->origFontSize * data->magnifier);
	TTF_SetFontSize(font, fontSize * 2);
	SDL_Surface* surf = TTF_RenderUTF8_Blended_Wrapped(font, bookChapterName, (SDL_Colour){0xff, 0xff, 0xff, 0xff}, (int)round(data->wrapWidth * data->wrapWidthMult));
	SDL_Texture* header = SDL_CreateTextureFromSurface(globalWindow->renderer, surf);
	SDL_FreeSurface(surf);

	if (!xmlChapter) {
		fprintf(stderr, "Error: No chapter %d!", chapter);
	}

	int numVerses;
	for (numVerses = 0; ezxml_idx(ezxml_child(xmlChapter, "v"), numVerses); numVerses++);

	char* text = NULL;
	int textSize = 0;

	for (int i = 0; i < numVerses; i++) {
		ezxml_t xmlVerse = ezxml_idx(ezxml_child(xmlChapter, "v"), i);
		int verseNumber = atoi(ezxml_attr(xmlVerse, "n"));
		char digits[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
		Superscript(verseNumber, digits);

		if (data->versePerLine) {
			text = (char*)realloc(text, sizeof(char) * (strlen(xmlVerse->txt) + textSize + strlen(digits) + strlen(" ") + 5));
			textSize += (strlen(xmlVerse->txt) + strlen(digits) + strlen(" ") + 5);
			if (i == 0) memset(text, 0, textSize);
			if (!text) {
				fprintf(stderr, "Error: allocating Bible text failed");
				break;
			}

			snprintf(text, textSize, "%s%s %s\n\n", text, digits, xmlVerse->txt);
		} else {
			text = (char*)realloc(text, sizeof(char) * (strlen(xmlVerse->txt) + textSize + strlen(digits) + strlen(" ") + 4));
			textSize += (strlen(xmlVerse->txt) + strlen(digits) + strlen(" ") + 4);
			if (i == 0) memset(text, 0, textSize);
			if (!text) {
				fprintf(stderr, "Error: allocating Bible text failed");
				break;
			}

			snprintf(text, textSize, "%s%s %s ", text, digits, xmlVerse->txt);
		}
	}

	TTF_SetFontSize(font, fontSize);
	surf = TTF_RenderUTF8_Blended_Wrapped(font, text, (SDL_Colour){0xff, 0xff, 0xff, 0xff}, (int)round(data->wrapWidth * data->wrapWidthMult));
	SDL_Texture* mainTextTex = SDL_CreateTextureFromSurface(globalWindow->renderer, surf);
	SDL_FreeSurface(surf);

	int headerWidth, headerHeight;
	SDL_QueryTexture(header, NULL, NULL, &headerWidth, &headerHeight);

	int mainTextTexWidth, mainTextTexHeight;
	SDL_QueryTexture(mainTextTex, NULL, NULL, &mainTextTexWidth, &mainTextTexHeight);
	SDL_Texture* dstTex = SDL_CreateTexture(globalWindow->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, (int)round(data->wrapWidth * data->wrapWidthMult), mainTextTexHeight + headerHeight);

	SDL_SetRenderTarget(globalWindow->renderer, dstTex);
	SDL_SetRenderDrawColor(globalWindow->renderer, 0xff, 0xff, 0xff, 0x00);
	SDL_SetTextureBlendMode(dstTex, SDL_BLENDMODE_BLEND);
	SDL_RenderClear(globalWindow->renderer);

	SDL_RenderCopy(globalWindow->renderer, header, NULL, &(SDL_Rect){(int)round(data->wrapWidth * data->wrapWidthMult) / 2 - headerWidth / 2, 0, headerWidth, headerHeight});
	SDL_RenderCopy(globalWindow->renderer, mainTextTex, NULL, &(SDL_Rect){0, headerHeight, mainTextTexWidth, mainTextTexHeight});

	SDL_DestroyTexture(header);
	SDL_DestroyTexture(mainTextTex);
	memset(text, 0, textSize);
	free(text);
	SDL_SetRenderTarget(globalWindow->renderer, NULL);
	return dstTex;
}

void CloseBook(Book* book) {
	for (int i = 0; i < book->numChapters; i++) {
		SDL_DestroyTexture(book->chapters[i].tex.data);
	}
	free(book->chapters);
}

void OpenBook(Book* dstBook, TTF_Font* font, BibleData* data, cJSON* jsonBooks, ezxml_t xmlBible) {
	ezxml_t xmlBook = ezxml_idx(ezxml_child(xmlBible, "b"), data->usedBook);
	for (dstBook->numChapters = 0; ezxml_idx(ezxml_child(xmlBook, "c"), dstBook->numChapters); dstBook->numChapters++);
	dstBook->chapters = malloc(dstBook->numChapters * sizeof(Chapter));
	if (dstBook->chapters == NULL) {
		fprintf(stderr, "Error: Insufficient memory\n");
		return;
	}
	for (int j = 0; j < dstBook->numChapters; j++) {
		dstBook->chapters[j].tex.data = RenderChapter(font, data, jsonBooks, xmlBible, j + 1);
		SDL_QueryTexture(dstBook->chapters[j].tex.data, NULL, NULL, &dstBook->chapters[j].tex.width, &dstBook->chapters[j].tex.height);
	}
}
