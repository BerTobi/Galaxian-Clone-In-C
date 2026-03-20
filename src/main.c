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
    float speed;
} Entity;

int createEntity(Entity** entities, EntityType type, Vector2 position, int size, float speed)
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
    newEntity->speed = speed;
    entities[freeSlot] = newEntity;

    return freeSlot;
}

void shootProjectile(Entity** entities, Entity* shooter)
{
    Entity* projectile = entities[createEntity(entities, PROJECTILE, shooter->position, 5, 10.0f)];
    projectile->movingInDirection = FORWARD;
    return;
}


void Update(Entity** entities, int currentTick)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (entities[i]->type != NONE)
        {
            switch (entities[i]->movingInDirection)
            {
            case FORWARD:
                entities[i]->position.y -= entities[i]->speed;
                break;
            case BACKWARDS:
                entities[i]->position.y += entities[i]->speed;
                break;
            case LEFT:
                entities[i]->position.x -= entities[i]->speed;
                break;
            case RIGHT:
                entities[i]->position.x += entities[i]->speed;
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

    if (currentTick % 400 == 0)
    {
        for (int i = 0; i < MAX_ENTITIES; i++)
        {
            if (entities[i]->type == ENEMY && entities[i]->movingInDirection == LEFT)
            {
                entities[i]->movingInDirection = RIGHT;
            }
            else if (entities[i]->type == ENEMY && entities[i]->movingInDirection == RIGHT)
            {
                entities[i]->movingInDirection = LEFT;
            }
		}
    }

    //if (currentTick % 400 == 0)
    //{
    //    for (int i = 0; i < MAX_ENTITIES; i++)
    //    {
    //        if (entities[i]->type == ENEMY)
    //        {
    //            shootProjectile(entities, entities[i]);
    //        }
    //    }
    //}
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

void HandleInput(Entity** entities, Entity* player)
{
    //if (IsKeyDown(KEY_UP)) player->movingInDirection = FORWARD;
    //else if (IsKeyDown(KEY_DOWN)) player->movingInDirection = BACKWARDS;
    if (IsKeyDown(KEY_LEFT)) player->movingInDirection = LEFT;
    else if (IsKeyDown(KEY_RIGHT)) player->movingInDirection = RIGHT;
    else player->movingInDirection = NONE;
    if (IsKeyPressed(KEY_LEFT_CONTROL)) shootProjectile(entities, player);
}

int main()
{
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    int currentTick = 200;
    double lastTickTime = 0;

    InitWindow(screenWidth, screenHeight, "Galaxian Clone");
    SetTargetFPS(60);

    Entity* entities[MAX_ENTITIES];
    int freeEntitySlot = 0;

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entities[i] = malloc(sizeof(Entity));
        entities[i]->type = NONE;
    }

    Vector2 startingPlayerPosition = { 900, 900 };
    Entity* player = entities[createEntity(entities, PLAYER, startingPlayerPosition, 20, 5.0f)];
    player->movingInDirection = NONE;
    player->facing = FORWARD;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            Vector2 enemyPosition = { 500 + 100 * j, 100 + 100 * i };
            createEntity(entities, ENEMY, enemyPosition, 20, 1.0f);
        }
    }

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (entities[i]->type == ENEMY)
        {
            entities[i]->movingInDirection = LEFT;
			entities[i]->facing = BACKWARDS;
		}
    }

    while (!WindowShouldClose())
    {
        

        HandleInput(entities, player);
        if (GetTime() - lastTickTime >= 0.01)
        {
            lastTickTime = GetTime();
            currentTick++;
            Update(entities, currentTick);
		}
        Draw(entities);
    }

    CloseWindow();
    return 0;
}
