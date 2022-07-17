#include "menu.h"
#include "globals.h"
#include "sdlhelperfuncs.h"

typedef struct
{
	SDL_Surface *menu_bg;
	SDL_Surface *title_text;
	
	SDL_Surface *start_instruction;
	SDL_Surface *highscores_instruction;
	SDL_Surface *speed_instruction;
	
	SDL_Surface *help_instruction;
	SDL_Surface *quit_instruction;
	
	SDL_Surface *speed_display;
} menu_T;

static void set_speed(menu_T *, int speed);
static void draw_menu(menu_T *);
static void clean_up_menu(menu_T *);

nav_vars_T run_menu(void)
{
	menu_T *menu = malloc(sizeof(menu_T));
	
	/* load the text */
	menu->title_text = TTF_RenderText_Blended(font_medium, "Snake", white_colour);
	
	menu->start_instruction      = TTF_RenderText_Blended(font_small, "\"s\" to start the game",         white_colour);
	menu->highscores_instruction = TTF_RenderText_Blended(font_small, "\"h\" for the highscores",        white_colour);
	menu->speed_instruction      = TTF_RenderText_Blended(font_small, "1 through 5 to change the speed", white_colour);
	
	menu->help_instruction = TTF_RenderText_Blended(font_small, "\"e\" for help", white_colour);
	menu->quit_instruction = TTF_RenderText_Blended(font_small, "\"q\" to quit",  white_colour);
	
	menu->speed_display = NULL;
	
	/* 
	 * If the speed is -1 (aka. has never been set), pass the default mid-range
	 * value, else use the old speed value.
	 */
	set_speed(menu, (speed_human == -1 ? 2 : speed_human));
	
	/* load the images */
	menu->menu_bg = load_image("images/menu_bg.png");
	
	draw_menu(menu);
	
	SDL_Event event;
	while (1)
	{
		SDL_Delay(5); /* we don't need the loop running at 100% CPU */
		
		if (SDL_PollEvent(&event))
		{
			if (event.key.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
					case SDLK_1: set_speed(menu, 0); draw_menu(menu); break;
					case SDLK_2: set_speed(menu, 1); draw_menu(menu); break;
					case SDLK_3: set_speed(menu, 2); draw_menu(menu); break;
					case SDLK_4: set_speed(menu, 3); draw_menu(menu); break;
					case SDLK_5: set_speed(menu, 4); draw_menu(menu); break;
					
					case SDLK_q: clean_up_menu(menu); return QUIT_ID;
					case SDLK_s: clean_up_menu(menu); return GAME_ID;
					case SDLK_h: clean_up_menu(menu); return HIGHSCORE_ID;
					
					case SDLK_e:
						printf(
						"'a' - turn left\n"
						"'d' - turn right\n"
						"'p' - pause\n"
						"'m' - go to the main menu\n\n"
						"Left and right are relative to the direction the snake is heading.\n\n"
						"--------------------\n\n"
						"The score multiplier is based upon the speed of the game.\n\n"
						"Red block    - 1x points\n"
						"Yellow block - 3x points\n"
						"Purple block - 1x points and will get rid of 20%% of the obstacles\n"
						"Orange block - Either 10x points or controls temporarily reversed\n");
						break;
					
					default: break;
				}
			}
			else if (event.type == SDL_QUIT)
			{
				clean_up_menu(menu);
				return QUIT_ID;
			}
		}
	}
}

static void set_speed(menu_T *menu, int speed_human_A)
{
	const int BASE_TIME_BETWEEN_TICKS = 220;
	const int SPEED_STEP = 35;
	
	const int BASE_SCORE = 30;
	const int SCORE_STEP = 5;
	
	speed_human = speed_human_A;
	speed = BASE_TIME_BETWEEN_TICKS - (speed_human * SPEED_STEP);
	score_multiplier = BASE_SCORE   + (speed_human * SCORE_STEP);
	
	char speed_string[10];
	snprintf(speed_string, 10, "Speed - %d", speed_human + 1);
	
	if (menu->speed_display != NULL)
		SDL_FreeSurface(menu->speed_display);
	
	menu->speed_display = TTF_RenderText_Blended(font_small, speed_string, white_colour);
}

static void draw_menu(menu_T *menu)
{
	apply_surface(0, 0, menu->menu_bg, screen);
	
	apply_surface(20, 15, menu->title_text, screen);
	
	apply_surface(20, 70, menu->start_instruction,      screen);
	apply_surface(20, 94, menu->highscores_instruction, screen);
	apply_surface(20, 118, menu->speed_instruction,     screen);
	
	apply_surface(20, 168, menu->help_instruction, screen);
	apply_surface(20, 191, menu->quit_instruction, screen);
	apply_surface(20, 438, menu->speed_display,    screen);
	
	SDL_Flip(screen);
}

static void clean_up_menu(menu_T *menu)
{
	SDL_FreeSurface(menu->menu_bg);
	SDL_FreeSurface(menu->title_text);
	
	SDL_FreeSurface(menu->start_instruction);
	SDL_FreeSurface(menu->highscores_instruction);
	SDL_FreeSurface(menu->help_instruction);
	SDL_FreeSurface(menu->speed_instruction);
	SDL_FreeSurface(menu->quit_instruction);
	
	SDL_FreeSurface(menu->speed_display);
	
	free(menu);
}
