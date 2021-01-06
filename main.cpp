#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"SDL2-2.0.10/include/SDL.h"
#include"SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define ETI_HEIGHT 90
#define ETI_WIDTH 90
#define SPEED_MULTIPLIER 100
#define ETI_LEFT_FIX 100
#define JUMP_TIME 150
#define INITIAL_ETI_HEIGHT 315
#define JUMP_VELOCITY 2
#define FALL_VELOCITY 1
#define IMMUNITY_TIME 500
#define DASH_TIME 100
#define LEVEL_WIDTH 3300
#define LEVEL_HEIGHT 1272
#define OBSTACLE_X 90
#define OBSTACLE_Y 50
#define OBSTACLES_AMOUNT 3
#define STALACTITE_X 65
#define STALACTITE_Y 145
#define STALACTITES_AMOUNT 2
#define X_OFFSET 440
#define Y_OFFSET 410
#define SURFACE_AMOUNT 6
const int surfaceHeights[SURFACE_AMOUNT] = { INITIAL_ETI_HEIGHT, 155, -70, INITIAL_ETI_HEIGHT, 410, 325 };
const int surfaceX[SURFACE_AMOUNT][2] = { {0,843},{843,2194}, {2423,3300}, {1500, 2206}, {1350,2012}, {2012,2311} };
const int obstaclesLocation[OBSTACLES_AMOUNT][2] = { {304,315}, {1005,153}, {1900, 153}};
const int stalactitesLocation[STALACTITES_AMOUNT][2] = { {1880,-170}, {2459,-170} };



// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

void jump(double* etiHeight, int* isJumping, int* isFalling, int* jumper, int* jumpCounter, int* isDashing)
{
	//jump/double jump handling, the height of the jump is bounded by the MAXJUMP constant
	if (*isJumping == 1)
	{
		*isFalling = 0;
	}
	if (*jumper<=0)
	{
		*isJumping = 0;
		*isFalling = 1;
		*jumper = JUMP_TIME;
		
	}
	if (*isJumping == 1 && *jumper > 0)
	{
		*jumper-=1;
		*etiHeight -= JUMP_VELOCITY;
	}
	
	if (*isFalling ==1)
	{
		*etiHeight+=FALL_VELOCITY;
	}

	if (*isJumping == 0 && *isFalling == 0)
	{
		*jumpCounter = 0;
		*jumper = JUMP_TIME;
	}
}


int collision(double *distance, double *etiHeight)
{
	//returns 1 if collision with an obstacle occurs
	for (int i = 0; i < OBSTACLES_AMOUNT; i++)
	{
		if (obstaclesLocation[i][0] >= (*distance - OBSTACLE_X) && obstaclesLocation[i][0] <= (*distance + OBSTACLE_X))
		{
			if (*etiHeight >= obstaclesLocation[i][1] && (*etiHeight - OBSTACLE_Y) <= obstaclesLocation[i][1])
			{
				return 1;
			}
		}
	}
	for (int i = 0; i < STALACTITES_AMOUNT; i++)
	{
		if (stalactitesLocation[i][0] >= (*distance - STALACTITE_X) && stalactitesLocation[i][0] <= (*distance + STALACTITE_X))
		{
			if (*etiHeight <= stalactitesLocation[i][1] && (*etiHeight - STALACTITE_Y) <= stalactitesLocation[i][1])
			{
				return 1;
			}
		}
	}
	return 0;
}


void dash(int *isDashing, int* dashTime, int *speedChangeToken, int *isFalling, int *isJumping, double *etiSpeed, int *jumpCounter) 
{
	//this function makes the unicorn dash by momentarily multiplying its speed by 10 and stopping any falling
	if (*isDashing == 1)
	{
		*isFalling = 0;
	}
	if (*isDashing == 1 && *dashTime > 0)
	{
		if (*speedChangeToken == 0)
		{
			*etiSpeed *= 10;
			*speedChangeToken = 1;
		}
		*dashTime -= 1;
		*isFalling = 0;
		*isJumping = 0;
		*jumpCounter = 1;
	}
	if (*dashTime <= 0)
	{
		*isFalling = 1;
		*isDashing = 0;
		*speedChangeToken = 0;
		if (*etiSpeed >= 100) *etiSpeed /= 10;
		*dashTime = DASH_TIME;
	}
}

void falling(double* etiHeight, double distance, int* isFalling)
{
	//this function is responsible for the unicorn staying on the platforms
	if (distance <= surfaceX[0][1] && *etiHeight >= surfaceHeights[0])
	{
		*isFalling = 0;
	}
	if (distance <= surfaceX[0][1] && *etiHeight < surfaceHeights[0])
	{
		*isFalling = 1;
	}

	if (distance > surfaceX[1][0] && distance < surfaceX[1][1])
	{
		if (*etiHeight == surfaceHeights[1]) *isFalling = 0;
		else *isFalling = 1;
	}
	if (distance >= surfaceX[2][0] - ETI_WIDTH && *etiHeight < surfaceHeights[3])
	{
		*isFalling = 1;
	}
	if (distance >= surfaceX[2][0] - ETI_WIDTH && *etiHeight >=surfaceHeights[3])
	{
		*isFalling = 0;
	}
	if (distance > surfaceX[3][0] - ETI_WIDTH && distance < surfaceX[3][1] + ETI_WIDTH)
	{
		if (*etiHeight == surfaceHeights[2]) *isFalling = 0;
		else if(*etiHeight == surfaceHeights[1] && distance > surfaceX[1][0] && distance < surfaceX[1][1])* isFalling = 0;
		else *isFalling = 1;
	}
	if (distance > surfaceX[4][0] -ETI_WIDTH && distance < surfaceX[4][1] + ETI_WIDTH)
	{
		if (*etiHeight == surfaceHeights[4]) *isFalling = 0;
		else if (*etiHeight == surfaceHeights[2]) *isFalling = 0;
		else if (*etiHeight == surfaceHeights[1] && distance > surfaceX[1][0] && distance < surfaceX[1][1])*isFalling = 0;
		else *isFalling = 1;
	}
	if (distance > surfaceX[5][0] -ETI_WIDTH && distance < surfaceX[5][1] + ETI_WIDTH)
	{
		if (*etiHeight == surfaceHeights[5]) *isFalling = 0;
		else if (*etiHeight == surfaceHeights[4]) *isFalling = 0;
		else if (*etiHeight == surfaceHeights[2] && distance <surfaceX[3][1]) *isFalling = 0;
		else if (*etiHeight == surfaceHeights[1] && distance > surfaceX[1][0] && distance < surfaceX[1][1])*isFalling = 0;
		else *isFalling = 1;
	}

}

void roundOver(int* frames, double* fpsTimer, double* fps, int* quit, double* worldTime, double* distance, double* etiSpeed, double* etiHeight, int *jumper)
{
	//resetting gamestate values
	//use this after player looses a life or clicks n
	
	*frames = 0;
	*fpsTimer = 0;
	*fps = 0;
	*quit = 0;
	*worldTime = 0;
	*distance = 0;
	*etiSpeed = 300;
	*etiHeight = INITIAL_ETI_HEIGHT;
	*jumper = 0;
}


int inLevelCheck(double* etiHeight)
{
	//this function checks whether the unicorn is inside the acceptable height
	if (*etiHeight > SCREEN_HEIGHT + ETI_HEIGHT)
	{
		return 0;
	}
	else 
	{
		return 1;
	}
}

void levelLoop(double* distance)
{
	if (*distance > LEVEL_WIDTH - SCREEN_WIDTH)
	{
		//this makes the level loop
		*distance = 0;
	}

}
void dashCounterReset(int *isFalling, int* isJumping, int* isDashing, int* dashCounter)
{
	if (*isFalling == 0 && *isJumping == 0 && *isDashing == 0)
	{
		*dashCounter = 0;
	}
}

void topHit(double* distance, double* etiHeight, int* isFalling, int* isJumping)
{
	//this function is responsible for making the unicorn fall after hitting the bottom of a hanging surface
	//2 and 4 to the height as i found it works better on small interval than on a fixed value
	if (*distance > surfaceX[1][0] && *distance < surfaceX[1][1])
	{
		if (*etiHeight >= surfaceHeights[1] + ETI_HEIGHT+2 && *etiHeight <= surfaceHeights[1] + ETI_HEIGHT + 3)
		{
			*isFalling = 1;
			*isJumping = 0;
		}
	}
	if (*distance > surfaceX[3][0] && *distance < surfaceX[3][1])
	{
		if (*etiHeight >= surfaceHeights[2] + ETI_HEIGHT + 2 && *etiHeight <= surfaceHeights[2] + ETI_HEIGHT + 3)
		{
			*isFalling = 1;
			*isJumping = 0;
		}
	}
}

int terrainCollision(double* distance, double* etiHeight)
{
	if (*distance > surfaceX[4][0] && *distance < surfaceX[4][1])
	{
		if (*etiHeight > surfaceHeights[4] + ETI_HEIGHT/2) return 1;
	}
	if (*distance >= surfaceX[2][0] - ETI_WIDTH/2 && *etiHeight > surfaceHeights[3] +ETI_HEIGHT/2)
	{
		return 1;
	}
	if (*distance > surfaceX[5][0] - ETI_WIDTH/2 && *distance < surfaceX[5][1])
	{
		if (*etiHeight > surfaceHeights[5] + ETI_HEIGHT/2) return 1;
	}

	return 0;
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc, isJumping, isFalling, jumper, controlMode, jumpCounter, lives,isDashing, dashCounter,dashTime,speedChangeToken;
	double delta, worldTime, fpsTimer, fps, distance, etiSpeed, etiHeight;
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *eti, *background;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	

	

	// okno konsoli nie jest widoczne, je¿eli chcemy zobaczyæ
	// komunikaty wypisywane printf-em trzeba w opcjach:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// zmieniæ na "Console"
	// console window is not visible, to see the printf output
	// the option:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// must be changed to "Console"
	printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	// tryb pe³noekranowy / fullscreen mode
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2021");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("cs8x8.bmp");
	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);

	eti = SDL_LoadBMP("eti.bmp");
	if(eti == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	background = SDL_LoadBMP("bg.bmp");
	t1 = SDL_GetTicks();
	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	distance = 0;
	etiSpeed = 300;
	isJumping = 0;
	isFalling = 0;
	jumper = JUMP_TIME;
	etiHeight = INITIAL_ETI_HEIGHT;
	controlMode = 0;
	jumpCounter = 0;
	lives = 3;
	isDashing = 0;
	dashCounter = 0;
	dashTime = DASH_TIME;
	speedChangeToken = 0;
	while(!quit) {
		t2 = SDL_GetTicks();

		// w tym momencie t2-t1 to czas w milisekundach,
		// jaki uplyna³ od ostatniego narysowania ekranu
		// delta to ten sam czas w sekundach
		// here t2-t1 is the time in milliseconds since
		// the last screen was drawn
		// delta is the same time in seconds

		if (lives == 0)
		{
			//quit the program if the player has no lives available
			SDL_FreeSurface(charset);
			SDL_FreeSurface(screen);
			SDL_DestroyTexture(scrtex);
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);

			SDL_Quit();
			return 0;
		}
		
		

		delta = (t2 - t1) * 0.001;

		t1 = t2;

		worldTime += delta;
		
		distance += etiSpeed * delta;
		etiSpeed += delta ; //making world move faster relative to time passed
		
		levelLoop(&distance);

		
		
		


		dash(&isDashing, &dashTime, &speedChangeToken, &isFalling, &isJumping, &etiSpeed, &jumpCounter);
		dashCounterReset(&isFalling, &isJumping, &isDashing, &dashCounter);
		jump(&etiHeight, &isJumping, &isFalling, &jumper, &jumpCounter, &isDashing);
		topHit(&distance, &etiHeight, &isFalling, &isJumping);
		
		falling(&etiHeight, distance, &isFalling);
		

		DrawSurface(screen, background, SCREEN_WIDTH*2 - distance + X_OFFSET, Y_OFFSET - etiHeight);

		DrawSurface(screen, eti, ETI_LEFT_FIX, SCREEN_HEIGHT/2);
		if (!inLevelCheck(&etiHeight) || collision(&distance, &etiHeight) || terrainCollision(&distance, &etiHeight))
		{
			//this ends the round if the player collides with an obstacle or falls out of the screen
			if(isDashing == 1) isDashing = 0; //this is to make sure the speed doesn't get decreased after dying while dashing
			lives -= 1;
			roundOver(&frames, &fpsTimer, &fps, &quit, &worldTime, &distance, &etiSpeed, &etiHeight, &jumper);
		}


		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
			};
		// tekst informacyjny / info text
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
		sprintf(text, "lives = %f, elapsed time = %.1lf s  %.0lf fps",etiHeight, worldTime, fps);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		//	      "Esc - exit, \030 - faster, \031 - slower"
		sprintf(text, "control mode = %d, \033 - accelerate, \032 - slow down", isJumping);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
		
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
//		SDL_RenderClear(renderer);

		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
		// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if(event.key.keysym.sym == SDLK_RIGHT) etiSpeed *= 2.0;
					else if(event.key.keysym.sym == SDLK_LEFT) etiSpeed *= 0.3;
					else if (event.key.keysym.sym == SDLK_n) {
						roundOver(&frames, &fpsTimer, &fps, &quit, &worldTime, &distance, &etiSpeed, &etiHeight,&jumper);
					}
					else if (event.key.keysym.sym == SDLK_UP && controlMode == 0) {
						if (isJumping == 0 && jumper >= 0  && jumpCounter<2)
						{
							isJumping = 1;
							isFalling = 0;
							jumpCounter++;
							jumper = 150;
						}
					}
					else if (event.key.keysym.sym == SDLK_z && controlMode == 1) {
						if (isJumping == 0 && jumper >= 0 && jumpCounter < 2)
						{
							isJumping = 1;
							isFalling = 0;
							jumpCounter++;
							jumper = 150;
						}
					}
					else if (event.key.keysym.sym == SDLK_d)
					{
						if (controlMode == 0) controlMode = 1;
						else if (controlMode == 1) controlMode = 0;
					}
					else if (event.key.keysym.sym == SDLK_x)
					{
						if (dashCounter == 0)
						{
							isDashing = 1;
							dashCounter = 1;
						}
					}
					break;
				case SDL_KEYUP:
					isJumping = 0;
					break;
					
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		
		frames++;
		
		};

	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
	};
