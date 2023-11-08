#include "Navigation.h"
#include "Books.h"

// WARNING: This file has *DIRTY* code.

void ScrollAndZoom(Book* books, float* magnifier, int usedBook, int chapter, float* scrollAmount, TTF_Font* font, int fontSize, int textWrapWidth, cJSON* jsonBooks, ezxml_t xmlBible, Timer* text) {
	if (globalWindow->mouseScroll.y > 0 || (globalWindow->keys[SDL_SCANCODE_UP] && !globalWindow->lastKeys[SDL_SCANCODE_UP])) {
		if (globalWindow->keys[SDL_SCANCODE_LCTRL]) {
			CloseBook(&books[usedBook]);
			(*magnifier) += 0.2;
			OpenBook(&books[(usedBook)], font, (int)round(fontSize * (*magnifier)), (usedBook), textWrapWidth, jsonBooks, xmlBible);
		} else {
			text->start = (*scrollAmount);
			if ((*scrollAmount) + 200 < globalWindow->height / 2)
				text->end = (*scrollAmount) + 200;
			else
				text->end = globalWindow->height / 2;
			text->timePlayed = 0;
		}
	} if (globalWindow->mouseScroll.y < 0 || (globalWindow->keys[SDL_SCANCODE_DOWN] && !globalWindow->lastKeys[SDL_SCANCODE_DOWN])) {
		if (globalWindow->keys[SDL_SCANCODE_LCTRL]) {
			CloseBook(&books[usedBook]);
			(*magnifier) -= 0.2;
			OpenBook(&books[(usedBook)], font, (int)round(fontSize * (*magnifier)), (usedBook), textWrapWidth, jsonBooks, xmlBible);
		} else {
			text->start = (*scrollAmount);
			if ((*scrollAmount) - 200 > -books[usedBook].chapters[chapter].tex.height + globalWindow->height / 2)
				text->end = (*scrollAmount) - 200;
			else
				text->end = -books[usedBook].chapters[chapter].tex.height + globalWindow->height / 2;
			text->timePlayed = 0;
		}
	}
}

void ChangeChapter(Book* books, int* usedBook, int* chapter, float* textOffset, float* scrollAmount, TTF_Font* font, int fontSize, int textWrapWidth, cJSON* jsonBooks, ezxml_t xmlBible, Timer* text, Timer* textTransition) {
	if (globalWindow->keys[SDL_SCANCODE_RIGHT] && !globalWindow->lastKeys[SDL_SCANCODE_RIGHT]) {
		if (!((*chapter) + 1 == books[(*usedBook)].numChapters && (*usedBook) == 65)) {
			(*scrollAmount) = text->end = 0;
			textTransition->start = (*textOffset) = -globalWindow->width;
			textTransition->end = 0;
			textTransition->timePlayed = 0;
			if ((*chapter) + 1 == books[(*usedBook)].numChapters) {
				(*usedBook)++;
				if ((*usedBook) != 65) OpenBook(&books[(*usedBook)], font, fontSize, (*usedBook), textWrapWidth, jsonBooks, xmlBible);
				(*chapter) = 0;
				textTransition->duration = 1.2;
			} else {
				(*chapter)++;
				textTransition->duration = 0.7;
			}
		}
	} if (globalWindow->keys[SDL_SCANCODE_LEFT] && !globalWindow->lastKeys[SDL_SCANCODE_LEFT]) {
		if (!((*chapter) == 0 && (*usedBook) == 0)) {
			(*scrollAmount) = text->end = 0;
			textTransition->start = (*textOffset) = globalWindow->width;
			textTransition->end = 0;
			textTransition->timePlayed = 0;
			if ((*chapter) == 0) {
				(*usedBook)--;
				if ((*usedBook) != 0) OpenBook(&books[(*usedBook)], font, fontSize, (*usedBook), textWrapWidth, jsonBooks, xmlBible);
				(*chapter) = books[(*usedBook)].numChapters - 1;
				textTransition->duration = 1.2;
			} else {
				(*chapter)--;
				textTransition->duration = 0.7;
			}
		}
	}
}
