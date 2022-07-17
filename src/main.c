#include <time.h>

#include "constants.h"
#include "globals.h"
#include "sdlhelperfuncs.h"

extern nav_vars_T run_menu();
extern nav_vars_T run_highscores_menu();
extern nav_vars_T run_game();

void initialise(const char *);

int main(int argc, const char *argv[])
{
	initialise("Snake");
	
	error_Texture = load_image("images/error.png");
	
	font_small  = TTF_OpenFont("coolvetica.ttf", 20);
	font_medium = TTF_OpenFont("coolvetica.ttf", 40);
	font_large  = TTF_OpenFont("coolvetica.ttf", 60);
	
	nav_vars_T navigation = run_menu();
	do
	{
		switch (navigation)
		{
			case MENU_ID:      navigation = run_menu();            break;
			case GAME_ID:      navigation = run_game();            break;
			case HIGHSCORE_ID: navigation = run_highscores_menu(); break;
			default: break;
		}
	} while (navigation != QUIT_ID);
	
	SDL_FreeSurface(screen);
	SDL_FreeSurface(error_Texture);
	
	TTF_CloseFont(font_small);
	TTF_CloseFont(font_medium);
	TTF_CloseFont(font_large);
	
	SDL_Quit();
	TTF_Quit();
	
	return 0;
}

void initialise(const char *window_name)
{
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		printf("Could not load SDL.\n");
		exit(1);
	}
	
	if (TTF_Init() == -1)
	{
		printf("Could not load SDL_tTF.\n");
		exit(1);
	}
	
	if ((screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT,
		SCREEN_BPP, SDL_SWSURFACE)) == NULL)
	{
		printf("Could not initialise the screen.(%dx%dx%d)\n",
			SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);
		exit(1);
	}
	
	SDL_WM_SetCaption(window_name, NULL);
	
	srand(time(NULL));
}
