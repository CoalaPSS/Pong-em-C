#include <stdio.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define WHITE 0xFFFFFFu
#define BLACK 0x000000u
#define RED 0xFF0000u

#define PADEL_SPEED 6
#define BALL_SPEED 3

static int global_running;

typedef struct
{
    SDL_Rect rect;
    int vely;
    int points;
}Padel;

typedef struct
{
    int x;
    int y;
    int radius;
    int velx;
    int vely;
    uint32_t color;
    int start_movement;

}Ball;

int Max(int a, int b)
{
	return (a > b) ? a : b;
}

void set_pixel(SDL_Surface *Surface, int x, int y, uint32_t color)
{
    uint32_t *Buffer = Surface->pixels;
    const int BUFFER_LIMIT = (Surface->w * Surface->h) - 1;
    int pixel_loc = (Surface->w * y) + x;

    if ((pixel_loc >= 0) && (pixel_loc <= BUFFER_LIMIT))
    {
        Buffer[pixel_loc] = color;
    }
    
}

void draw_line(SDL_Surface *Surface, int x0, int y0, int x1, int y1, uint32_t color)
{
    uint32_t *Buffer = Surface->pixels;
    const int BUFFER_LIMIT = (Surface->w * Surface->h) - 1;
    int dx = x1 - x0;
    int dy = y1 - y0;
    int step = Max(abs(dx), abs(dy));

    if (step != 0)
    {
        float stepx = (float)dx / step;
        float stepy = (float)dy / step;
        
        for (int i = 0; i < step; i++)
        {
            int x = (int)(x0 + (i * stepx));
            int y = (int)(y0 + (i * stepy));
            int pixel_loc = (Surface->w * y) + x;

            if ((pixel_loc >= 0) && (pixel_loc <= BUFFER_LIMIT))
            {
                Buffer[pixel_loc] = color;
            }
        }
    }
}

void draw_ball(SDL_Surface *Surface, Ball ball)
{
    int x0 = ball.x;
    int y0 = ball.y;
    int x = 0;
    int y = ball.radius;
    int d = 1 - ball.radius; // Decision variable

    while (x <= y)
    {
        // Draw horizontal lines to fill the circle
        draw_line(Surface, x0 - x, y0 + y, x0 + x, y0 + y, ball.color);
        draw_line(Surface, x0 - x, y0 - y, x0 + x, y0 - y, ball.color);
        draw_line(Surface, x0 - y, y0 + x, x0 + y, y0 + x, ball.color);
        draw_line(Surface, x0 - y, y0 - x, x0 + y, y0 - x, ball.color);

        // Midpoint algorithm update
        if (d < 0)
            d += 2 * x + 3;
        else
        {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}


void clear_window_surface(SDL_Surface *Surface)
{
    SDL_FillRect(Surface, NULL, BLACK);
}

void move_padel(Padel *padel)
{
    if ((padel->vely > 0) && (padel->rect.y + padel->rect.h < SCREEN_HEIGHT - 10))
    {
        padel->rect.y += padel->vely;
        padel->vely = 0;
    }
    if ((padel->vely < 0) && (padel->rect.y > 10))
    {
        padel->rect.y += padel->vely;
        padel->vely = 0;
    }
}

void move_ball(Ball *ball)
{
    if (ball->start_movement)
    {
        ball->x += ball->velx;
        ball->y += ball->vely;
    }

}

void ball_window_colision(Ball *ball, Padel *padel1, Padel *padel2)
{
    if (ball->x + ball->radius >= SCREEN_WIDTH)
    {
        ball->x = SCREEN_WIDTH / 2;
        ball->y = SCREEN_HEIGHT / 2;
        ball->start_movement = 0;
        ball->velx = -BALL_SPEED;
        padel1->points++;
    }
    if (ball->x - ball->radius <= 0)
    {
        ball->x = SCREEN_WIDTH / 2;
        ball->y = SCREEN_HEIGHT / 2;
        ball->start_movement = 0;
        ball->velx = BALL_SPEED;
        padel2->points++;
    }
    if (ball->y + ball->radius >= SCREEN_HEIGHT)
    {
        ball->vely = -BALL_SPEED;
    }
    if (ball->y - ball->radius <= 0)
    {
        ball->vely = BALL_SPEED;
    }
}

// void display_score(SDL_Surface *Surface, )

void ball_padel_colision(Ball *ball, Padel *padel1, Padel *padel2)
{

    if (ball->velx > 0 && ((ball->y + ball->radius > padel2->rect.y) && 
       (ball->y - ball->radius < padel2->rect.y + padel2->rect.h)))
    {
        if (ball->x + ball->radius + 1 >= padel2->rect.x)
        {
            ball->velx = -BALL_SPEED;
        }
    }
    if (ball->velx < 0 && ((ball->y + ball->radius > padel1->rect.y) && 
       (ball->y - ball->radius - 1 <= padel1->rect.y + padel1->rect.h)))
    {
        if (ball->x - ball->radius <= padel1->rect.x + padel1->rect.w)
        {
            ball->velx = BALL_SPEED;
        }
    }
}

int main(int argc, char *argv[])
{
    SDL_Window *Window = NULL;
    SDL_Surface *WindowSurface = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL failed to initialize. SDL Error: %s\n", SDL_GetError());
        return 1;
    }
    
    Window = SDL_CreateWindow("Pong", 
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (Window == NULL)
    {
        printf("Failed to create window. SDL Error: %s\n", SDL_GetError());
        return 1;
    }

    
    WindowSurface = SDL_GetWindowSurface(Window);
    SDL_FillRect(WindowSurface, NULL, BLACK);

    //Creating padels
    Padel padel1 = {{30, 30, 15, 75}, 0};
    Padel padel2 = {{(SCREEN_WIDTH - 45), 350, 15, 75}, 0};

    //Create ball
    Ball ball = {(SCREEN_WIDTH/2), (SCREEN_HEIGHT/2), 6, BALL_SPEED, BALL_SPEED, WHITE};
    ball.start_movement = 0;

    global_running = 1;
    SDL_Event event;

    while(global_running)
    {
        
        //Handle events
        SDL_PollEvent(&event);
        switch (event.type)
        {
            case SDL_QUIT:
            {
                global_running = 0;
            }
        }
        //Handle keyboard input
        const uint8_t *keystates = SDL_GetKeyboardState(NULL);

        if (keystates[SDL_SCANCODE_W])
        {
            padel1.vely = -PADEL_SPEED;
        }
        if (keystates[SDL_SCANCODE_S])
        {
            padel1.vely = PADEL_SPEED;
        }
        if (keystates[SDL_SCANCODE_UP])
        {
            padel2.vely = -PADEL_SPEED;
        }
        if (keystates[SDL_SCANCODE_DOWN])
        {
            padel2.vely = PADEL_SPEED;
        }        
        if (keystates[SDL_SCANCODE_RETURN])
        {
            ball.start_movement = 1;
        }

        clear_window_surface(WindowSurface);

        //Padel movement
        move_padel(&padel1);
        move_padel(&padel2);

        //Ball movement
        ball_padel_colision(&ball, &padel1, &padel2);
        ball_window_colision(&ball, &padel1, &padel2);
        move_ball(&ball);

        //TODO: Improve ball colision
        //TODO: Display player points
        //TODO: Dynamic gameplay with changing dificulty

        //Draw Padels
        SDL_FillRect(WindowSurface, &(padel1.rect), RED);
        SDL_FillRect(WindowSurface, &(padel2.rect), WHITE);

        //Draw ball
        draw_ball(WindowSurface, ball);


        SDL_UpdateWindowSurface(Window);
        SDL_Delay(10);
    }
    SDL_DestroyWindow(Window);
}
