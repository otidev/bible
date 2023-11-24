#include "Navigation.h"
#include "Books.h"

void LoadBibleIcon(BibleData* data, cJSON* jsonBooks) {
	if (data->bgImg.data) SDL_DestroyTexture(data->bgImg.data);
	char bookName[50];
	strcpy(bookName, cJSON_GetArrayItem(cJSON_GetObjectItem(jsonBooks, data->lang), data->usedBook)->valuestring);
	for (int i = 0; i < 50; i++) bookName[i] = tolower(bookName[i]);

	char iconLocation[60];
	snprintf(iconLocation, 50, "../icons/books/%s.png", bookName);
	SDL_Surface* surf = IMG_Load(iconLocation);
	data->bgImg.data = SDL_CreateTextureFromSurface(globalWindow->renderer, surf);
	SDL_FreeSurface(surf);
	SDL_QueryTexture(data->bgImg.data, NULL, NULL, &data->bgImg.width, &data->bgImg.height);
}

void ScrollAndZoom(Book* books, BibleData* data, TTF_Font* font, cJSON* jsonBooks, ezxml_t xmlBible, Timer* text) {
	if (globalWindow->mouseScroll.y > 0 || (globalWindow->keys[SDL_SCANCODE_UP] && !globalWindow->lastKeys[SDL_SCANCODE_UP])) {
		if (globalWindow->keys[SDL_SCANCODE_LCTRL]) {
			CloseBook(&books[data->usedBook]);
			data->magnifier += 0.2;
			OpenBook(&books[data->usedBook], font, data, jsonBooks, xmlBible);
		} else {
			text->start = data->scrollAmount;
			if (data->scrollAmount + 200 < globalWindow->height / 2)
				text->end = data->scrollAmount + 200;
			else
				text->end = globalWindow->height / 2;
			text->timePlayed = 0;
		}
	} if (globalWindow->mouseScroll.y < 0 || (globalWindow->keys[SDL_SCANCODE_DOWN] && !globalWindow->lastKeys[SDL_SCANCODE_DOWN])) {
		if (globalWindow->keys[SDL_SCANCODE_LCTRL]) {
			CloseBook(&books[data->usedBook]);
			data->magnifier -= 0.2;
			OpenBook(&books[data->usedBook], font, data, jsonBooks, xmlBible);
		} else {
			text->start = data->scrollAmount;
			if (data->scrollAmount - 200 > -books[data->usedBook].chapters[data->chapter].tex.height + globalWindow->height / 2)
				text->end = data->scrollAmount - 200;
			else
				text->end = -books[data->usedBook].chapters[data->chapter].tex.height + globalWindow->height / 2;
			text->timePlayed = 0;
		}
	}
}

void ChangeChapter(Book* books, BibleData* data, TTF_Font* font, cJSON* jsonBooks, ezxml_t xmlBible, Timer* text, Timer* textTransition) {
	if (globalWindow->keys[SDL_SCANCODE_RIGHT] && !globalWindow->lastKeys[SDL_SCANCODE_RIGHT]) {
		if (!(data->chapter + 1 == books[data->usedBook].numChapters && data->usedBook == 65)) {
			data->scrollAmount = text->end = 0;
			textTransition->start = data->textOffset = -globalWindow->width;
			textTransition->end = 0;
			textTransition->timePlayed = 0;
			if (data->chapter + 1 == books[data->usedBook].numChapters && data->usedBook != 65) {
				data->usedBook++;
				OpenBook(&books[data->usedBook], font, data, jsonBooks, xmlBible);
				data->chapter = 0;
				textTransition->duration = 1.2;
				LoadBibleIcon(data, jsonBooks);
			} else {
				data->chapter++;
				textTransition->duration = 0.7;
			}
		}
	} if (globalWindow->keys[SDL_SCANCODE_LEFT] && !globalWindow->lastKeys[SDL_SCANCODE_LEFT]) {
		if (!(data->chapter == 0 && data->usedBook == 0)) {
			data->scrollAmount = text->end = 0;
			textTransition->start = data->textOffset = globalWindow->width;
			textTransition->end = 0;
			textTransition->timePlayed = 0;
			if (data->chapter == 0 && data->usedBook != 0) {
				data->usedBook--;
				OpenBook(&books[data->usedBook], font, data, jsonBooks, xmlBible);
				data->chapter = books[data->usedBook].numChapters - 1;
				textTransition->duration = 1.2;
				LoadBibleIcon(data, jsonBooks);
			} else {
				data->chapter--;
				textTransition->duration = 0.7;
			}
		}
	}
}

void SearchVerse(Highlight* hl, bool* lookup, Book* books, TTF_Font* font, BibleData* data, cJSON* jsonBooks, ezxml_t xmlBible, Timer* text, int lookupTexWidth, int lookupTexHeight, SDL_Texture* lookupTex) {
	// Draw text and borders
	char inputBook[30];
	TTF_SetFontSize(font, data->origFontSize + 5);
	TTF_SetFontStyle(font, TTF_STYLE_ITALIC);
	SDL_Surface* surf = TTF_RenderUTF8_Blended_Wrapped(font, globalWindow->textInput, (SDL_Colour){255, 255, 255, 255}, globalWindow->width);
	SDL_Texture* inputTex = SDL_CreateTextureFromSurface(globalWindow->renderer, surf);
	TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
	int inputTexWidth, inputTexHeight;
	SDL_QueryTexture(inputTex, NULL, NULL, &inputTexWidth, &inputTexHeight);

	SDL_SetRenderDrawBlendMode(globalWindow->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(globalWindow->renderer, data->bgColour.r, data->bgColour.g, data->bgColour.b, 0xbf);
	SDL_RenderFillRect(globalWindow->renderer, &(SDL_Rect){0, 0, globalWindow->width, (data->origFontSize + 5) * 2});
	SDL_SetRenderDrawColor(globalWindow->renderer, 0, 0, 0, 0xbf);
	SDL_RenderDrawLine(globalWindow->renderer, 0, (data->origFontSize + 5) * 2, globalWindow->width, (data->origFontSize + 5) * 2);
	SDL_SetRenderDrawBlendMode(globalWindow->renderer, SDL_BLENDMODE_NONE);

	SDL_SetTextureColorMod(lookupTex, data->textColour.r, data->textColour.g, data->textColour.b);
	SDL_SetTextureColorMod(inputTex, data->textColour.r, data->textColour.g, data->textColour.b);

	SDL_RenderCopyF(globalWindow->renderer, lookupTex, NULL, &(SDL_FRect){5, (data->origFontSize + 5) * 2 / 2 - lookupTexHeight / 2, lookupTexWidth, lookupTexHeight});
	SDL_RenderCopyF(globalWindow->renderer, inputTex, NULL, &(SDL_FRect){lookupTexWidth, (data->origFontSize + 5) * 2 / 2 - inputTexHeight / 2, inputTexWidth, inputTexHeight});
	if (globalWindow->keys[SDL_SCANCODE_BACKSPACE]) {
		for (int i = 0; i < 200; i++) {
			if (globalWindow->textInput[i] != 0 && globalWindow->textInput[i + 1] == 0) {
				globalWindow->textInput[i] = 0;
			}
		}
	}

	if (globalWindow->keys[SDL_SCANCODE_RETURN]) {
		(*lookup) = false;
		char* token;
		// Get number of tokens before the chapter & verse
		int count = 0;
		char *ptr = globalWindow->textInput;
		while((ptr = strchr(ptr, ' ')) != NULL) {
			count++;
			ptr++;
		}
		count++;

		token = strtok(globalWindow->textInput, " ");
		bool firstToken = true;
		for (int i = 0; i < count - 2; i++) {
			if (firstToken) {
				snprintf(inputBook, 30, "%s", token);
				firstToken = false;
			} else {
				snprintf(inputBook, 30, "%s %s", inputBook, token);
			}
			token = strtok(NULL, " ");
		}

		data->chapter = atoi(token) - 1;

		for (int i = 0; i < 66; i++) {
			if (strcmp(inputBook, cJSON_GetArrayItem(cJSON_GetObjectItem(jsonBooks, data->lang), i)->valuestring) == 0) {
				if (data->usedBook == i) {
					break;
				} else {
					CloseBook(&books[data->usedBook]);
					data->usedBook = i;
					OpenBook(&books[data->usedBook], font, data, jsonBooks, xmlBible);
					LoadBibleIcon(data, jsonBooks);
				}
				break;
			}
		}

		if (data->chapter >= books[data->usedBook].numChapters) {
			data->chapter = 0;
		}

		hl->chapter = atoi(token) - 1;
		TTF_SetFontSize(font, data->origFontSize);

		ezxml_t xmlBook = ezxml_idx(ezxml_child(xmlBible, "b"), data->usedBook);
		if (!xmlBook) {
			fprintf(stderr, "Error: No book %d!", data->usedBook);
			(*lookup) = false;
			return;
		}
		ezxml_t xmlChapter = ezxml_idx(ezxml_child(xmlBook, "c"), data->chapter);
		if (!xmlChapter) {
			fprintf(stderr, "Error: No chapter %d!", data->chapter);
			(*lookup) = false;
			return;
		}

		int numVerses;
		for (numVerses = 0; ezxml_idx(ezxml_child(xmlChapter, "v"), numVerses); numVerses++);

		token = strtok(NULL, " ");
		if (atoi(token) - 1 >= numVerses) {
			fprintf(stderr, "Error: No verse %d!", atoi(token) - 1);
			return;
		}

		char* dText = NULL;
		int dTextSize = 0;
		int extra = 0;

		for (int i = 0; i < atoi(token) - 1; i++) {
			ezxml_t xmlVerse = ezxml_idx(ezxml_child(xmlChapter, "v"), i);
			int verseNumber = atoi(ezxml_attr(xmlVerse, "n"));
			char digits[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
			Superscript(verseNumber, digits);

			if (data->versePerLine) {
				dText = malloc(sizeof(char) * (strlen(xmlVerse->txt) + dTextSize + strlen(digits) + strlen(" ") + 3));
				dTextSize += (strlen(xmlVerse->txt) + strlen(digits) + strlen(" ") + 3);
				memset(dText, 0, dTextSize);
				if (!dText) {
					fprintf(stderr, "Error: allocating Bible text failed");
					break;
				}

				int width;

				// For each line:
				// Check if the size of the line excedes wrap width
				// If so, add by how many lines to a counter.

				snprintf(dText, dTextSize, "%s%s %s", dText, digits, xmlVerse->txt);
				TTF_SetFontSize(font, (int)round(data->origFontSize * data->magnifier));
				TTF_SizeText(font, dText, &width, NULL);
				extra += (int)floor((float)width / (int)round(data->wrapWidth * data->wrapWidthMult));
			} else {
				dText = (char*)realloc(dText, sizeof(char) * (strlen(xmlVerse->txt) + dTextSize + strlen(digits) + strlen(" ") + 4));
				dTextSize += (strlen(xmlVerse->txt) + strlen(digits) + strlen(" ") + 4);
				if (i == 0) memset(dText, 0, dTextSize);
				if (!dText) {
					fprintf(stderr, "Error: allocating Bible text failed");
					break;
				}


				snprintf(dText, dTextSize, "%s%s %s ", dText, digits, xmlVerse->txt);
			}
		}

		int width, chapterHeight, height;

		char bookChapterName[50];
		snprintf(bookChapterName, 50, "%s %d", cJSON_GetArrayItem(cJSON_GetObjectItem(jsonBooks, data->lang), data->usedBook)->valuestring, data->chapter);
		TTF_SetFontSize(font, (int)round(data->origFontSize * data->magnifier) * 2);
		TTF_SizeText(font, bookChapterName, NULL, &chapterHeight);

		TTF_SetFontSize(font, (int)round(data->origFontSize * data->magnifier));
		TTF_SizeText(font, dText, &width, &height);

		if (!dTextSize) {
			width = height = 0;
		}

		int newlineHeight = 0;
		if (data->versePerLine) {
			width = 0;
			TTF_SizeText(font, "", NULL, &newlineHeight);
			newlineHeight *= (atoi(token) - 1) * 2 + extra;
		} else {
			width = (int)floor((width / (int)round(data->wrapWidth * data->wrapWidthMult)) * height);
		}


		width += chapterHeight + newlineHeight;
		text->start = data->scrollAmount;
		if (data->scrollAmount - 200 > -books[data->usedBook].chapters[data->chapter].tex.height + globalWindow->height / 2)
			text->end = -width;
		else
			text->end = -books[data->usedBook].chapters[data->chapter].tex.height + globalWindow->height / 2;
		text->timePlayed = 0;

		hl->width = -1;
		hl->offset = 0;
		hl->book = data->usedBook;
		hl->verse = atoi(token) - 1;
		free(dText);
		SDL_FreeSurface(surf);
		SDL_DestroyTexture(inputTex);
	}
}
