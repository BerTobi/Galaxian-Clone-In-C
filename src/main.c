#include <stdlib.h>
#include <stdio.h>
#include "raylib.h"
#include <assert.h>
#include <time.h>
#include <math.h>

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
    ENEMY,
    ENEMY2
} EntityType;

typedef enum EnemyBehaviour {
    IN_FORMATION,
    DIVING,
    RETURNING
} EnemyBehaviour;

typedef struct EnemyFormation
{
    Vector2 center;
    Direction movinginDirection;
    int divingEnemies;
} EnemyFormation;

typedef union EntityData
{
    struct {
        EnemyBehaviour behaviour; 
        Vector2 formationOffset; 
        EnemyFormation* formation; 
        float diveAngle;
        int diveStartTick;
        int type;
    } enemyData;
} EntityData;

typedef struct Entity
{
    EntityType type;
    Vector2 position;
    Vector2 velocity;
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
    int playerProjectileCount;
	int maxPlayerProjectiles;
    int powerupTicks;
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
	newEntity->velocity = (Vector2){ 0, 0 };
	newEntity->facing = NONE;
    entities[freeSlot] = newEntity;

    return freeSlot;
}

void initializeGame(GameData* gameData)
{
    if (gameData->entities != NULL) free(gameData->entities);
    if (gameData->enemyFormation != NULL) free(gameData->enemyFormation);
	gameData->score = 0;
    gameData->playerProjectileCount = 0;
    gameData->maxPlayerProjectiles = 1;
    gameData->powerupTicks = 0;
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
	gameData->player->velocity = (Vector2){ 0, 0 };
    gameData->player->facing = FORWARD;
    
	gameData->enemyFormation->center = (Vector2){ 112, 100 };
    gameData->enemyFormation->movinginDirection = LEFT;
	gameData->enemyFormation->divingEnemies = 0;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            
            Entity* enemy = gameData->entities[createEntity(gameData->entities, ENEMY, (Vector2) {0, 0}, 16, 0.1f)];
            Vector2 enemyPosition = { -80.0f + 16.0f * j, -50.0f + 16.0f * i };
			enemy->data.enemyData.formationOffset = enemyPosition;
			enemy->data.enemyData.behaviour = IN_FORMATION;
            enemy->data.enemyData.diveAngle = 0;
            enemy->data.enemyData.type = 0;
            enemy->position = (Vector2){ gameData->enemyFormation->center.x + enemyPosition.x, gameData->enemyFormation->center.y + enemyPosition.y };
			enemy->velocity = (Vector2){ 0, 0 };
			enemy->facing = BACKWARDS;
        }
    }
    for (int i = 0; i < 8; i++)
    {
        Entity* enemy = gameData->entities[createEntity(gameData->entities, ENEMY, (Vector2) { 0, 0 }, 16, 0.1f)];
        Vector2 enemyPosition = { -64.0f + 16.0f * i, -66.0f};
        enemy->data.enemyData.formationOffset = enemyPosition;
        enemy->data.enemyData.behaviour = IN_FORMATION;
        enemy->data.enemyData.diveAngle = 0;
        enemy->data.enemyData.type = 1;
        enemy->position = (Vector2){ gameData->enemyFormation->center.x + enemyPosition.x, gameData->enemyFormation->center.y + enemyPosition.y };
        enemy->velocity = (Vector2){ 0, 0 };
        enemy->facing = BACKWARDS;       
    }
}

void shootProjectile(GameData* gameData, Entity* shooter)
{
	EntityType projectileType = NONE;
    if (shooter->type == PLAYER && gameData->playerProjectileCount < gameData->maxPlayerProjectiles)
    {
        projectileType = PLAYER_PROJECTILE;
        gameData->playerProjectileCount++;
		PlaySound(soundEffects[3]);
    }
    else if (shooter->type == ENEMY) projectileType = ENEMY_PROJECTILE;
    
    Entity* projectile = gameData->entities[createEntity(gameData->entities, projectileType, shooter->position, 2, 2.0f)];
    if (shooter->facing == FORWARD) projectile->velocity = (Vector2){ 0, -projectile->speed };
    else projectile->velocity = (Vector2){ 0, projectile->speed };
	
    return;
}

void UpdateEntityPosition(GameData* gameData, Entity* entity)
{
	entity->position.x = entity->position.x + entity->velocity.x;
    entity->position.y = entity->position.y + entity->velocity.y;

    if (entity->position.x > GetScreenWidth() + 100 || entity->position.x < -100 || entity->position.y > GetScreenHeight() + 100 || entity->position.y < -100)
    {
        if (entity->type == PLAYER_PROJECTILE) gameData->playerProjectileCount--;
        entity->type = NONE;
    }
}

void UpdateEnemyAI(GameData* gameData, Entity* enemy, int currentTick)
{
    if (enemy->data.enemyData.behaviour == IN_FORMATION)
    {
        if (gameData->enemyFormation->movinginDirection == LEFT)
        {
			enemy->velocity.x = -enemy->speed;
        }
        else if (gameData->enemyFormation->movinginDirection == RIGHT)
        {
            enemy->velocity.x = enemy->speed;
        }
	}

    if (enemy->data.enemyData.behaviour == DIVING)
    {
        int shootChance = rand();
        if (enemy->data.enemyData.type == 0)
        {
            enemy->speed = 0.5f;
            float t = (currentTick - enemy->data.enemyData.diveStartTick) * 0.02f;
            enemy->velocity.x = cosf(t) * enemy->speed * 3.0f;
            enemy->velocity.y = sinf(t * 0.5f) * enemy->speed;

            if (shootChance % 10000 < 25)
            {
                shootProjectile(gameData, enemy);
            }
        }
        
        if (enemy->data.enemyData.type == 1)
        {
            enemy->speed = 0.7f;
            float t = (currentTick - enemy->data.enemyData.diveStartTick) * 0.02f;
            enemy->velocity.x = cosf(t) * enemy->speed * 4.0f;
            enemy->velocity.y = sinf(t * 0.75f) * enemy->speed;

            if (shootChance % 10000 < 60)
            {
                shootProjectile(gameData, enemy);
            }
        }

		
        
    }

    int diveChance = rand();

    if (diveChance % 10000 < 2 && gameData->enemyFormation->divingEnemies < 5)
    {
        enemy->data.enemyData.diveStartTick = currentTick;
        enemy->data.enemyData.behaviour = DIVING;
        gameData->enemyFormation->divingEnemies++;
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
                gameData->playerProjectileCount--;
                gameData->score += 20;
                PlaySound(soundEffects[7]);
                if (gameData->entities[j]->data.enemyData.behaviour == DIVING)
                {
                    gameData->enemyFormation->divingEnemies--;
				}
                if (gameData->entities[j]->data.enemyData.type == 1)
                {
                    gameData->powerupTicks += 600;
                }
            }
        }
    }
}

void UpdatePlayerCollisions(GameData* gameData)
{
    for (int j = 0; j < MAX_ENTITIES; j++)
    {
        if (gameData->entities[j]->type == ENEMY_PROJECTILE || gameData->entities[j]->type == ENEMY)
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
	IsSoundPlaying(soundEffects[0]) ? 0 : PlaySound(soundEffects[0]);
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

    if (gameData->enemyFormation->center.x < 90)
    {
        gameData->enemyFormation->movinginDirection = RIGHT;
    }
    else if (gameData->enemyFormation->center.x > 150)
    {
        gameData->enemyFormation->movinginDirection = LEFT;
    }

    if (gameData->enemyFormation->movinginDirection == RIGHT)
    {
        gameData->enemyFormation->center.x += 0.1f;
    }
    else
    {
		gameData->enemyFormation->center.x -= 0.1f;
    }

    UpdatePlayerCollisions(gameData);

    if (gameData->powerupTicks > 0)
    {
        gameData->maxPlayerProjectiles = 2;
        gameData->powerupTicks--;
    }
    else
    {
        gameData->maxPlayerProjectiles = 1;
	}

}

void Draw(RenderTexture2D target, GameData* gameData, Texture2D* spritesheet)
{
    BeginTextureMode(target);
    ClearBackground(BLACK);
    BeginBlendMode(BLEND_ALPHA);
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        Rectangle spritePosition = (Rectangle){(int)(gameData->entities[i]->position.x - (gameData->entities[i]->size / 2.0f)), (int)(gameData->entities[i]->position.y - (gameData->entities[i]->size / 2.0f)),(int)gameData->entities[i]->size, (int)gameData->entities[i]->size };
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
            if (gameData->entities[i]->data.enemyData.type == 0) 
                DrawTexturePro(*spritesheet, (Rectangle) { 1, 34, 16, 16 }, spritePosition, (Vector2) { 0, 0 }, 0.0f, WHITE);
			else if (gameData->entities[i]->data.enemyData.type == 1)
                DrawTexturePro(*spritesheet, (Rectangle) { 1, 17, 16, 16 }, spritePosition, (Vector2) { 0, 0 }, 0.0f, WHITE);
        }

    }
    char sScore[20];
    EndBlendMode();
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
    if (IsKeyDown(KEY_LEFT)) player->velocity = (Vector2){ -1, 0 };
    else if (IsKeyDown(KEY_RIGHT)) player->velocity = (Vector2){ 1, 0 };
    else player->velocity = (Vector2){ 0,0 };
    if (IsKeyPressed(KEY_LEFT_CONTROL)) shootProjectile(gameData, gameData->player);
}

void cleanup(GameData* gameData)
{
    for (int i = 0; i < 8; i++)
    {
        UnloadSound(soundEffects[i]);
    }
	free(soundEffects);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        free(gameData->entities[i]);
	}
    if (gameData->entities != NULL) free(gameData->entities);
    if (gameData->enemyFormation != NULL) free(gameData->enemyFormation);
}

int main()
{
    srand((unsigned int) time(NULL));

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

    Image spritesheetImage = LoadImage("res/Spritesheet.png");
    ImageFormat(&spritesheetImage, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8); // add alpha channel
    ImageColorReplace(&spritesheetImage, BLACK, (Color) { 0, 0, 0, 0 });
    Texture2D spritesheet = LoadTextureFromImage(spritesheetImage);
    UnloadImage(spritesheetImage);
    SetTextureFilter(spritesheet, TEXTURE_FILTER_POINT);
    if (!spritesheet.id)
    {
        printf("%s\n", GetWorkingDirectory());
        printf("Failed to load spritesheet\n");
    }

	soundEffects = malloc(sizeof(Sound) * 8);
    assert(soundEffects != NULL);
    soundEffects[0] = LoadSound("res/Battle Theme.mp3");
    SetSoundVolume(soundEffects[0], 0.5f);
	soundEffects[3] = LoadSound("res/03.Shoot.mp3");
    soundEffects[4] = LoadSound("res/04. Fighter Loss.mp3");
    soundEffects[7] = LoadSound("res/07. Hit Enemy.mp3");

	GameData* gameData = calloc(1, sizeof(GameData));
    assert(gameData != NULL);

    initializeGame(gameData);

    RenderTexture2D target = LoadRenderTexture(224, 256);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

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

    cleanup(gameData);

    CloseWindow();
    return 0;
}
