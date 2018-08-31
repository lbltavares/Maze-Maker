/*
 * Gerador de labirintos - Maze Maker -  v0.1
 * Autor: Lucas Branco Laborne Tavares
 * Algoritmo: Backtracking
 * Data: 31/08/2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "SDL2/SDL.h"
#include "define.h"
#include "vector.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Event event;

SDL_Point currentBuilderPoint;

int running = false;

int map[NUM_COLS % 2 ? NUM_COLS + 1 : NUM_COLS][NUM_ROWS % 2 ? NUM_ROWS + 1 : NUM_ROWS];

vector builder;

void initSDL();
void initMap();
void quitSDL();

void loop();
void update();
void render();

void build();

void renderMap();

void printMap();

void initBuilder();

int getTile(int c, int r);
int setTile(int c, int r, int v);

void checkBuildMove(int c, int r, vector *v);

void initSDL()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("SDL_Init error: %s\n", SDL_GetError());
        exit(ERROR);
    }
    window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        exit(ERROR);
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
        exit(ERROR);
    }
}

void initMap()
{
    srand(time(NULL));
    for (int c = 0; c < NUM_COLS; c++)
    {
        for (int r = 0; r < NUM_ROWS; r++)
        {
            setTile(c, r, WALL);
        }
    }
    setTile(BUILDER_STARTING_COL, BUILDER_STARTING_ROW, PATH);
}

void initBuilder()
{
    vector_init(&builder);
    SDL_Point *p = (SDL_Point *)malloc(sizeof(SDL_Point));
    p->x = BUILDER_STARTING_COL;
    p->y = BUILDER_STARTING_ROW;
    vector_add(&builder, p);
}

void quitSDL()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void loop()
{
    running = true;
    long time = SDL_GetTicks();
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
        }
        if (time + 1000 / FPS > SDL_GetTicks())
            continue;
        time = SDL_GetTicks();
        update();
        render();
    }
}

void printMap()
{
    for (int c = 0; c < NUM_COLS; c++)
    {
        for (int r = 0; r < NUM_ROWS; r++)
        {
            printf("%d ", getTile(c, r));
        }
        printf("\n");
    }
    printf("\n");
}

int getTile(int c, int r)
{
    if (c >= 0 && c < NUM_COLS && r >= 0 && r < NUM_ROWS)
        return map[c][r];
    else
        return ERROR;
}

int setTile(int c, int r, int v)
{
    if (c >= 0 && c < NUM_COLS && r >= 0 && r < NUM_ROWS)
    {
        map[c][r] = v;
        return SUCCESS;
    }
    else
        return ERROR;
}

void checkBuildMove(int c, int r, vector *v)
{
    if (getTile(c, r) == WALL)
    {
        SDL_Point *p = (SDL_Point *)malloc(sizeof(SDL_Point));
        p->x = c;
        p->y = r;
        vector_add(v, p);
    }
}

void build()
{
    if (!vector_isEmpty(&builder))
    {
        SDL_Point p = *((SDL_Point *)vector_get(&builder, builder.count - 1));
        int c = p.x;
        int r = p.y;
        int offset = 2;

        vector possiveis;
        vector_init(&possiveis);
        checkBuildMove(c + offset, r, &possiveis); // Leste
        checkBuildMove(c, r + offset, &possiveis); // Sul
        checkBuildMove(c - offset, r, &possiveis); // Oeste
        checkBuildMove(c, r - offset, &possiveis); // Norte

        if (possiveis.count > 0)
        {
            SDL_Point *rand_point = (SDL_Point *)vector_get(&possiveis, rand() % possiveis.count);
            vector_add(&builder, rand_point);
            int rc = rand_point->x;
            int rr = rand_point->y;
            int modulo = (int)sqrt((rc - c) * (rc - c) + (rr - r) * (rr - r));
            setTile(rc, rr, PATH);
            setTile(rc - ((rc - c) / modulo), rr - ((rr - r) / modulo), PATH);
            vector_free(&possiveis);

            currentBuilderPoint.x = rc;
            currentBuilderPoint.y = rr;
        }
        else
        {
            currentBuilderPoint.x = c;
            currentBuilderPoint.y = r;

            vector_delete(&builder, builder.count - 1);
        }
    }
    else
    {
        initMap();
        initBuilder();
    }
}

void update()
{
    static long builderTime = 0;
    if (builderTime + BUILD_DELAY > SDL_GetTicks())
        return;
    builderTime = SDL_GetTicks();
    build();
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    renderMap();

    SDL_RenderPresent(renderer);
}

void renderMap()
{
    int TILE_WIDTH = WINDOW_WIDTH / NUM_COLS;
    int TILE_HEIGHT = WINDOW_HEIGHT / NUM_ROWS;

    for (int c = 0; c < NUM_COLS; c++)
    {
        for (int r = 0; r < NUM_ROWS; r++)
        {
            SDL_Rect tile;
            tile.x = c * TILE_WIDTH;
            tile.y = r * TILE_HEIGHT;
            tile.w = TILE_WIDTH;
            tile.h = TILE_HEIGHT;
            if (getTile(c, r) == PATH)
                SDL_SetRenderDrawColor(renderer, PATH_COLOR);
            else if (getTile(c, r) == WALL)
                SDL_SetRenderDrawColor(renderer, WALL_COLOR);

            if (c == currentBuilderPoint.x && r == currentBuilderPoint.y)
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

            SDL_RenderFillRect(renderer, &tile);
        }
    }
}

int main(int argv, char **args)
{
    initSDL();
    initMap();
    initBuilder();
    loop();
    quitSDL();
    return 0;
}