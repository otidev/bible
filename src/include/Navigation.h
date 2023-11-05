#ifndef NAVI_H
#define NAVI_H

#include "Include.h"

void ScrollAndZoom(Book* books, float* magnifier, int usedBook, int chapter, float* scrollAmount, TTF_Font* font, int fontSize, cJSON* jsonBooks, Timer* text);

void ChangeChapter(Book* books, int* usedBook, int* chapter, float* textOffset, float* scrollAmount, TTF_Font* font, int fontSize, cJSON* jsonBooks, Timer* text, Timer* textTransition);

#endif
