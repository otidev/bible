#ifndef NAVI_H
#define NAVI_H

#include "Include.h"

void LoadBibleIcon(BibleData* data, cJSON* jsonBooks);

void ScrollAndZoom(Book* books, BibleData* data, TTF_Font* font, cJSON* jsonBooks, ezxml_t xmlBible, Timer* text);

void ChangeChapter(Book* books, BibleData* data, TTF_Font* font, cJSON* jsonBooks, ezxml_t xmlBible, Timer* text, Timer* textTransition);

void SearchVerse(Highlight* hl, bool* lookup, Book* books, TTF_Font* font, BibleData* data, cJSON* jsonBooks, ezxml_t xmlBible, Timer* text, int lookupTexWidth, int lookupTexHeight, SDL_Texture* lookupTex);

#endif
