#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

extern const int SCREEN_BPP;
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

extern SDL_Surface *screen;

extern int score;
extern int score_multiplier; /* changed according to the speed of the game */

extern int speed_human; /* the human readable speed value, 1-5 */
extern int speed;       /* the delay between game steps */

extern TTF_Font *font_small;
extern TTF_Font *font_medium;
extern TTF_Font *font_large;

extern SDL_Color black_colour;
extern SDL_Color white_colour;

extern SDL_Surface *error_Texture;
#endif
