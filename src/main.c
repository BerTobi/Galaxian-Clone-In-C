#include "raylib.h"

#define MAX_ENTITIES 1000
#define NONE -1

typedef enum Direction {
    FORWARD,
    BACKWARDS,
    LEFT,
    RIGHT
} Direction;

typedef enum EntityType {
    PLAYER,
    PROJECTILE,
    ENEMY
} EntityType;

typedef struct Entity
{
    EntityType type;
    Vector2 position;
    Direction movingInDirection;
    Direction facing;
    int size;
} Entity;

void Update(Entity** entities)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (entities[i]->type != NONE)
        {
            switch (entities[i]->movingInDirection)
            {
            case FORWARD:
                entities[i]->position.y -= 1;
                break;
            case BACKWARDS:
                entities[i]->position.y += 1;
                break;
            case LEFT:
                entities[i]->position.x -= 1;
                break;
            case RIGHT:
                entities[i]->position.x += 1;
                break;
            }

            if (entities[i]->position.x > GetScreenWidth() + 100 || entities[i]->position.x < -100 || entities[i]->position.y > GetScreenHeight() + 100 || entities[i]->position.y < -100) entities[i]->type = NONE;
        }
        if (entities[i]->type == ENEMY)
        {
            for (int j = 0; j < MAX_ENTITIES; j++)
            {
                if (entities[j]->type == PROJECTILE)
                {
                    if (CheckCollisionCircles(entities[i]->position, entities[i]->size, entities[j]->position, entities[j]->size))
                    {
                        entities[i]->type = NONE;
                        entities[j]->type = NONE;
                    }
                }
            }
        }
    }
}

void Draw(Entity** entities)
{
    BeginDrawing();
    ClearBackground(BLACK);
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (entities[i]->type == PLAYER)
        {
            DrawCircle(entities[i]->position.x, entities[i]->position.y, entities[i]->size, GREEN);
        }
        if (entities[i]->type == PROJECTILE)
        {
            DrawCircle(entities[i]->position.x, entities[i]->position.y, entities[i]->size, RED);
        }
        if (entities[i]->type == ENEMY)
        {
            DrawCircle(entities[i]->position.x, entities[i]->position.y, entities[i]->size, BLUE);
        }
    }
    EndDrawing();
}

int createEntity(Entity** entities, EntityType type, Vector2 position, int size)
{
    int freeSlot = 0;

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (entities[i]->type == NONE)
        {
            freeSlot = i;
            break;
        }
    }

    Entity* newEntity = malloc(sizeof(Entity));
    newEntity->type = type;
    newEntity->position = position;
    newEntity->size = size;
    entities[freeSlot] = newEntity;

    return freeSlot;
}

void shootProjectile(Entity** entities, Entity* shooter)
{
    Entity* projectile = entities[createEntity(entities, PROJECTILE, shooter->position, 5)];
    projectile->movingInDirection = FORWARD;
    return;
}

void HandleInput(Entity** entities, Entity* player)
{
    if (IsKeyDown(KEY_UP)) player->movingInDirection = FORWARD;
    else if (IsKeyDown(KEY_DOWN)) player->movingInDirection = BACKWARDS;
    else if (IsKeyDown(KEY_LEFT)) player->movingInDirection = LEFT;
    else if (IsKeyDown(KEY_RIGHT)) player->movingInDirection = RIGHT;
    else player->movingInDirection = NONE;
    if (IsKeyPressed(KEY_LEFT_CONTROL)) shootProjectile(entities, player);
}

int main()
{
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Hello World");
    SetTargetFPS(240);

    Entity* entities[MAX_ENTITIES];
    int freeEntitySlot = 0;

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entities[i] = malloc(sizeof(Entity));
        entities[i]->type = NONE;
    }

    Vector2 startingPlayerPosition = { 900, 600 };
    Entity* player = entities[createEntity(entities, PLAYER, startingPlayerPosition, 20)];
    player->movingInDirection = NONE;
    player->facing = FORWARD;

    for (int i = 0; i < 10; i++)
    {
        Vector2 enemyPosition = { 300 + 100 * i, 100 };
        createEntity(entities, ENEMY, enemyPosition, 20);
    }

    while (!WindowShouldClose())
    {
        HandleInput(entities, player);
        Update(entities);
        Draw(entities);
    }

    CloseWindow();
    return 0;
}
