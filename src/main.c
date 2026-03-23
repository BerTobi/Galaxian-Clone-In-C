#include <stdlib.h>
#include <stdio.h>
#include "raylib.h"
#include <assert.h>

#define MAX_ENTITIES 1000
#define NONE -1

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

typedef enum EnemyBehaviour {
    IN_FORMATION,
    DIVING,
    RETURNING
} EnemyBehaviour;

typedef struct EnemyFormation
{
    Vector2 center;
    Direction movingInDirection;
} EnemyFormation;

typedef union EntityData
{
    struct { EnemyBehaviour behaviour; Vector2 formationOffset; EnemyFormation* formation; } enemyData;
} EntityData;

typedef struct Entity
{
    EntityType type;
    Vector2 position;
    Direction movingInDirection;
    Direction facing;
    float size;
    float speed;
	EntityData data;
} Entity;

typedef struct GameData
{
    Entity** entities;
    Entity* player;
	EnemyFormation* enemyFormation;
    int score;
    bool playerProjectileActive;
} GameData;

int createEntity(Entity** entities, EntityType type, Vector2 position, float size, float speed)
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
    assert(newEntity != NULL);
    newEntity->type = type;
    newEntity->position = position;
    newEntity->size = size;
    newEntity->speed = speed;
	newEntity->movingInDirection = NONE;
	newEntity->facing = NONE;
    entities[freeSlot] = newEntity;

    return freeSlot;
}

void initializeGame(GameData* gameData)
{
    if (gameData->entities != NULL) free(gameData->entities);
    if (gameData->enemyFormation != NULL) free(gameData->enemyFormation);
	gameData->score = 0;
    gameData->playerProjectileActive = false;
	gameData->player = NULL;
	gameData->entities = malloc(sizeof(Entity*) * MAX_ENTITIES);
    gameData->enemyFormation = malloc(sizeof(EnemyFormation));
    assert(gameData->entities != NULL);
    assert(gameData->enemyFormation != NULL);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        gameData->entities[i] = malloc(sizeof(Entity));
        assert(gameData->entities[i] != NULL);
        gameData->entities[i]->type = NONE;
    }

    Vector2 startingPlayerPosition = { 100, 235 };
    gameData->player = gameData->entities[createEntity(gameData->entities, PLAYER, startingPlayerPosition, 20, 1.0f)];
    gameData->player->movingInDirection = NONE;
    gameData->player->facing = FORWARD;
    
	gameData->enemyFormation->center = (Vector2){ 150, 100 };
    gameData->enemyFormation->movingInDirection = LEFT;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            
            Entity* enemy = gameData->entities[createEntity(gameData->entities, ENEMY, (Vector2) {0, 0}, 16, 0.4f)];
            Vector2 enemyPosition = { -80.0f + 16.0f * j, -50.0f + 16.0f * i };
			enemy->data.enemyData.formationOffset = enemyPosition;
			enemy->data.enemyData.behaviour = IN_FORMATION;
            enemy->position = (Vector2){ gameData->enemyFormation->center.x + enemyPosition.x, gameData->enemyFormation->center.y + enemyPosition.y };
			enemy->movingInDirection = LEFT;
			enemy->facing = BACKWARDS;
        }
    }
}

void shootProjectile(GameData* gameData, Entity* shooter)
{
	EntityType projectileType = NONE;
    if (shooter->type == PLAYER && !gameData->playerProjectileActive)
    {
        projectileType = PLAYER_PROJECTILE;
        gameData->playerProjectileActive = true;
		PlaySound(soundEffects[3]);
    }
    else if (shooter->type == ENEMY) projectileType = ENEMY_PROJECTILE;
    
    Entity* projectile = gameData->entities[createEntity(gameData->entities, projectileType, shooter->position, 2, 2.0f)];
    projectile->movingInDirection = shooter->facing;
    return;
}

void UpdateEntityPosition(GameData* gameData, Entity* entity)
{
    switch (entity->movingInDirection)
    {
    case FORWARD:
        entity->position.y -= entity->speed;
        break;
    case BACKWARDS:
        entity->position.y += entity->speed;
        break;
    case LEFT:
        entity->position.x -= entity->speed;
        break;
    case RIGHT:
        entity->position.x += entity->speed;
        break;
    }

    if (entity->position.x > GetScreenWidth() + 100 || entity->position.x < -100 || entity->position.y > GetScreenHeight() + 100 || entity->position.y < -100)
    {
        if (entity->type == PLAYER_PROJECTILE) gameData->playerProjectileActive = false;
        entity->type = NONE;
    }
}

void UpdateEnemyAI(GameData* gameData, Entity* enemy, int currentTick)
{
    if (enemy->data.enemyData.behaviour == IN_FORMATION)
    {
		enemy->movingInDirection = gameData->enemyFormation->movingInDirection;
	}
    if (currentTick % 400 == 0)
    {
        shootProjectile(gameData, enemy);
    }
}

void UpdateProjectileCollisions(GameData* gameData, Entity* playerProjectile)
{
    for (int j = 0; j < MAX_ENTITIES; j++)
    {
        if (gameData->entities[j]->type == ENEMY)
        {
            if (CheckCollisionCircles(gameData->entities[j]->position, gameData->entities[j]->size / 2.5f, playerProjectile->position, playerProjectile->size / 2.5f))
            {
                playerProjectile->type = NONE;
                gameData->entities[j]->type = NONE;
                gameData->playerProjectileActive = false;
                gameData->score += 20;
                PlaySound(soundEffects[7]);
            }
        }
    }
}

void UpdatePlayerCollisions(GameData* gameData)
{
    for (int j = 0; j < MAX_ENTITIES; j++)
    {
        if (gameData->entities[j]->type == ENEMY_PROJECTILE)
        {

            if (CheckCollisionCircles(gameData->player->position, gameData->player->size / 2.4f, gameData->entities[j]->position, gameData->entities[j]->size / 2.5f))
            {
                gameData->player->type = NONE;
                gameData->entities[j]->type = NONE;
                PlaySound(soundEffects[4]);
                for (int i = 0; i < MAX_ENTITIES; i++)
                {
                    free(gameData->entities[i]);
                }
                initializeGame(gameData);
                return;
            }
        }
    }
}

void Update(GameData* gameData, int currentTick)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (gameData->entities[i]->type != NONE)
        {
			UpdateEntityPosition(gameData, gameData->entities[i]);
        }
        if (gameData->entities[i]->type == ENEMY)
        {
            UpdateEnemyAI(gameData, gameData->entities[i], currentTick);
        }
        if (gameData->entities[i]->type == PLAYER_PROJECTILE)
        {
			UpdateProjectileCollisions(gameData, gameData->entities[i]);
        }
    }
    if (currentTick % 400 == 0)
    {
        if (gameData->enemyFormation->movingInDirection == LEFT)
        {
            gameData->enemyFormation->movingInDirection = RIGHT;
        }
        else if (gameData->enemyFormation->movingInDirection == RIGHT)
        {
            gameData->enemyFormation->movingInDirection = LEFT;
        }
    }
    UpdatePlayerCollisions(gameData);

}

void Draw(RenderTexture2D target, GameData* gameData, Texture2D* spritesheet)
{
    BeginTextureMode(target);
    ClearBackground(BLACK);
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        Rectangle spritePosition = (Rectangle){ gameData->entities[i]->position.x - (gameData->entities[i]->size / 2.0f), gameData->entities[i]->position.y - (gameData->entities[i]->size / 2.0f), gameData->entities[i]->size, gameData->entities[i]->size };
        if (gameData->entities[i]->type == PLAYER)
        {
            DrawTexturePro(*spritesheet, (Rectangle) { 1, 70, 16, 16 }, spritePosition, (Vector2) { 0, 0 }, 0.0f, WHITE);
        }
        if (gameData->entities[i]->type == PLAYER_PROJECTILE || gameData->entities[i]->type == ENEMY_PROJECTILE)
        {
            DrawTexturePro(*spritesheet, (Rectangle) { 200, 97, 1, 2 }, spritePosition, (Vector2) { 0, 0 }, 0.0f, WHITE);
        }
        if (gameData->entities[i]->type == ENEMY)
        {
            DrawTexturePro(*spritesheet, (Rectangle) { 1, 34, 16, 16 }, spritePosition, (Vector2) { 0, 0 }, 0.0f, WHITE);
        }
    }
    char sScore[20];
    sprintf(sScore, "SCORE: %i", gameData->score);
    DrawText(sScore, 10, 10, 8, WHITE);
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(target.texture,
        (Rectangle) {0, 0, 224.0f, -256.0f},   // source (negative height flips it)
        (Rectangle) {0, 0, (float)screenWidth, (float)screenHeight}, // destination
        (Vector2) {0, 0},
        0.0f,
        WHITE
    );
    EndDrawing();
}

void HandleInput(GameData* gameData, Entity* player)
{
    if (IsKeyDown(KEY_LEFT)) player->movingInDirection = LEFT;
    else if (IsKeyDown(KEY_RIGHT)) player->movingInDirection = RIGHT;
    else player->movingInDirection = NONE;
    if (IsKeyPressed(KEY_LEFT_CONTROL)) shootProjectile(gameData, gameData->player);
}

int main()
{
    
    int currentTick = 200;
    double lastTickTime = 0;

    InitWindow(screenWidth, screenHeight, "Galaxian Clone");
    InitAudioDevice();
    DisableCursor();
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
    if (!spritesheet.id)
    {
        printf("%s\n", GetWorkingDirectory());
        printf("Failed to load spritesheet\n");
    }

	soundEffects = malloc(sizeof(Sound) * 8);
    assert(soundEffects != NULL);
	soundEffects[3] = LoadSound("res/03.Shoot.mp3");
    soundEffects[4] = LoadSound("res/04. Fighter Loss.mp3");
    soundEffects[7] = LoadSound("res/07. Hit Enemy.mp3");

	GameData* gameData = calloc(1, sizeof(GameData));
    assert(gameData != NULL);

    initializeGame(gameData);

    RenderTexture2D target = LoadRenderTexture(224, 256);

    while (!WindowShouldClose())
    {
        
        HandleInput(gameData, gameData->player);
        if (GetTime() - lastTickTime >= 0.01)
        {
            lastTickTime = GetTime();
            currentTick++;
            Update(gameData, currentTick);
		}
        Draw(target, gameData, &spritesheet);
    }

    //for (int i = 0; i < MAX_ENTITIES; i++)
    //{
    //    free(entities[i]);
    //}
    CloseWindow();
    return 0;
}
