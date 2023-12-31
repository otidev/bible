#ifndef BOOKS_H
#define BOOKS_H

#include "Include.h"

void Superscript(int number, char* string);

cJSON* GetRoot(char* filename);

SDL_Texture* RenderChapter(TTF_Font* font, BibleData* data, cJSON* books, ezxml_t xmlBible, int chapter);

void CloseBook(Book* book);

void OpenBook(Book* dstBook, TTF_Font* font, BibleData* data, cJSON* jsonBooks, ezxml_t xmlBible);

void RenderBookAndBooksBeside(TTF_Font* font, int fontSize, int textWrapWidth, Book* books, int bookNumber, cJSON* jsonBooks, ezxml_t xmlBible);

void ChangeBibleVersion(char* filename, BibleData* data, ezxml_t* xmlBible, cJSON* jsonBooks);

#endif
