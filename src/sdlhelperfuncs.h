#ifndef SDLHELPERFUNCS_H
#define SDLHELPERFUNCS_H

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

SDL_Surface *load_image(char *);

void apply_surface(int x, int y, SDL_Surface *src, SDL_Surface *dst);

/*
 * Not suitable to be called repeatedly -- has to render the string on each
 * call. If drawing text in a loop, pre-render the text with `TTF_RenderFont()`
 * and use `apply_surface()` on it.
 */
void apply_text_blended(int x, int y, char *s, TTF_Font *f, SDL_Color c, SDL_Surface *dst);
void apply_text_shaded (int x, int y, char *s, TTF_Font *f, SDL_Color fg, SDL_Color bg, SDL_Surface *dst);
#endif
