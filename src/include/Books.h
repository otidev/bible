#ifndef BOOKS_H
#define BOOKS_H

#include "Include.h"

cJSON* GetRoot(char* filename);

SDL_Texture* RenderChapter(TTF_Font* font, int fontSize, cJSON* books, ezxml_t xmlBible, int bookNumber, int chapter, int lineWidth);

void CloseBook(Book* book);

void OpenBook(Book* dstBook, TTF_Font* font, int fontSize, int bookNumber, int textWrapWidth, cJSON* jsonBooks, ezxml_t xmlBible);

void RenderBookAndBooksBeside(TTF_Font* font, int fontSize, int textWrapWidth, Book* books, int bookNumber, cJSON* jsonBooks, ezxml_t xmlBible);

#endif
