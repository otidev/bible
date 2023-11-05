#ifndef BOOKS_H
#define BOOKS_H

#include "Include.h"

cJSON* GetRoot(char* filename);

SDL_Texture* RenderChapter(TTF_Font* font, int fontSize, cJSON* books, cJSON* bookCont, int bookNumber, int chapter, SDL_Colour colour, int lineWidth);

void CloseBook(Book* book);

void OpenBook(Book* dstBook, TTF_Font* font, int fontSize, int bookNumber, cJSON* jsonBooks);

void RenderBookAndBooksBeside(TTF_Font* font, int fontSize, Book* books, int bookNumber, cJSON* jsonBooks);

#endif
