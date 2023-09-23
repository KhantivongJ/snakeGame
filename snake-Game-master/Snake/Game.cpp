
#include "Game.h"
#include <iostream>
#include <string>
using namespace std;

const int WALL_THICKNESS = 20; // Original = 20
const int SCREEN_WIDTH = 1000; // Original = 1000
const int SCREEN_HEIGHT = 800;// Original = 800

const int SNAKE_SIZE = 25; // Original = 25
// How many Snake blocks can fit into screen
// Size of All Walls / Size of Snake
const int SNAKE_COUNT = ((SCREEN_WIDTH - WALL_THICKNESS * 2) * (SCREEN_HEIGHT - WALL_THICKNESS * 2)) /
(SNAKE_SIZE * SNAKE_SIZE);

Game::Game()
{
	mWindow=nullptr;
	mRenderer=nullptr;
	mTicksCount=0;
	mIsRunning=true;
	tickInterval = 40; // Initial Speed of Snake
}

bool Game::Initialize()
{
	// Initialize SDL
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);
	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}
	
	// Create an SDL Window
	mWindow = SDL_CreateWindow(
		"Score = 0", // Window title
		100,	// Top left x-coordinate of window
		100,	// Top left y-coordinate of window
		SCREEN_WIDTH,	// Width of window
		SCREEN_HEIGHT,	// Height of window
		0		// Flags (0 for no flags set)
	);

	if (!mWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}
	
	//// Create SDL renderer
	mRenderer = SDL_CreateRenderer(
		mWindow, // Window to create renderer for
		-1,		 // Usually -1
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	if (!mRenderer)
	{
		SDL_Log("Failed to create renderer: %s", SDL_GetError());
		return false;
	}
	// Snake
	xSnake = SNAKE_SIZE;
	ySnake = 0;
	GenerateSnake();

	// Food 
	GenerateFood();
	food.w = SNAKE_SIZE;
	food.h = SNAKE_SIZE;

	// Music
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024);
	Mix_VolumeMusic(15);
	//Mix_VolumeChunk(4);
	int chunkVolume = 15;
	gLTurn = Mix_LoadWAV("lTurn.wav");
	gRTurn = Mix_LoadWAV("rTurn.wav");
	gOver = Mix_LoadWAV("gOver.wav");
	gEat = Mix_LoadWAV("gEat.wav");
	gBGM = Mix_LoadMUS("bgm.wav");
	Mix_FadeInMusic(gBGM, -1, 1000);
	Mix_VolumeChunk(gLTurn, chunkVolume);
	Mix_VolumeChunk(gRTurn, chunkVolume);
	Mix_VolumeChunk(gOver, chunkVolume);
	Mix_VolumeChunk(gEat, chunkVolume+15);

	// Game Over 
	TTF_Init();
	theFont = TTF_OpenFont("font.ttf", 300);
	textColor = { 255, 0, 0 };
	string textureText = "GAME OVER";
	textSurface = TTF_RenderText_Solid(theFont, textureText.c_str(), textColor);
	fontTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
	fontWidth = textSurface->w;
	fontHeight = textSurface->h;

	// Hi Score
	// Open file for reading
	hiScore.open("hiScore.txt", fstream::in);
	if (hiScore.is_open())
	{
		hiScore >> hiScoreACC;
	}
	hiScore.close();
	
	SDL_FreeSurface(textSurface);

	// Snake Head
	IMG_Init(IMG_INIT_PNG);
	snakeHead = IMG_Load("snakeHead.png");
	snakeHeadTexture = SDL_CreateTextureFromSurface(mRenderer, snakeHead);
	SDL_FreeSurface(snakeHead);
	return true;
}

void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
	PrintScore();
	SDL_Delay(3000);
}

void Game::ProcessInput()
{
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			// If we get an SDL_QUIT event, end loop
			case SDL_QUIT:
				mIsRunning = false;
				break;
		}
		if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_w
			|| event.key.keysym.sym == SDLK_s
			|| event.key.keysym.sym == SDLK_a
			|| event.key.keysym.sym == SDLK_d))
		{
			SnakeDirection(event.key.keysym.sym);
		}
	}
	
	// Get state of keyboard
	const Uint8* state = SDL_GetKeyboardState(NULL);
	
	// Mute and Unmute Music
	if (state[SDL_SCANCODE_M])
	{
		if (Mix_PlayingMusic() == 0)
		{
			Mix_PlayMusic(gBGM, -1);
		}
		else
		{
			if (Mix_PausedMusic() == 1)
			{
				Mix_ResumeMusic();
			}
			else
			{
				Mix_PauseMusic();
			}
		}
	}
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}

	// Increase Speed of Snake
	if (state[SDL_SCANCODE_UP])
	{
		if (tickInterval >= 30)
			tickInterval -= 5;
	}

	// Decrease Speed of Snake
	if (state[SDL_SCANCODE_DOWN])
	{
		if (tickInterval <= 55)
			tickInterval += 5;
	}

	
}

void Game::UpdateGame()
{
	// Wait until 16ms has elapsed since last frame
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + tickInterval))
		;

	// Delta time is the difference in ticks from last frame
	// (converted to seconds)
	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
	
	// Clamp maximum delta time value
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	// Update tick counts (for next frame)
	mTicksCount = SDL_GetTicks();
	
}

void Game::GenerateOutput()
{
	SDL_SetRenderDrawColor(
		mRenderer,
		0,		// R
		0,		// G 
		0,	// B
		255		// A
	);
	
	// Clear back buffer
	SDL_RenderClear(mRenderer);

	// Draw Walls
	DrawWalls();
	DrawSnake();
	MoveSnake();
	SnakeCollision();
	DrawFood();
	
	
	SDL_RenderPresent(mRenderer);
}

void Game::DrawWalls()
{
	SDL_SetRenderDrawColor(mRenderer, 200, 210, 205, 255);
	SDL_Rect wall = {
		0, // x
		0, // y
		WALL_THICKNESS, // width 
		SCREEN_HEIGHT // height
	};

	// Left Wall
	SDL_RenderFillRect(mRenderer, &wall);

	// Right Wall
	wall.x = SCREEN_WIDTH - WALL_THICKNESS;
	SDL_RenderFillRect(mRenderer, &wall);

	// Top Wall
	wall.x = 0;
	wall.w = SCREEN_WIDTH;
	wall.h = WALL_THICKNESS;
	SDL_RenderFillRect(mRenderer, &wall);

	// Bottom Wall
	wall.y = SCREEN_HEIGHT - WALL_THICKNESS;
	SDL_RenderFillRect(mRenderer, &wall);
}

void Game::DrawSnake() 
{
	// Render Snake Head Image to snake[0]
	SDL_RenderCopy(mRenderer, snakeHeadTexture, NULL, &snake[0]);
	SDL_RenderPresent(mRenderer);

	// Rest of Body
	for (int i = 1; i < sizeof(snake) / sizeof(snake[0]); i++)
	{
		if (snake[i].w == 0) {
			break;
		}

		// Draw Body of snake
		SDL_SetRenderDrawColor(mRenderer, 69, 176, 57, 122);
		SDL_RenderFillRect(mRenderer, &snake[i]);

		// Creates a border around the snake bodies
		SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 40);
		SDL_RenderDrawRect(mRenderer, &snake[i]);
	}
}

void Game::GenerateSnake()
{
	for (int i = 0; i < sizeof(snake) / sizeof(snake[0]); i++) {
		snake[i].x = 0;
		snake[i].y = 0;
		snake[i].w = 0;
		snake[i].h = 0;
	}

	// Head Position
	snake[0].x = SCREEN_WIDTH / 2 - SNAKE_SIZE;
	snake[0].y = SCREEN_HEIGHT / 2 - SNAKE_SIZE;
	snake[0].w = SNAKE_SIZE;
	snake[0].h = SNAKE_SIZE;

	// Increase Initial Size of Snake to 3 
	for (int i = 1; i < 3; i++)
	{
		snake[i] = snake[0];
		snake[i].x = snake[0].x - (SNAKE_SIZE * i);
	}
}

void Game::MoveSnake() 
{
	// Shifts everything right to make room for snake head 
	for (int i = sizeof(snake) / sizeof(snake[0]) - 1; i >= 1; i--)
	{
		snake[i] = snake[i-1];
	}

	snake[0].x = snake[1].x + xSnake;
	snake[0].y = snake[1].y + ySnake;
	snake[0].w = SNAKE_SIZE;
	snake[0].h = SNAKE_SIZE;

	// If Collision with food is detected then don't remove tail of snake
	if (food.x == snake[0].x && food.y == snake[0].y) 
	{
		Mix_PlayChannel(-1, gEat, 0);
		score++;
		GenerateFood();
	}
	else 
	{
		// Removes the end of the snake
		// Current Size / Starting Size = size of snake to draw
		for (int i = 3; i < sizeof(snake) / sizeof(snake[0]); i++)
		{
			if (snake[i].w == 0)
			{
				snake[i - 1].x = 0;
				snake[i - 1].y = 0;
				snake[i - 1].w = 0;
				snake[i - 1].h = 0;
				break;
			}
		}
	}
}

void Game::SnakeDirection(SDL_Keycode direction)
{
	// Tests if ySnake or xSnake is in a certain direction
	int moveUp = ySnake == -SNAKE_SIZE;
	int moveDown = ySnake == SNAKE_SIZE;
	int moveLeft = xSnake == -SNAKE_SIZE;
	int moveRight = xSnake == SNAKE_SIZE;

	if (direction == SDLK_w && !moveDown)
	{
		xSnake = 0;
		ySnake = -SNAKE_SIZE;
		Mix_PlayChannel(-1, gLTurn, 0);
	}

	if (direction == SDLK_s && !moveUp)
	{
		xSnake = 0;
		ySnake = SNAKE_SIZE;
		Mix_PlayChannel(-1, gLTurn, 0);
	}

	if (direction == SDLK_a && !moveRight)
	{
		ySnake = 0;
		xSnake = -SNAKE_SIZE;
		Mix_PlayChannel(-1, gRTurn, 0);
	}

	if (direction == SDLK_d && !moveLeft)
	{
		ySnake = 0;;
		xSnake = SNAKE_SIZE;
		Mix_PlayChannel(-1, gRTurn, 0);
	}
}

void Game::SnakeCollision()
{
	for (int i = 1; i < sizeof(snake) / sizeof(snake[0]); i++)
	{
		// Exits loop once at the end of live snake body
		if (snake[i].w == 0)
		{
			break;
		}

		// Check for self collision
		if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
		{
			mIsRunning = 0;
			return;
		}
	}

	// Left side collision
	if (snake[0].x < WALL_THICKNESS) {
		mIsRunning = 0;
		return;
	}

	// Right wall collision
	if (snake[0].x > SCREEN_WIDTH - WALL_THICKNESS - SNAKE_SIZE)
	{
		mIsRunning = 0;
		return;
	}

	// Top wall collision
	if (snake[0].y < WALL_THICKNESS)
	{
		mIsRunning = 0;
		return;
	}

	// Bottom Wall Collision
	if (snake[0].y > SCREEN_HEIGHT - WALL_THICKNESS - SNAKE_SIZE)
	{
		mIsRunning = 0;
		return;
	}

}

void Game::GenerateFood() 
{
	// Generate Food at random spot on screen
	food.x = (rand() % (((SCREEN_WIDTH - SNAKE_SIZE - WALL_THICKNESS) / SNAKE_SIZE) + 1) * SNAKE_SIZE); 
	food.y = (rand() % (((SCREEN_HEIGHT - SNAKE_SIZE - WALL_THICKNESS) / SNAKE_SIZE) + 1) * SNAKE_SIZE); 

	// If number generated is below size of wall spawn it beside left wall
	if (food.x < WALL_THICKNESS)
	{
		food.x = WALL_THICKNESS + 5;
	}

	// If number generated is less than top wall Spawn under top wall
	if (food.y < WALL_THICKNESS)
	{
		food.y = WALL_THICKNESS + 5;
	}

	for (int i = 0; i < sizeof(snake) / sizeof(snake[0]); i++)
	{
		// Exits loop if at end of active snake
		if (snake[i].w == 0)
		{
			break;
		}

		if (snake[i].x == food.x && snake[i].y == food.y)
		{
			GenerateFood();
			break;
		}
	}

}

void Game::DrawFood()
{
	int color1 = rand();
	int color2 = rand();
	int color3 = rand();

	SDL_SetRenderDrawColor(mRenderer, color1, color2, color3, 255);
	SDL_RenderFillRect(mRenderer, &food);
}

void Game::PrintScore()
{
	Mix_HaltMusic();
	Mix_PlayChannel(-1, gOver, 0);
	SDL_Rect textRender = { (SCREEN_WIDTH - fontWidth) / 2, ((SCREEN_HEIGHT - fontHeight) / 2) - 25, fontWidth, fontHeight };
	SDL_RenderCopyEx(mRenderer, fontTexture, NULL, &textRender, 0, NULL, SDL_FLIP_NONE);
	

	// Score
	theFont = TTF_OpenFont("font.ttf", 100);
	textColor = { 255, 255, 0 };
	string scoreText = "SCORE: " + to_string(score);
	scoreSurface = TTF_RenderText_Solid(theFont, scoreText.c_str(), textColor);
	scoreTexture = SDL_CreateTextureFromSurface(mRenderer, scoreSurface);

	scoreWidth = scoreSurface->w;
	scoreHeight = scoreSurface->h;

	SDL_FreeSurface(scoreSurface);

	SDL_Rect scoreRender = { (SCREEN_WIDTH - scoreWidth) / 2, ((SCREEN_HEIGHT - scoreHeight) / 2) + 180, scoreWidth, scoreHeight };
	SDL_RenderCopyEx(mRenderer, scoreTexture, NULL, &scoreRender, 0, NULL, SDL_FLIP_NONE);

	// Hi Score
	textColor = { 0, 255, 255 };
	if (score > hiScoreACC)
	{
		// If New High score use this text
		scoreText = "NEW HIGH SCORE: " + to_string(score);
	}
	else
	{
		// Otherwise use this text if not new high score
		scoreText = "HIGH SCORE: " + to_string(hiScoreACC);
	}
	scoreSurface = TTF_RenderText_Solid(theFont, scoreText.c_str(), textColor);
	scoreTexture = SDL_CreateTextureFromSurface(mRenderer, scoreSurface);

	scoreWidth = scoreSurface->w;
	scoreHeight = scoreSurface->h;

	SDL_FreeSurface(scoreSurface);

	scoreRender = { (SCREEN_WIDTH - scoreWidth) / 2, ((SCREEN_HEIGHT - scoreHeight) / 2) + 110, scoreWidth, scoreHeight };
	SDL_RenderCopyEx(mRenderer, scoreTexture, NULL, &scoreRender, 0, NULL, SDL_FLIP_NONE);

	if (score > hiScoreACC)
	{
		hiScore.open("hiScore.txt", fstream::trunc | fstream::out);
		hiScore << score;
	}
	hiScore.close();

	SDL_RenderPresent(mRenderer);
}

void Game::Shutdown()
{
	SDL_DestroyTexture(snakeHeadTexture);
	SDL_DestroyTexture(scoreTexture);
	SDL_DestroyTexture(fontTexture);
	Mix_FreeChunk(gEat);
	Mix_FreeChunk(gLTurn);
	Mix_FreeChunk(gRTurn);
	Mix_FreeChunk(gOver);
	Mix_FreeMusic(gBGM);
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}