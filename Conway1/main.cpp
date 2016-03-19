//
//  main.cpp
//  Conway1
//
//  Created by Koissi Adjorlolo on 2/26/16.
//  Copyright © 2016 ;. All rights reserved.
//

#include <iostream>
#include <SDL2/SDL.h>

typedef struct
{
    int x, y;
} Vector2i;

typedef struct
{
    bool alive;
    bool toDie;
} Cell;

static const char *TITLE = "Conway's Game of Life";
static const int WINDOW_POSX = SDL_WINDOWPOS_UNDEFINED;
static const int WINDOW_POSY = SDL_WINDOWPOS_UNDEFINED;
static const int WINDOW_WIDTH = 1024;
static const int WINDOW_HEIGHT = 1024;
static const Uint32 WINDOW_FLAGS = 0;
static const Uint32 RENDERER_FLAGS = SDL_RENDERER_ACCELERATED |
                                     SDL_RENDERER_PRESENTVSYNC;

static const double MS_PER_UPDATE = 1000.0 / 4;
static const int BOARD_WIDTH = 64;
static const int BOARD_HEIGHT = 64;
static Cell BOARD[BOARD_HEIGHT][BOARD_WIDTH];
static const int CELL_WIDTH = WINDOW_WIDTH / BOARD_WIDTH;
static const int CELL_HEIGHT = WINDOW_HEIGHT / BOARD_HEIGHT;

static void init();
static void quit();
static void update();
static void updateCellAtPosition(Vector2i position);
static void render();
static void renderBoard();
static void renderMouseRect(SDL_Rect mouseRect);
static void setCellAtPosition(Vector2i position, Cell value);
static Cell getCellAtPosition(Vector2i position);
static Vector2i getMouseCellPosition();

static SDL_Window *gWindow = nullptr;
static SDL_Renderer *gRenderer = nullptr;
static bool gRunning = false;
static bool gUpdateCells = false;

static SDL_Rect gMouseRect = {
    0,
    0,
    CELL_WIDTH,
    CELL_HEIGHT
};

int main(int argc, const char * argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "Unable to init SDL" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        exit(1);
    }
    
    gWindow = SDL_CreateWindow(TITLE,
                               WINDOW_POSX,
                               WINDOW_POSY,
                               WINDOW_WIDTH,
                               WINDOW_HEIGHT,
                               WINDOW_FLAGS);
    
    if (gWindow == nullptr)
    {
        std::cout << "Unable to create window" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }
    
    gRenderer = SDL_CreateRenderer(gWindow, -1, RENDERER_FLAGS);
    
    if (gRenderer == nullptr)
    {
        std::cout << "Unable to create renderer" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        SDL_DestroyWindow(gWindow);
        gWindow = nullptr;
        SDL_Quit();
        exit(1);
    }
    
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    
    SDL_Event event;
    gRunning = true;
    init();
    
    double previous = SDL_GetTicks();
    double lag = 0.0;
    
    while (gRunning)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    gRunning = false;
                    break;
                    
                case SDL_MOUSEMOTION:
                    gMouseRect.x = (event.motion.x / CELL_WIDTH) * CELL_WIDTH;
                    gMouseRect.y = (event.motion.y / CELL_HEIGHT) * CELL_HEIGHT;
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    
                    switch (event.button.button)
                    {
                        case SDL_BUTTON_LEFT:
                            setCellAtPosition(getMouseCellPosition(), {
                                true,
                                getCellAtPosition(getMouseCellPosition()).toDie
                            });
                            break;
                            
                        case SDL_BUTTON_RIGHT:
                            setCellAtPosition(getMouseCellPosition(), {
                                false,
                                getCellAtPosition(getMouseCellPosition()).toDie
                            });
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case SDL_KEYDOWN:
                    
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_s:
                            gUpdateCells = true;
                            break;
                            
                        case SDLK_p:
                            gUpdateCells = false;
                            break;
                            
                        case SDLK_RETURN:
                            init();
                            gUpdateCells = false;
                            break;
                            
                        default:
                            break;
                    }
                    
                    break;
                    
                case SDL_KEYUP:
                    
                    break;
                    
                default:
                    break;
            }
        }
        
        double current = SDL_GetTicks();
        double elapsed = current - previous;
        lag += elapsed;
        previous = current;
        
        while (lag >= MS_PER_UPDATE)
        {
            update();
            lag -= MS_PER_UPDATE;
        }
        
        render();
    }
    
    quit();
    
    return 0;
}

static void init()
{
    // Clear board.
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            setCellAtPosition({ x, y }, { false, false });
        }
    }
}

static void quit()
{
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gRenderer = nullptr;
    gWindow = nullptr;
    
    SDL_Quit();
}

static void update()
{
    if (gUpdateCells)
    {
        for (int y = 0; y < BOARD_HEIGHT; y++)
        {
            for (int x = 0; x < BOARD_WIDTH; x++)
            {
                updateCellAtPosition({ x, y });
            }
        }
        
        for (int y = 0; y < BOARD_HEIGHT; y++)
        {
            for (int x = 0; x < BOARD_WIDTH; x++)
            {
                setCellAtPosition({ x, y }, {
                    getCellAtPosition({ x, y }).toDie,
                    false
                });
            }
        }
    }
}

static void updateCellAtPosition(Vector2i position)
{
    bool cell = getCellAtPosition(position).alive;
    bool tL = getCellAtPosition({ position.x - 1, position.y - 1 }).alive;
    bool tC = getCellAtPosition({ position.x, position.y - 1 }).alive;
    bool tR = getCellAtPosition({ position.x + 1, position.y - 1 }).alive;
    bool cL = getCellAtPosition({ position.x - 1, position.y }).alive;
    bool cR = getCellAtPosition({ position.x + 1, position.y }).alive;
    bool bL = getCellAtPosition({ position.x - 1, position.y + 1 }).alive;
    bool bC = getCellAtPosition({ position.x, position.y + 1 }).alive;
    bool bR = getCellAtPosition({ position.x + 1, position.y + 1 }).alive;
    
    // I hope this works.  I think it will.
    int aliveCount = (int)tL + (int)tC + (int)tR + (int)cL +
                     (int)cR + (int) bL + (int)bC + (int)bR;
    
    if (cell)
    {
        if (aliveCount < 2)
        {
            setCellAtPosition(position, { true, false });
        }
        else if (aliveCount > 3)
        {
            setCellAtPosition(position, { true, false });
        }
        else
        {
            setCellAtPosition(position, { true, true });
        }
    }
    else
    {
        if (aliveCount == 3)
        {
            setCellAtPosition(position, { false, true });
        }
    }
}

static void render()
{
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
    SDL_RenderClear(gRenderer);
    
    renderBoard();
    renderMouseRect(gMouseRect);
    
    SDL_RenderPresent(gRenderer);
}

static void renderBoard()
{
    SDL_Rect rect = {
        0,
        0,
        CELL_WIDTH,
        CELL_HEIGHT
    };
    
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            rect.x = x * CELL_WIDTH;
            rect.y = y * CELL_HEIGHT;
            
            if (getCellAtPosition({ x, y }).alive)
            {
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(gRenderer, 200, 200, 200, 255);
            }
            
            SDL_RenderFillRect(gRenderer, &rect);
        }
    }
}

static void renderMouseRect(SDL_Rect mouseRect)
{
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 63);
    SDL_RenderFillRect(gRenderer, &mouseRect);
}

static void setCellAtPosition(Vector2i position, Cell cell)
{
    if (position.x < 0 || position.x >= BOARD_WIDTH ||
        position.y < 0 || position.x >= BOARD_HEIGHT)
    {
        return;
    }
    
    BOARD[position.y][position.x] = cell;
}

static Cell getCellAtPosition(Vector2i position)
{
    if (position.x < 0 || position.x >= BOARD_WIDTH ||
        position.y < 0 || position.x >= BOARD_HEIGHT)
    {
        return {};
    }
    
    return BOARD[position.y][position.x];
}

static Vector2i getMouseCellPosition()
{
    return {
        gMouseRect.x / CELL_WIDTH,
        gMouseRect.y / CELL_HEIGHT
    };
}
