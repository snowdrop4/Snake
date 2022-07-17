#include "globals.h"

/* ----------------------------------------------------- */
/* screen properties */

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 475;
const int SCREEN_BPP = 32;

SDL_Surface *screen = NULL;

/* ----------------------------------------------------- */
/* global game variables */
int score;
int score_multiplier = 40; /* changed according to the speed of the game */

int speed_human = -1;
int speed = -1; /* the delay between game ticks */

/* ----------------------------------------------------- */
/* misc */

TTF_Font *font_small = NULL;
TTF_Font *font_medium = NULL;
TTF_Font *font_large = NULL;

SDL_Color white_colour = {255, 255, 255};
SDL_Color black_colour = {0, 0, 0};

/* will be returned if load_image couldn't find the image */
SDL_Surface *error_Texture = NULL;
