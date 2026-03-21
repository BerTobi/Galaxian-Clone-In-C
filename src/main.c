#include "raylib.h"

#define MAX_ENTITIES 1000
#define NONE -1

int score = 0;
bool playerProjectileActive = false;
const int screenWidth = 896;
const int screenHeight = 1024;

Sound* soundEffects;

typedef enum Direction {
    FORWARD,
    BACKWARDS,
    LEFT,
    RIGHT
} Direction;

typedef enum EntityType {
    PLAYER,
    PLAYER_PROJECTILE,
	ENEMY_PROJECTILE,
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

void initializeGame(Entity** entities, Entity** player)
{
    score = 0;
    playerProjectileActive = false;

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entities[i] = malloc(sizeof(Entity));
        entities[i]->type = NONE;
    }

    Vector2 startingPlayerPosition = { 100, 235 };
    *player = entities[createEntity(entities, PLAYER, startingPlayerPosition, 20, 1.0f)];
    (*player)->movingInDirection = NONE;
    (*player)->facing = FORWARD;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            Vector2 enemyPosition = { 50 + 16 * j, 50 + 16 * i };
            createEntity(entities, ENEMY, enemyPosition, 16, 0.4f);
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
}

void shootProjectile(Entity** entities, Entity* shooter)
{
	EntityType projectileType = NONE;
    if (shooter->type == PLAYER && !playerProjectileActive)
    {
        projectileType = PLAYER_PROJECTILE;
        playerProjectileActive = true;
		PlaySound(soundEffects[3]);
    }
    else if (shooter->type == ENEMY) projectileType = ENEMY_PROJECTILE;
    
    Entity* projectile = entities[createEntity(entities, projectileType, shooter->position, 2, 2.0f)];
    projectile->movingInDirection = shooter->facing;
    return;
}


void Update(Entity** entities, Entity** player, int currentTick)
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

            if (entities[i]->position.x > GetScreenWidth() + 100 || entities[i]->position.x < -100 || entities[i]->position.y > GetScreenHeight() + 100 || entities[i]->position.y < -100)
            {
                if (entities[i]->type == PLAYER_PROJECTILE) playerProjectileActive = false;
                entities[i]->type = NONE;
            }
        }
        if (entities[i]->type == PLAYER_PROJECTILE)
        {
            for (int j = 0; j < MAX_ENTITIES; j++)
            {
                if (entities[j]->type == ENEMY)
                {
                    if (CheckCollisionCircles(entities[j]->position, entities[j]->size / 2.5f, entities[i]->position, entities[i]->size / 2.5f))
                    {
                        entities[i]->type = NONE;
                        entities[j]->type = NONE;
						playerProjectileActive = false;
                        score += 20;
                        PlaySound(soundEffects[7]);
                    }
                }
            }
        }
        if (entities[i]->type == PLAYER)
        {
            for (int j = 0; j < MAX_ENTITIES; j++)
            {
                if (entities[j]->type == ENEMY_PROJECTILE)
                {
                
                    if (CheckCollisionCircles(entities[i]->position, entities[i]->size / 2.4f, entities[j]->position, entities[j]->size / 2.5f))
                    {
                        entities[i]->type = NONE;
                        entities[j]->type = NONE;
                        PlaySound(soundEffects[4]);
                        for (int i = 0; i < MAX_ENTITIES; i++)
                        {
                            free(entities[i]);
                        }
						initializeGame(entities, player);
                        return;
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

    if (currentTick % 400 == 0)
    {
        for (int i = 0; i < MAX_ENTITIES; i++)
        {
            if (entities[i]->type == ENEMY)
            {
                shootProjectile(entities, entities[i]);
            }
        }
    }
}

void Draw(RenderTexture2D target, Entity** entities, Texture2D* spritesheet)
{
    BeginTextureMode(target);
    ClearBackground(BLACK);
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        Rectangle spritePosition = (Rectangle){ entities[i]->position.x - (entities[i]->size / 2), entities[i]->position.y - (entities[i]->size / 2), entities[i]->size, entities[i]->size };
        if (entities[i]->type == PLAYER)
        {
            DrawTexturePro(*spritesheet, (Rectangle) { 1, 70, 16, 16 }, spritePosition, (Vector2) { 0, 0 }, 0.0f, WHITE);
        }
        if (entities[i]->type == PLAYER_PROJECTILE || entities[i]->type == ENEMY_PROJECTILE)
        {
            DrawTexturePro(*spritesheet, (Rectangle) { 200, 97, 1, 2 }, spritePosition, (Vector2) { 0, 0 }, 0.0f, WHITE);
        }
        if (entities[i]->type == ENEMY)
        {
            DrawTexturePro(*spritesheet, (Rectangle) { 1, 34, 16, 16 }, spritePosition, (Vector2) { 0, 0 }, 0.0f, WHITE);
        }
    }
    char sScore[20];
    sprintf(sScore, "SCORE: %i", score);
    DrawText(sScore, 10, 10, 8, WHITE);
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(target.texture,
        (Rectangle) {0, 0, 224, -256},   // source (negative height flips it)
        (Rectangle) {0, 0, screenWidth, screenHeight}, // destination
        (Vector2) {0, 0},
        0.0f,
        WHITE
    );
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
    
    int currentTick = 200;
    double lastTickTime = 0;

    InitWindow(screenWidth, screenHeight, "Galaxian Clone");
    InitAudioDevice();
    SetTargetFPS(60);

    if (IsAudioDeviceReady())
    {
        printf("Audio device initialized successfully\n");
    }
    else
    {
        printf("Failed to initialize audio device\n");
	}

    Texture2D spritesheet = LoadTexture("res/Spritesheet.png");
    if (spritesheet.id == NULL)
    {
        printf("%s\n", GetWorkingDirectory());
        printf("Failed to load spritesheet\n");
    }

	soundEffects = malloc(sizeof(Sound) * 8);
	soundEffects[3] = LoadSound("res/03.Shoot.mp3");
    soundEffects[4] = LoadSound("res/04. Fighter Loss.mp3");
    soundEffects[7] = LoadSound("res/07. Hit Enemy.mp3");

    Entity* entities[MAX_ENTITIES];
    Entity* player = malloc(sizeof(Entity));
    
    initializeGame(entities, &player);

    RenderTexture2D target = LoadRenderTexture(224, 256);

    while (!WindowShouldClose())
    {
        
        HandleInput(entities, player);
        if (GetTime() - lastTickTime >= 0.01)
        {
            lastTickTime = GetTime();
            currentTick++;
            Update(entities, &player, currentTick);
		}
        Draw(target, entities, &spritesheet);
    }

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        free(entities[i]);
    }
    CloseWindow();
    return 0;
}
