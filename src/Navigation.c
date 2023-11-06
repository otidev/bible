#include "Navigation.h"
#include "Books.h"

// WARNING: This file has *DIRTY* code.

void ScrollAndZoom(Book* books, float* magnifier, int usedBook, int chapter, float* scrollAmount, TTF_Font* font, int fontSize, int textWrapWidth, cJSON* jsonBooks, Timer* text) {
	if (globalWindow->mouseScroll.y > 0 || (globalWindow->keys[SDL_SCANCODE_UP] && !globalWindow->lastKeys[SDL_SCANCODE_UP])) {
		if (globalWindow->keys[SDL_SCANCODE_LCTRL]) {
			CloseBook(&books[usedBook]);
			if (usedBook != 0) CloseBook(&books[usedBook - 1]);
			if (usedBook != 65) CloseBook(&books[usedBook + 1]);
			(*magnifier) += 0.2;
			RenderBookAndBooksBeside(font, (int)round(fontSize * (*magnifier)), textWrapWidth, books, usedBook, jsonBooks);
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
			if (usedBook != 0) CloseBook(&books[usedBook - 1]);
			if (usedBook != 65) CloseBook(&books[usedBook + 1]);
			(*magnifier) -= 0.2;
			RenderBookAndBooksBeside(font, (int)round(fontSize * (*magnifier)), textWrapWidth, books, usedBook, jsonBooks);
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

void ChangeChapter(Book* books, int* usedBook, int* chapter, float* textOffset, float* scrollAmount, TTF_Font* font, int fontSize, int textWrapWidth, cJSON* jsonBooks, Timer* text, Timer* textTransition) {
	if (globalWindow->keys[SDL_SCANCODE_RIGHT] && !globalWindow->lastKeys[SDL_SCANCODE_RIGHT]) {
		if (!((*chapter) + 1 == books[(*usedBook)].numChapters && (*usedBook) == 65)) {
			(*scrollAmount) = text->end = 0;
			textTransition->start = (*textOffset) = -globalWindow->width;
			textTransition->end = 0;
			textTransition->timePlayed = 0;
			if ((*chapter) + 1 == books[(*usedBook)].numChapters) {
				if ((*usedBook) != 0) CloseBook(&books[(*usedBook) - 1]);
				(*usedBook)++;
				if ((*usedBook) != 65) OpenBook(&books[(*usedBook) + 1], font, fontSize, (*usedBook) + 1, textWrapWidth, jsonBooks);
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
				if ((*usedBook) != 65) CloseBook(&books[(*usedBook) + 1]);
				(*usedBook)--;
				if ((*usedBook) != 0) OpenBook(&books[(*usedBook) - 1], font, fontSize, (*usedBook) - 1, textWrapWidth, jsonBooks);
				(*chapter) = books[(*usedBook)].numChapters - 1;
				textTransition->duration = 1.2;
			} else {
				(*chapter)--;
				textTransition->duration = 0.7;
			}
		}
	}
}
