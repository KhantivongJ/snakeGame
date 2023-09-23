#pragma once
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_image.h"
#include <fstream>

using namespace std;

struct Vector2
{
	float x;
	float y;
};

class Game
{
public:
	Game();
	bool Initialize();
	void RunLoop();
	void Shutdown();
private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();
	SDL_Event event;
	// Background and Music
	void DrawWalls();
	Mix_Chunk* gLTurn;
	Mix_Chunk* gRTurn;
	Mix_Chunk* gOver;
	Mix_Chunk* gEat;
	Mix_Music* gBGM;

	// Snake
	void DrawSnake();
	void GenerateSnake();
	void MoveSnake();
	void SnakeDirection(SDL_Keycode);
	void SnakeCollision();
	SDL_Rect snake[((1024 - 20 * 2) * (768 - 20 * 2)) / (20 * 20)]; 
	int ySnake;
	int xSnake;
	// Snake Head
	SDL_Surface* snakeHead;
	SDL_Texture* snakeHeadTexture;

	// Food
	void DrawFood();
	void GenerateFood();
	SDL_Rect food;

	// Game Over 
	TTF_Font* theFont;
	int fontWidth;
	int fontHeight;
	SDL_Color textColor;
	SDL_Surface* textSurface;
	SDL_Texture* fontTexture;

	// Score
	void PrintScore();
	int scoreWidth;
	int scoreHeight;
	SDL_Surface* scoreSurface;
	SDL_Texture* scoreTexture;
	int score = 0;

	// HiScore
	fstream hiScore;
	int hiScoreACC;



	int tickInterval;
	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;
	Uint32 mTicksCount;
	bool mIsRunning;

};
