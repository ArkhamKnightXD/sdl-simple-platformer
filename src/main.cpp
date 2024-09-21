#include "sdl_starter.h"
#include "sdl_assets_loader.h"
#include <vector>

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_GameController *controller = nullptr;

Mix_Chunk *actionSound = nullptr;
Mix_Music *music = nullptr;

Sprite playerSprite;

const int PLAYER_SPEED = 50;

float velocityX;
float velocityY;

bool isGamePaused;
bool playerCanJump;

SDL_Rect platform = {0, SCREEN_HEIGHT - 32, SCREEN_WIDTH, 32};
SDL_Rect platform2 = {100, SCREEN_HEIGHT - 68, 36, 36};

std::vector<SDL_Rect> platforms;

SDL_Texture *pauseTexture = nullptr;
SDL_Rect pauseBounds;

TTF_Font *fontSquare = nullptr;

void quitGame()
{
    Mix_FreeMusic(music);
    Mix_FreeChunk(actionSound);
    SDL_DestroyTexture(playerSprite.texture);
    SDL_DestroyTexture(pauseTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
        {
            quitGame();
            exit(0);
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p)
        {
            isGamePaused = !isGamePaused;
            Mix_PlayChannel(-1, actionSound, 0);
        }

        if (event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_START)
        {
            isGamePaused = !isGamePaused;
            Mix_PlayChannel(-1, actionSound, 0);
        }
    }
}

bool checkCollisionInX(SDL_Rect player, SDL_Rect platform)
{
    return player.x + player.w > platform.x && player.x < platform.x + platform.w;
}

bool checkCollisionInY(SDL_Rect player, SDL_Rect platform)
{
    return player.y + player.h > platform.y && player.y < platform.y + platform.h;
}

SDL_Rect getPreviousPosition(SDL_Rect &playerBounds)
{
    int positionX = playerBounds.x - velocityX;
    int positionY = playerBounds.y - velocityY;

    return {positionX, positionY, playerBounds.w, playerBounds.h};
}

void update(float deltaTime)
{
    const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);

    velocityY += 20.8f * deltaTime;

    playerSprite.textureBounds.y += velocityY;
    playerSprite.textureBounds.x += velocityX;
    velocityX *= 0.9f;

    if (playerSprite.textureBounds.y > SCREEN_HEIGHT)
    {
        playerSprite.textureBounds.y = 0;
        playerSprite.textureBounds.x = SCREEN_WIDTH / 2;
        velocityY = 0;
        velocityX = 0;
    }

    for (SDL_Rect &platform : platforms)
    {
        if (SDL_HasIntersection(&playerSprite.textureBounds, &platform))
        {
            if (checkCollisionInX(getPreviousPosition(playerSprite.textureBounds), platform))
            {
                if (velocityY > 0)
                {
                    playerSprite.textureBounds.y = platform.y - playerSprite.textureBounds.h;
                    velocityY = 0;
                }

                else
                {
                    playerSprite.textureBounds.y = platform.y + platform.h;
                    velocityY = 0;
                }
            }
            else if (checkCollisionInY(getPreviousPosition(playerSprite.textureBounds), platform))
            {
                if (velocityX > 0)
                {
                    playerSprite.textureBounds.x = platform.x - playerSprite.textureBounds.w;
                    velocityX = 0;
                }

                else
                {
                    playerSprite.textureBounds.x = platform.x + platform.w;
                    velocityX = 0;
                }
            }
        }
    }

    if (currentKeyStates[SDL_SCANCODE_A])
    {
        velocityX -= PLAYER_SPEED * deltaTime;
    }

    else if (currentKeyStates[SDL_SCANCODE_D])
    {
        velocityX += PLAYER_SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
    {
        playerSprite.textureBounds.x -= PLAYER_SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
    {
        playerSprite.textureBounds.x += PLAYER_SPEED * deltaTime;
    }
}

void renderSprite(Sprite sprite)
{
    SDL_RenderCopy(renderer, sprite.texture, NULL, &sprite.textureBounds);
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    renderSprite(playerSprite);

    for (SDL_Rect &platform : platforms) {

        SDL_RenderFillRect(renderer, &platform);
    }

    if (isGamePaused)
    {
        SDL_RenderCopy(renderer, pauseTexture, NULL, &pauseBounds);
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char *args[])
{
    window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (startSDL(window, renderer) > 0)
    {
        return 1;
    }

    if (SDL_NumJoysticks() < 1)
    {
        SDL_Log("No game controllers connected!");
        // return -1;
    }
    else
    {
        controller = SDL_GameControllerOpen(0);
        if (controller == NULL)
        {
            SDL_Log("Unable to open game controller! SDL Error: %s\n", SDL_GetError());
            return -1;
        }
    }

    fontSquare = TTF_OpenFont("res/fonts/square_sans_serif_7.ttf", 36);

    updateTextureText(pauseTexture, "Game Paused", fontSquare, renderer);

    SDL_QueryTexture(pauseTexture, NULL, NULL, &pauseBounds.w, &pauseBounds.h);
    pauseBounds.x = SCREEN_WIDTH / 2 - pauseBounds.w / 2;
    pauseBounds.y = 100;

    playerSprite = loadSprite(renderer, "res/sprites/alien_1.png", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    actionSound = loadSound("res/sounds/magic.wav");

    Mix_VolumeChunk(actionSound, MIX_MAX_VOLUME / 2);

    music = loadMusic("res/music/music.wav");

    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

    // Mix_PlayMusic(music, -1);

    platforms.push_back(platform);
    platforms.push_back(platform2);

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime = previousFrameTime;
    float deltaTime = 0.0f;

    while (true)
    {
        currentFrameTime = SDL_GetTicks();
        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f;
        previousFrameTime = currentFrameTime;

        SDL_GameControllerUpdate();

        handleEvents();

        if (!isGamePaused)
        {
            update(deltaTime);
        }

        render();

        capFrameRate(currentFrameTime);
    }

    quitGame();
}