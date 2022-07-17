#include "sdlhelperfuncs.h"

#include <SDL/SDL_image.h>

extern SDL_Surface *error_Texture;

void apply_surface(int x, int y, SDL_Surface* src, SDL_Surface* dst)
{
	SDL_Rect offset;
	
	offset.x = x;
	offset.y = y;
	
	SDL_BlitSurface(src, NULL, dst, &offset);
}

SDL_Surface *load_image(char *filename)
{
	SDL_Surface *loaded_image = NULL;
	
	loaded_image = IMG_Load(filename);
	
	if (loaded_image != NULL)
	{
		SDL_Surface *optimized_image = NULL;
		optimized_image = SDL_DisplayFormat(loaded_image);
		
		SDL_FreeSurface(loaded_image);
		
		return optimized_image;
	}
	
	printf("could not load image - %s\n", filename);
	return error_Texture;
}

void apply_text_blended(int x, int y, char *s, TTF_Font *f, SDL_Color c, SDL_Surface *dst)
{
	SDL_Surface *text = TTF_RenderText_Blended(f, s, c);
	apply_surface(x, y, text, dst);
	SDL_FreeSurface(text);
}

void apply_text_shaded(int x, int y, char *s, TTF_Font *f, SDL_Color fg, SDL_Color bg, SDL_Surface *dst)
{
	SDL_Surface *text = TTF_RenderText_Shaded(f, s, fg, bg);
	apply_surface(x, y, text, dst);
	SDL_FreeSurface(text);
}
