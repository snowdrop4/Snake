#include "highscores.h"
#include "globals.h"
#include "sdlhelperfuncs.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

typedef struct
{
	SDL_Surface *highscores_bg;
} highscores_T;

static void clean_up_highscores(highscores_T *);

nav_vars_T run_highscores_menu(void)
{
	highscores_T *highscores = malloc(sizeof(highscores_T));
	
	highscores->highscores_bg = load_image("images/menu_bg.png");
	apply_surface(0, 0, highscores->highscores_bg, screen);
	
	/* draw the highscore menu labels */
	apply_text_blended(20, 15, "Highscores",       font_medium, white_colour, screen);
	apply_text_blended(20, 70, "\"m\" to go back", font_small,  white_colour, screen);
	apply_text_blended(20, 94, "\"q\" to quit",    font_small,  white_colour, screen);
	
	/* read the highscores file and render the results on-screen: */
	FILE *highscores_file;
	highscores_file = fopen("highscores", "r+");
	
	if (highscores_file != NULL)
	{
		/* same y value as the "press e for help" text in the main menu */
		int y = 168;
		
		const int HIGHSCORE_ENTRIES = 10;
		char highscores_strings[HIGHSCORE_ENTRIES][20];
		
		/*
		 * Read the highscores from the file and discard the '\n' from
		 *  each line as TTF_RenderText can't render '\n' and will just
		 *  display an error character '[]'.
		*/
		for (int i = 0; i < HIGHSCORE_ENTRIES; i++)
		{
			char in = 'a';
			for (int t = 0; t < 20; t++)
			{
				in = fgetc(highscores_file);
				
				if (in == '\n')
				{
					highscores_strings[i][t] = 0;
					break;
				}
				
				highscores_strings[i][t] = in;
			}
			
			/* draw the place number and the score */
			char place[4];
			snprintf(place, 4, "%d.", i + 1);
			
			apply_text_blended(20, y, place,                 font_small, white_colour, screen);
			apply_text_blended(42, y, highscores_strings[i], font_small, white_colour, screen);
			
			y += 25; /* move down to the next line */
		}
		
		fclose(highscores_file);
		
		SDL_Flip(screen);
		
		SDL_Event event;
		while (1)
		{
			SDL_Delay(5);
			
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_q)
				{
					clean_up_highscores(highscores);
					return QUIT_ID;
				}
				else if (event.key.keysym.sym == SDLK_m)
				{
					clean_up_highscores(highscores);
					return MENU_ID;
				}
			}
		}
	}
	/* Else the highscores file can't be opened: */
	else
	{
		printf("Couldn't open/find the highscores file");
		
		clean_up_highscores(highscores);
		return MENU_ID;
	}
}

static void clean_up_highscores(highscores_T *highscores)
{
	SDL_FreeSurface(highscores->highscores_bg);
	free(highscores);
}
