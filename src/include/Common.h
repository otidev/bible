#ifndef COMMON_H
#define COMMON_H

#include "Include.h"

bool PointRectColl(Vec2 point, Rect rect);
float Lerp(float a, float b, float t);
void InitTimer(Timer* timer, float duration);

#endif
