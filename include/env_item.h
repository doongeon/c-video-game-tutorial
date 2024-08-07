#ifndef ENVITEM_H
#define ENVITEM_H

#include "raylib.h"

typedef struct EnvItem
{
    Rectangle rect;
    bool blockX;
    bool blockY;
    char *name;
    Color color;
} EnvItem;

EnvItem createEnvItem(char *name, Vector2 position, int width, int height, bool blockX, bool blockY);

float envItemLeft(EnvItem envitem);

float envItemRight(EnvItem envitem);

float envItemTop(EnvItem envitem);

float envItemBot(EnvItem envitem);



#endif
