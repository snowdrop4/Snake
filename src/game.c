#include "game.h"
#include "sdlhelperfuncs.h"
#include "globals.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>
#include <time.h>
#include <stdbool.h>

/* time in number of snake moves (aka. game ticks) */
const int POWERUP_FREQUENCY = 150;
const int POWERUP_DURATION = 75;

/* size to grow snake by when food consumed */
const int SNAKE_LENGTH_INCREMENT = 3;

#define NUM_APPLES 3
#define MAX_SNAKE_SIZE 400

typedef struct
{
	int x;
	int y;
} xy_T;

typedef struct
{
	int x;
	int y;
	
	bool active;
	
	int type;
	
	int time_until_active;
	int time_active;
} powerup_T;

typedef struct
{
	SDL_Surface *game_bg;
	SDL_Surface *score_display;
	
	/*
	 * For keeping track of which direction the snake is moving.
	 * Will be reversed when the snake eats a bad powerup.
	*/
	int snake_x_vel;
	int snake_y_vel;
	int old_snake_x_vel;
	int old_snake_y_vel;
	
	bool controls_reversed;
	
	int num_rocks;
	xy_T rock[400];
	
	int snake_length;
	int pending_snake_segments;
	
	xy_T snake[MAX_SNAKE_SIZE];
	xy_T apple[NUM_APPLES];
	
	powerup_T powerup;
} game_T;

/* 
 * Returns NULL on GAME_OVER, or a nav_vars_T if the user explicitly chooses
 * a navigation value.
*/
static nav_vars_T *start_game(void);

static nav_vars_T end_game(void);
static void draw_game(game_T *);
static void clean_up_game(game_T *);

static void handle_input(game_T *game, int multiplier);

static void add_rock(game_T *);
static void add_food(game_T *, int);
static void add_power_up(game_T *);

/* returns -1 on no new highscore set, else returns position of new highscore */
static int highscores_io(void);

nav_vars_T run_game()
{
	nav_vars_T *navigation = start_game();
	
	if (navigation == NULL)
		return end_game();
	
	nav_vars_T ret = *navigation;
	free(navigation);
	return ret;
}

static nav_vars_T *start_game(void)
{
	game_T *game = malloc(sizeof(game_T));
	
	/* initialise the game structure variables */
	game->snake_x_vel = 15;
	game->snake_y_vel = 0;
	game->old_snake_x_vel = 15;
	game->old_snake_y_vel = 0;
	
	game->num_rocks = 0;
	
	game->controls_reversed = false;
	
	#define STARTING_SNAKE_LEN 4
	game->snake_length = STARTING_SNAKE_LEN;
	game->pending_snake_segments = 0;
	
	/* set up the snake */
	for (int i = 0, j = 195; i < STARTING_SNAKE_LEN; i++)
	{
		game->snake[i].x = j;
		game->snake[i].y = 195;
		
		j -= 15;
	}
	
	/* pick random apple starting positions */
	for (int i = 0; i < NUM_APPLES; i++)
	{
		game->apple[i].x = ((rand() %(SCREEN_WIDTH / 10)) * 10);
		game->apple[i].y = ((rand() %(SCREEN_HEIGHT / 10)) * 10);
		game->apple[i].x = game->apple[i].x - (game->apple[i].x % 15);
		game->apple[i].y = game->apple[i].y - (game->apple[i].y % 15);
	}
	
	/* set up the powerup */
	game->powerup.time_until_active = POWERUP_FREQUENCY;
	game->powerup.active = false;
	game->powerup.time_active = 0;
	
	/* load the score text */
	game->score_display = TTF_RenderText_Blended(font_small, "0", black_colour);
	
	/* load the images */
	game->game_bg = load_image("images/game_bg.png");
	
	draw_game(game);
	
	/* reset the score */
	score = 0;
	
	/* holds the score as a string for display */
	#define SCORE_STRING_LEN 15
	char score_string[SCORE_STRING_LEN];
	
	/* draw the "Go!" message */
	SDL_Surface *go_msg = TTF_RenderText_Blended(font_large, "Go!", black_colour);
	apply_surface((SCREEN_WIDTH  - go_msg->w) / 2,
	              (SCREEN_HEIGHT - go_msg->h) / 2,
	               go_msg, screen);
	SDL_FreeSurface(go_msg);
	
	SDL_Flip(screen);
	
	/* game loop */
	/*
	 * For keeping track of when the next game tick is, speed is added
	 * to move_Timer after each game tick.
	 * 
	 * Starting value of +500 so the player has enough time to get
	 * ready.
	*/
	clock_t timer = SDL_GetTicks();
	clock_t move_timer = timer + 500;
	
	SDL_Event event;
	bool paused = false;
	while (1)
	{
		/* 
		 * Poll for events for a certain amount of time (as specified
		 * in the `speed` variable). Then draw the game.
		 */
		while (timer < move_timer)
		{
			/* 
			 * If the game is paused, don't update the time. This
			 * "fools" the game into thinking no time has passed, 
			 * it will then continue to poll for events until we
			 * unpause.
			 */
			if (paused == false)
				timer = SDL_GetTicks();
			
			if (SDL_PollEvent(&event))
			{
				if (event.key.type == SDL_KEYDOWN)
				{
					switch (event.key.keysym.sym)
					{
						case SDLK_a: handle_input(game, (game->controls_reversed ? -1 :  1)); break;
						case SDLK_d: handle_input(game, (game->controls_reversed ?  1 : -1)); break;
						
						case SDLK_m:
						{
							clean_up_game(game);
							nav_vars_T *ret = malloc(sizeof(nav_vars_T));
							*ret = MENU_ID;
							return ret;
						};
						
						case SDLK_p:
						{
							if (paused == false)
							{
								SDL_Surface *text = TTF_RenderText_Blended(font_medium, "paused", black_colour);
								
								apply_surface(SCREEN_WIDTH  / 2 - text->w / 2,
											  SCREEN_HEIGHT / 2 - text->h / 2,
											  text, screen);
								SDL_FreeSurface(text);
								
								SDL_Flip(screen);
							}
							paused = !paused;
							break;
						}
						
						default: break;
					}
				}
				else if (event.type == SDL_QUIT)
				{
					clean_up_game(game);
					nav_vars_T *ret = malloc(sizeof(nav_vars_T));
					*ret = QUIT_ID;
					return ret;
				}
			}
		
			SDL_Delay(5);
		}
		
		/* add any pending snake segments */
		if (game->pending_snake_segments > 0)
		{
			game->pending_snake_segments--;
			game->snake_length++;
		}
		
		/*
		 * Move the snake.
		 * Get each segment to assume the x,y of the segment before it.
		*/
		for (int j = game->snake_length - 1; j > 0; j--)
		{
			game->snake[j].x = game->snake[j - 1].x;
			game->snake[j].y = game->snake[j - 1].y;
		}
		
		/* move the head in whatever direction was chosen */
		game->snake[0].x += game->snake_x_vel;
		game->snake[0].y += game->snake_y_vel;
		
		/* move the snake to the opposite edge of the screen if it goes off */
		if      (game->snake[0].x < 0)             game->snake[0].x = SCREEN_WIDTH  + 5;
		else if (game->snake[0].x > SCREEN_WIDTH)  game->snake[0].x = 0;
		else if (game->snake[0].y > SCREEN_HEIGHT) game->snake[0].y = 0;
		else if (game->snake[0].y < 0)             game->snake[0].y = SCREEN_HEIGHT + 5;
		
		/* if theres a collision */
		for (int i = 1; i < game->snake_length; i++)
			if (game->snake[0].x == game->snake[i].x && game->snake[0].y == game->snake[i].y)
			{
				clean_up_game(game);
				return NULL;
			}
		
		/* if the snake is over rock */
		for (int i = 0; i < game->num_rocks; i++)
			if (game->snake[0].x == game->rock[i].x && game->snake[0].y == game->rock[i].y)
			{
				clean_up_game(game);
				return NULL;
			}
		
		/* if the snake head is over an apple */
		for (int i = 0; i < NUM_APPLES; i++)
		{
			if (game->snake[0].x == game->apple[i].x && game->snake[0].y == game->apple[i].y)
			{
				score += score_multiplier;
				game->pending_snake_segments += SNAKE_LENGTH_INCREMENT;
				
				snprintf(score_string, SCORE_STRING_LEN, "%d", score);
				
				SDL_FreeSurface(game->score_display);
				game->score_display = TTF_RenderText_Blended(font_small, score_string, black_colour);
				
				add_food(game, i);
				add_rock(game);
			}
		}
		
		/* if there's a powerup in-game & snake head is over power-up */
		if ((game->powerup.active == true) && 
		    (game->snake[0].x == game->powerup.x &&
		     game->snake[0].y == game->powerup.y))
		{
			switch (game->powerup.type)
			{
				case 0: /* banana */
				{
					score += score_multiplier * 3;
					game->pending_snake_segments += SNAKE_LENGTH_INCREMENT;
					add_rock(game);
					break;
				}
				
				case 1: /* grape */
				{
					score += score_multiplier;
					game->pending_snake_segments += SNAKE_LENGTH_INCREMENT;
					game->num_rocks *= 0.8;
					break;
				}
				
				case 2: /* mystery box */
				{
					if (rand() % 2) /* pick a random outcome */
					{
						score += score_multiplier * 10;
						game->pending_snake_segments += SNAKE_LENGTH_INCREMENT;
						add_rock(game);
					}
					else /* reverse the controls */
					{
						score += score_multiplier;
						game->pending_snake_segments += SNAKE_LENGTH_INCREMENT;
						add_rock(game);
						
						game->controls_reversed = true;
					}
					break;
				}
				
				default:
				{
					printf("invalid powerup type %d\n", game->powerup.type);
					exit(1);
				}
			}
			
			snprintf(score_string, SCORE_STRING_LEN, "%d", score);
			
			SDL_FreeSurface(game->score_display);
			game->score_display = TTF_RenderText_Blended(font_small, score_string, black_colour);
			
			game->powerup.active = false;
			game->powerup.time_until_active = POWERUP_FREQUENCY;
			game->powerup.time_active = 0;
		}
		
		/* is it time for a new powerup? */
		game->powerup.time_until_active--;
		if (game->powerup.time_until_active == 0)
		{
			game->powerup.active = true;
			
			game->powerup.type = rand() % 3;
			
			add_power_up(game);
			
			/* if controls have been reversed, reset them */
			game->controls_reversed = false;
		}
		
		if (game->powerup.active == true)
		{
			game->powerup.time_active++;
			if (game->powerup.time_active == POWERUP_DURATION)
			{
				game->powerup.active = false;
				game->powerup.time_active = 0;
				game->powerup.time_until_active = POWERUP_FREQUENCY;
			}
		}
		
		/* reset the delay */
		move_timer = timer + speed;
		
		game->old_snake_x_vel = game->snake_x_vel;
		game->old_snake_y_vel = game->snake_y_vel;
		
		draw_game(game);
	}
}

static void draw_game(game_T *game)
{
	apply_surface(0, 0, game->game_bg, screen);
	apply_surface(6, 2, game->score_display, screen);
	
	/* apples */
	for (int i = 0; i < NUM_APPLES; i++)
	{
		boxColor(screen,
			game->apple[i].x,      game->apple[i].y,
			game->apple[i].x + 10, game->apple[i].y + 10,
			0xFF0000FF);
	}
	
	/* rocks */
	for (int i = 0; i < game->num_rocks; i++)
	{
		boxColor(screen,
			game->rock[i].x,      game->rock[i].y,
			game->rock[i].x + 10, game->rock[i].y + 10,
			0x000000FF);
	}
	
	/* powerups */
	if (game->powerup.active == true)
	{
		unsigned int colour;
		switch (game->powerup.type)
		{
			case 0: colour = 0xFFFF00FF; break; /* yellow */
			case 1: colour = 0x9C00FFFF; break; /* purple */
			case 2: colour = 0xFFAC00FF; break; /* orange */
		}
		
		boxColor(screen,
			game->powerup.x,      game->powerup.y,
			game->powerup.x + 10, game->powerup.y + 10,
			colour);
	}
	
	/* snake */
	unsigned int head_colour;
	unsigned int body_colour;
	
	if (game->controls_reversed == true)
	{
		head_colour = 0xAE0080FF; /* dark  pink */
		body_colour = 0xFF6AD8FF; /* light pink */
	}
	else
	{
		head_colour = 0x005917FF; /* dark  green */
		body_colour = 0x00AE2DFF; /* light green */
	}
	
	boxColor(screen,
			game->snake[0].x,      game->snake[0].y,
			game->snake[0].x + 10, game->snake[0].y + 10,
			head_colour);
	
	for (int i = 1; i < game->snake_length; i++)
		boxColor(screen,
			game->snake[i].x,      game->snake[i].y,
			game->snake[i].x + 10, game->snake[i].y + 10,
			body_colour);
			
	SDL_Flip(screen);
}

static nav_vars_T end_game()
{
	SDL_Surface *message;
	
	/* get the highscore position */
	int position = highscores_io();
	
	/* If a new highscore wasn't set, insult the player! Hahahaha....*/
	if (position == -1)
	{
		#define NUM_INSULTS 7
		char *insults[NUM_INSULTS] =
		{
			"Baffoon!",
			"Take a long, hard look at yourself.",
			"You're a complete disgrace!",
			"Bloody incompetent old fool!",
			"Twerp!",
			"Ninconpoop!",
			"Nitwit!"
		};
		
		int insultNum = rand() % NUM_INSULTS;
		message = TTF_RenderText_Shaded(font_medium,
					insults[insultNum], white_colour, black_colour);
	}
	else /* a new highscore was set */
	{
		char string[30];
		snprintf(string, 30, "New highscore! Position - %d", position);
		message = TTF_RenderText_Shaded(font_medium, string, white_colour, black_colour);
	}
	
	apply_surface(50, 180, message, screen);
	SDL_FreeSurface(message);
	
	apply_text_shaded(50, 227, "\"s\" to play again",      font_small, white_colour, black_colour, screen);
	apply_text_shaded(50, 251, "\"h\" for the highscores", font_small, white_colour, black_colour, screen);
	apply_text_shaded(50, 275, "\"m\" for the main menu",  font_small, white_colour, black_colour, screen);
	apply_text_shaded(50, 299, "\"q\" to quit",            font_small, white_colour, black_colour, screen);
	
	SDL_Flip(screen);
	
	SDL_Event event;
	while (1)
	{
		SDL_Delay(5);
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				return QUIT_ID;
			
			switch (event.key.keysym.sym)
			{
				case SDLK_m: return MENU_ID;
				case SDLK_s: return GAME_ID;
				case SDLK_h: return HIGHSCORE_ID;
				case SDLK_q: return QUIT_ID;
				default: break;
			}
		}
	}
}

static void clean_up_game(game_T *game)
{
	SDL_FreeSurface(screen);
	SDL_FreeSurface(game->game_bg);
	SDL_FreeSurface(game->score_display);
	
	free(game);
}

static void handle_input(game_T *game, int multiplier)
{
	if (game->old_snake_x_vel == -15)
	{
		game->snake_x_vel = 0;
		game->snake_y_vel = 15 * multiplier;
	}
	else if (game->old_snake_x_vel == 15)
	{
		game->snake_x_vel = 0;
		game->snake_y_vel = -15 * multiplier;
	}
	else if (game->old_snake_y_vel == -15)
	{
		game->snake_x_vel = -15 * multiplier;
		game->snake_y_vel = 0;
	}
	else if (game->old_snake_y_vel == 15)
	{
		game->snake_x_vel = 15 * multiplier;
		game->snake_y_vel = 0;
	}
}

static bool far_enough_from_snake(int x, int y, xy_T *snake, int snake_length)
{
	for (int i = 0; i < snake_length; i++)
	{
		if (x <= snake[i].x + 120 &&
			x >= snake[i].x - 120 &&
			y <= snake[i].y + 120 &&
			y >= snake[i].y - 120)
				return false;
	}
	
	return true;
}

static bool not_colliding_with(int x, int y, xy_T *objects, int num_objects)
{
	for (int i = 0; i < num_objects; i++)
	{
		if (x == objects[i].x && y == objects[i].y)
			return false;
	}
	
	return true;
}

static void add_rock(game_T *game)
{
	int x, y;
	
	while (1)
	{
		x = ((rand() % (SCREEN_WIDTH / 10)) * 10);
		y = ((rand() % (SCREEN_HEIGHT / 10)) * 10);
		
		/* make sure the rock falls on the 15px grid that the game runs on */
		x -= x % 15;
		y -= y % 15;
		
		if (far_enough_from_snake(x, y, game->snake, game->snake_length) == false)
			continue;
		
		if (not_colliding_with(x, y, game->apple, NUM_APPLES) == false)
			continue;
		
		if (not_colliding_with(x, y, game->rock, game->num_rocks) == false)
			continue;
		
		break;
	}
	
	game->rock[game->num_rocks].x = x;
	game->rock[game->num_rocks].y = y;
	game->num_rocks++;
}

static void add_food(game_T *game, int food)
{
	int x, y;
	
	while (1)
	{
		x = ((rand() % (SCREEN_WIDTH / 10)) * 10);
		y = ((rand() % (SCREEN_HEIGHT / 10)) * 10);
		
		/* make sure the rock falls on the 15px grid that the game runs on */
		x -= x % 15;
		y -= y % 15;
		
		if (far_enough_from_snake(x, y, game->snake, game->snake_length) == false)
			continue;
		
		if (not_colliding_with(x, y, game->rock, game->num_rocks) == false)
			continue;
		
		break;
	}
	
	game->apple[food].x = x;
	game->apple[food].y = y;
}

static void add_power_up(game_T *game)
{
	int x, y;
	
	while (1)
	{
		x = ((rand() % (SCREEN_WIDTH / 10)) * 10);
		y = ((rand() % (SCREEN_HEIGHT / 10)) * 10);
		
		/* make sure the rock falls on the 15px grid that the game runs on */
		x -= x % 15;
		y -= y % 15;
		
		if (far_enough_from_snake(x, y, game->snake, game->snake_length) == false)
			continue;
		
		if (not_colliding_with(x, y, game->apple, NUM_APPLES) == false)
			continue;
		
		if (not_colliding_with(x, y, game->rock, game->num_rocks) == false)
			continue;
		
		break;
	}
	
	game->powerup.x = x;
	game->powerup.y = y;
}

static int highscores_io(void)
{
	int ret_val = -1;
	
	const int NUM_HIGHSCORES = 10;
	char *highscores_strings[NUM_HIGHSCORES];
	
	FILE *highscores_file;
	highscores_file = fopen("highscores", "r");
	
	if (highscores_file == NULL)
	{
		printf("Couldn't open/find the highscores file");
		fclose(highscores_file);
		return -1;
	}
	
	/* read the highscores file and populate a string array from it */
	for (int i = 0; i < NUM_HIGHSCORES; i++)
	{
		highscores_strings[i] = malloc(15);
		fgets(highscores_strings[i], 15, highscores_file);
	}
	
	fclose(highscores_file);
	
	/*
	 * If the new score is greater than the last place in the highscores
	 * find it's exact place.
	 * Else return -1.
	*/
	if (score > atoi(highscores_strings[NUM_HIGHSCORES - 1]))
	{
		int position;
		/* get exact position */
		for (position = NUM_HIGHSCORES - 1;
		     score > atoi(highscores_strings[position]) &&
		     position != -1; position--)
			;
		
		position++;
		
		/* move all the scores down one to accomodate the new score */
		for (int i = 9; i >= position; i--)
			snprintf(highscores_strings[i], NUM_HIGHSCORES, "%d\n", atoi(highscores_strings[i - 1]));
		
		/* put the new score on the string array ready for writing to the file */
		snprintf(highscores_strings[position], NUM_HIGHSCORES, "%d\n", score);
		
		/* write the changes to the file */
		highscores_file = fopen("highscores", "r+");
		
		if (highscores_file == NULL)
			printf("Couldn't open/find the highscores file");
		else for (int i = 0; i < NUM_HIGHSCORES; i++)
			fprintf(highscores_file, "%s", highscores_strings[i]);
		
		fclose(highscores_file);
		
		ret_val = position + 1;
	}

	for (int i = 0; i < NUM_HIGHSCORES; i++)
		free(highscores_strings[i]);
	
	return ret_val;
}
