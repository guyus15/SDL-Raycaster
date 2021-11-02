#include <stdio.h>
#include <iostream>
#include <SDL.h>
#include <math.h>

#define W 1024
#define H 512
#define FRAME_RATE 60

#define PI 3.1415926535
#define P2 PI / 2
#define P3 3 * PI / 2
#define DR 0.0174533 // one degree in radians

SDL_Renderer* renderer = NULL;

enum CurrentInput
{
	NONE = 0,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

struct Player 
{
	float xPos = 512 / 2, yPos = H / 2;
	float angle = 0, deltaX = cos(angle) * 5, deltaY = sin(angle) * 5;
	float moveSpeed = 10;

	int width = 16, height = 16;
};

struct Player* p = NULL;

struct Map
{
	int mapX = 8, mapY = 8, mapS = mapX * mapY, blockSize = 64;

	int map[64] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 
		1, 0, 0, 1, 0, 0, 0, 1,
		1, 0, 0, 1, 0, 0, 0, 1,
		1, 0, 0, 1, 1, 1, 0, 1,
		1, 0, 0, 1, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1
	};
};

struct Map map;

float dist(float ax, float ay, float bx, float by, float ang)
{
	return (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
}

void drawRays2D()
{
	int ray, mapX, mapY, mapP, depthOfField;
	float rayX = 0, rayY = 0, rayAngle, xOffset = 0, yOffset = 0, disT = 0;

	rayAngle = p->angle - DR * 30;

	if (rayAngle < 0)
		rayAngle += 2 * PI;
	if (rayAngle > 2 * PI)
		rayAngle -= 2 * PI;
	
	for (ray = 0; ray < 60; ray++)
	{
		
		depthOfField = 0;
		float disH = 1000000, hx = p->xPos, hy = p->yPos;
		float aTan = -1 / tan(rayAngle);


		if (rayAngle > PI) // Ray facing upwards
		{
			rayY = (((int)p->yPos >> 6) << 6) - 0.0001;
			rayX = (p->yPos - rayY) * aTan + p->xPos;

			yOffset = -map.blockSize;
			xOffset = -yOffset * aTan;
		}

		if (rayAngle < PI) // Ray facing downwards
		{
			rayY = (((int)p->yPos >> 6) << 6) + map.blockSize;
			rayX = (p->yPos - rayY) * aTan + p->xPos;

			yOffset = map.blockSize;
			xOffset = -yOffset * aTan;
		}

		if (rayAngle == 0 || rayAngle == PI)
		{
			rayX = p->xPos;
			rayY = p->yPos;
			depthOfField = 8;
		}

		while (depthOfField < 8)
		{
			mapX = (int)(rayX) >> 6; 
			mapY = (int)(rayY) >> 6;

			mapP = mapY * map.mapX + mapX;

			if (mapP > 0 && (mapP < map.mapX * map.mapY) && map.map[mapP] > 0)
			{
				hx = rayX;
				hy = rayY;
				disH = dist(p->xPos, p->yPos, hx, hy, rayAngle);

				depthOfField = 8;
			}
			else
			{
				rayX += xOffset;
				rayY += yOffset;
				depthOfField += 1;
			}
		}

		if (rayX > 64 && rayX < 512 - 64)
			SDL_RenderDrawLine(renderer, p->xPos, p->yPos, rayX, rayY);

		depthOfField = 0;
		float disV = 1000000, vx = p->xPos, vy = p->yPos;
		float nTan = -tan(rayAngle);

		if (rayAngle > P2 && rayAngle < P3) // Ray facing upwards
		{
			rayX = (((int)p->xPos >> 6) << 6) - 0.0001;
			rayY = (p->xPos - rayX) * nTan + p->yPos;

			xOffset = -map.blockSize;
			yOffset = -xOffset * nTan;
		}

		if (rayAngle < P2 || rayAngle > P3) // Ray facing downwards
		{
			rayX = (((int)p->xPos >> 6) << 6) + map.blockSize;
			rayY = (p->xPos - rayX) * nTan + p->yPos;

			xOffset = map.blockSize;
			yOffset = -xOffset * nTan;
		}

		if (rayAngle == 0 || rayAngle == PI)
		{
			rayX = p->xPos;
			rayY = p->yPos;
			depthOfField = 8;
		}

		while (depthOfField < 8)
		{
			mapX = (int)(rayX) >> 6;
			mapY = (int)(rayY) >> 6;

			mapP = mapY * map.mapX + mapX;

			if (mapP > 0 && mapP < (map.mapX * map.mapY) && map.map[mapP] > 0) // Ray facing left
			{
				vx = rayX;
				vy = rayY;
				disV = dist(p->xPos, p->yPos, vx, vy, rayAngle);

				depthOfField = 8;
			}
			else // Ray facing right
			{
				rayX += xOffset;
				rayY += yOffset;
				depthOfField += 1;
			}
		}
		
		if (disV < disH)
		{
			rayX = vx;
			rayY = vy;
			disT = disV;
			SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
		}
		if (disH < disV)
		{
			rayX = hx;
			rayY = hy;
			disT = disH;
			SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
		}

		if (rayY > 64 && rayY < 512 - 64)
			SDL_RenderDrawLine(renderer, p->xPos, p->yPos, rayX, rayY);

		rayAngle += DR;

		// Draw 3D Walls

		float cosAngle = p->angle - rayAngle;

		if (cosAngle < 0) { cosAngle += 2 * PI; }
		if (cosAngle > 2 * PI) { cosAngle -= 2 * PI; }

		disT *= cos(cosAngle);

		float lineH = (map.mapS * H) / disT;
		float lineO = (H /2) - (lineH / 2);

		if (lineH > H) { lineH = H; }

		SDL_RenderDrawLine(renderer, (ray * 8 + (W / 2)), lineO, (ray * 8 + (W / 2)), lineH + lineO);

		if (rayAngle < 0)
			rayAngle += 2 * PI;
		if (rayAngle > 2 * PI)
			rayAngle -= 2 * PI;
	}
}

void drawMap2D()
{
	for (int y = 0; y < map.mapY; y++)
	{
		for (int x = 0; x < map.mapX; x++)
		{
			SDL_Rect rect;

			rect.x = map.blockSize * x;
			rect.y = map.blockSize * y;
			rect.w = map.blockSize;
			rect.h = map.blockSize;

			if (map.map[y * map.mapX + x] == 1) 
			{
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

				for (int line = 1; line <= map.blockSize; line++)
				{
					SDL_RenderDrawLine(renderer, rect.x + line, rect.y, rect.x + line, rect.y + map.blockSize);
				}
			}
			else 
			{
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

				for (int line = 1; line <= map.blockSize; line++)
				{
					SDL_RenderDrawLine(renderer, rect.x + line, rect.y, rect.x + line, rect.y + map.blockSize);
				}
			}
		}
	}
}

void drawPlayer()
{
	SDL_Rect* rect = new SDL_Rect;

	rect->x = p->xPos - ((float)p->width / 2);
	rect->y = p->yPos - ((float)p->height / 2);
	rect->w = p->width;
	rect->h = p->height;

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_RenderDrawRect(renderer, rect); // draw the player square
	SDL_RenderDrawLine(renderer, p->xPos, p->yPos, p->xPos + (p->deltaX * 5), p->yPos + (p->deltaY * 5)); // draw player look angle

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // resets draw color back to black
}

void movePlayer(CurrentInput currentKey)
{
	if (currentKey == LEFT) 
	{
		//p->xPos -= (float)p->moveSpeed;
		p->angle -= 0.1;

		if (p->angle < 0) { p->angle += 2 * PI; }

		p->deltaX = cos(p->angle) * 5;
		p->deltaY = sin(p->angle) * 5;
	}
	if (currentKey == RIGHT)
	{
		//p->xPos += (float)p->moveSpeed;

		p->angle += 0.1;

		if (p->angle > 2*PI) { p->angle -= 2 * PI; }

		p->deltaX = cos(p->angle) * 5;
		p->deltaY = sin(p->angle) * 5;
	}

	if (currentKey == UP)
	{
		p->xPos += p->deltaX;
		p->yPos += p->deltaY;
	}

	if (currentKey == DOWN)
	{
		p->xPos -= p->deltaX;
		p->yPos -= p->deltaY;
	}
}

CurrentInput input(SDL_Event* e)
{
	switch (e->key.keysym.sym)
	{
		case SDLK_LEFT:
			return LEFT;
			break;
		case SDLK_RIGHT:
			return RIGHT;
			break;
		case SDLK_UP:
			return UP;
			break;
		case SDLK_DOWN:
			return DOWN;
			break;
		default:
			return NONE;
			break;
	}
}

int main(int argc, char* args[])
{
	bool quit = false;

	p = new Player;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		fprintf(stderr, "Could not init SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window* screen = SDL_CreateWindow("Raycaster",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		W, H,
		0);

	if (!screen)
	{
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE);

	if (!renderer)
	{
		fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
	}

	CurrentInput currentKey = NONE;
	SDL_Event e;

	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
				quit = true;
			else 
			{
				SDL_RenderClear(renderer);

				if (e.type == SDL_KEYDOWN)
					currentKey = input(&e);

				movePlayer(currentKey);
				
				drawMap2D();
				drawRays2D();
				drawPlayer();

				SDL_RenderPresent(renderer);

				currentKey = NONE;
			}
		}
	}

	SDL_DestroyWindow(screen);
	SDL_Quit();

	return 0;
}