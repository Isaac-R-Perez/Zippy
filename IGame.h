#ifndef IGAME_H
#define IGAME_H

#include <stdint.h>
#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <random>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int32 bool32;
typedef float real32;
typedef double real64;

#define local_persist static 
#define global_variable static
#define internal static

#define KILOBYTES(value) (value * 1024LL)
#define MEGABYTES(value) (KILOBYTES(value) * 1024LL)
#define GIGABYTES(value) (MEGABYTES(value) * 1024LL)
#define TERABYTES(value) (GIGABYTES(value) * 1024LL)

#define ARRAY_LENGTH(value) (sizeof(value) / sizeof(value[0]))

#define SAVE_FILE_EMPTY_CONTENT_SIZE 2

#include "IMath.h"


//Flags
#define F_IS_RETICLE				0x1
#define F_IS_PLAYER					0x2
#define F_IS_FONT					0x4
#define F_DRAW_WITH_TRIANGLE_QUAD	0x8 
#define F_DRAW_WITH_RECT			0x10
#define F_TO_BE_DELETED				0x20
#define F_RENDERABLE				0x40
#define F_IS_STRING					0x80
#define F_DRAW_WITH_STRING			0x100
#define F_DRAW_WITH_CIRCLE			0x200
#define F_DRAW_WITH_FILLED_CIRCLE	0x400
#define F_IS_BULLET					0x800
#define F_IS_ENEMY					0x1000
#define F_IS_BOSS					0x2000
#define F_IS_UI_BAR					0x4000
#define F_IS_COLLIDABLE				0x8000
#define F_PLAYER_OWNED				0x10000
#define F_ENEMY_OWNED				0x20000
#define F_DRAW_WITH_FILLED_RECT		0x40000
#define F_BOSS_OWNED				0x80000
#define F_IS_PLAYER_HEALTH			0x100000
#define F_IS_PLAYER_GUN_BAR			0x200000
#define F_IS_PLAYER_DASH_COUNTER	0x400000
#define F_IS_PLAYER_BURST_COUNTER	0x800000
#define F_IS_WAVE_COUNTER			0x1000000
#define F_IS_DEATH_RING				0x2000000
#define F_BULLET_OWNED				0x4000000
#define F_IS_BUTTON					0x8000000
#define F_IS_START_BUTTON			0x10000000
#define F_IS_LOAD_BUTTON			0x20000000
#define F_IS_EXIT_BUTTON			0x40000000
#define F_IS_DEBUG_INFO				0x80000000

#define FONT_TEST_STRING "!\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"

//game constants
#define MAX_ENTITY_COUNT 20000
#define MAX_ENTITY_NAME_CHAR 64
#define MAX_ENTITY_STRING_CHAR 128
#define GAME_STATE_CHAR_BUFFER 4096

#define PLAYER_BASE_FIRERATE 0.175f
#define PLAYER_BASE_SCALE 22.0f
#define PLAYER_BASE_BULLET_RADIUS 5
#define PLAYER_BASE_SPEED 240.0f
#define PLAYER_BASE_DAMAGE 1
#define PLAYER_BASE_HEALTH 4

#define ENEMY_BASE_FIRERATE 1.0f
#define ENEMY_BASE_SCALE_WIDTH 64.0f
#define ENEMY_BASE_SCALE_HEIGHT 64.0f
#define ENEMY_BASE_BULLET_RADIUS 5
#define ENEMY_BASE_SPEED 55.0f
#define ENEMY_MOVE_TIMER 3.25f
#define ENEMY_BASE_BULLET_SPEED_MODIFIER 1.75f
#define ENEMY_BASE_HEALTH 3
#define ENEMY_BASE_BULLET_DAMAGE 1
#define ENEMY_BASE_CONTACT_DAMAGE 1
#define ENEMY_BASE_BULLET_LIFETIME 10.0f



#define BULLET_MOVEMENT_SCALE 2.125f
#define BURST_BULLET_MOVEMENT_SCALE 1.25f
#define BASE_BULLET_LIFETIME 1.185f
#define BASE_PLAYER_BULLET_SPREAD 6.5f
#define BASE_PLAYER_BURST_AMOUNT 1
#define BASE_PLAYER_BURST_COOLDOWN 1.65f
#define PLAYER_TIME_BETWEEN_BURSTS 0.05f
#define BASE_PLAYER_DASH_COOLDOWN 2.0f
#define BASE_PLAYER_DASH_TIME 0.265f
#define PLAYER_DASH_SPEED_BOOST 3.75f
#define PLAYER_BULLET_COLOR 0x15E132
#define PLAYER_DASH_BULLET_LIFETIME_BOOST 5.0f


#define BASE_ENEMY_BULLET_SPREAD 20.0f
#define ENEMY_DOUBLE_SHOT_SPREAD 10.0f
#define ENEMY_TRIPLE_SHOT_SPREAD 15.0f
#define ENEMY_BURST_SHOT_SPREAD 5.5f
#define ENEMY_MEGA_SHOT_SPREAD 2.5f
#define ENEMY_BURST_SHOT_AMOUNT 8
#define ENEMY_BURST_FIRERATE 0.125f
#define ENEMY_BURST_COOLDOWN 2.1f
#define ENEMY_BURST_SHOT_SPEED 1.65f
#define ENEMY_AOE_SHOT_AMOUNT 10
#define ENEMY_AOE_SHOT_FIRERATE 6.4f
#define ENEMY_CIRCLING_SPEED (TO_RADIANS(1.5f))
#define ENEMY_AFRAID_DISTANCE (90000.0f)

#define RETICLE_SCALE 28.0f
#define RETICLE_TRANSPARENCY 175

#define BOSS_HEALTH 3550
#define BOSS_SPEED 225.0f
#define BOSS_FIRERATE 0.0125f
#define BOSS_SCALE 64.0f
#define BOSS_HP_BAR_WIDTH 1000.0f 
#define BOSS_HP_BAR_HEIGHT 18.0f
#define BOSS_HP_BAR_XPOS 960.0f
#define BOSS_HP_BAR_YPOS 1025.0f
#define BOSS_MOVEMENT_COOLDOWN 3.75f
#define BOSS_BURST_FIRE_COOLDOWN 4.65f
#define BOSS_ENEMY_SPAWN_RATE 5.25f
#define BOSS_DANMAKU_ATTACK_COUNT 5
#define BOSS_MAX_DANMAKU_SPEED 50.0f
#define BOSS_DANMAKU_SPEED_SCALING (TO_RADIANS(3.25f))

#define UI_OFFSET_PIXELS 24.0f
#define PLAYER_HEALTH_DIMENSIONS 25
#define PLAYER_WEAPON_BAR_WIDTH 150.0f

#define MAX_ENEMIES_PER_GROUP 25
#define WAVES_BEFORE_BOSS 12
#define MAX_PLAYER_FIRERATE 0.015f
#define ENEMY_MAX_SPEED 75.0f
#define ENEMY_MAX_FIRERATE_MULT 0.85f

#define ENEMY_GROUP_POSITION_VARIANCE 150.0f
#define ENEMY_GROUP_SPAWN_DISTANCE (75000.0f)
#define TIME_BETWEEN_WAVES 3.0f

#define DEATH_RING_LINGER_TIME 0.545f
#define DEATH_RING_TRANSPARENCY 199
#define DEATH_RING_EXPANSION_SPEED 148.0f

#define BUTTON_WIDTH 400.0f
#define BUTTON_HEIGHT 75.0f
#define BUTTON_TEXT_WIDTH 1.0f
#define BUTTON_TEXT_HEIGHT 1.0f
#define START_BUTTON_TEXT_OFFSET 90.0f
#define LOAD_BUTTON_TEXT_OFFSET 60.0f
#define EXIT_BUTTON_TEXT_OFFSET 63.0f
#define GAME_TITLE_SCALE_X 3.0f
#define GAME_TITLE_SCALE_Y 3.0f

#define FULL_OPACITY 255

#define BACKGROUND_MUSIC_VOLUME 0.6f
#define BACKGROUND_MUISC_LOOP_TIME 487.0f
#define ENTITY_DEATH_SOUND_VOLUME 0.44f
#define HIT_SOUND_VOLUME 0.4f

enum Render_Order
{
	RO_Background,
	RO_Bullet,
	RO_Enemy,
	RO_Player,
	RO_UI,
	RO_Debug,
	RO_NONE, //<---- how many buckets for render order bucket sort
};

enum Textures
{
	Enemy_Sprite,
	Boss_Sprite,
	Player_Sprite,
	Font_Sprite,
	Reticle_Sprite,
	Number_Of_Textures,
};

enum Enemy_Behavior
{
	EB_Stationary,
	EB_Random,
	EB_Circling,
	EB_Pursuit,
	EB_Afraid,
};

enum Enemy_Weapon
{
	EW_Single,
	EW_Double,
	EW_Burst,
	EW_Triple,
	EW_Mega,
	EW_AOE,
};

enum Player_Skill
{
	PS_Dash,
	PS_Swap,
};

enum Boss_Phase
{
	BP_Gun,
	BP_Enemy_Spawn,
	BP_Danmaku,
};

enum Game_Menu_State
{
	GMS_Start_Screen,
	GMS_In_Game,
};

enum Collision_Type
{
	CT_INVALID,
	CT_Bullet_Player,
	CT_Bullet_Enemy,
};

enum Game_End_Screen
{
	GES_Game_Ongoing,
	GES_Player_Died,
	GES_Boss_Defeated,
};


enum Game_Sound_Effect
{
	GSE_BGM,
	GSE_Hit_Sound,
	GSE_Entity_Death,
	GSE_NUMBER_OF_SOUND_EFFECTS
};

//game structs

struct game_entity_bitmap
{
	int32 Width;
	int32 Height;
	int32 SizeInBytes;
	uint8* Memory;
};

// highest bit is the string's IsValid flag! (Length doesn't include null terminator)
struct game_string
{
	uint64 Length; 
	uint8* String;
};

struct game_bitmap_buffer
{
	void* Memory;
	int32 Width;
	int32 Height;
	int32 Pitch;
	int32 BytesPerPixel;
};

struct read_file_result
{
	uint32 ContentsSize;
	void* Contents;
};

//vertices must be in CLOCK-WISE ordering to render!!!
struct game_triangle
{
	vec2D Vertices[3];
	vec2D UVCoordinates[3];
	game_entity_bitmap* TextureHandle;
	uint32 Color;
};

struct game_quad
{
	game_triangle Triangles[2];
};

struct game_entity
{
	//Main componenets
	uint64 ID;
	char Name[MAX_ENTITY_NAME_CHAR];
	uint64 Flags;
	mat4x4 Scale;
	mat4x4 Rotation;
	vec3D Position;

	uint32 Color;
	int32 Radius;


	//Texture stuff
	game_entity_bitmap* TextureBitMap;

	int32 RenderOrder;

	char StringMessage[MAX_ENTITY_STRING_CHAR];
	uint8 Transparency;

	//Normalized vector at world origin (add entity position for true spawn location)
	vec3D BulletSpawnPoint;

	vec3D Direction;
	real32 Speed;

	//in seconds
	real32 LifeTime;

	//freelist pointer
	game_entity* NextEntityInFreeList;

	//weapons
	real32 FireRate;
	real32 WeaponCooldown;
	int32 Damage;

	int32 Health;

	bool32 BulletSpread;
	real32 BulletSpreadVariance;

	int32 EnemyBehavior;
	int32 EnemyWeapon;
	int32 BurstShotCount;
	real32 EnemyMovementCooldown;
	bool32 FlippedCircling;
	
	bool32 Dashing;
	real32 DashCooldown;
	real32 DashCooldownOnUse;
	int32 DashCharges;
	int32 MaxDashCharges;
	real32 DashingTimer; //how long the player is in the dashing state
	vec3D FixedDashDirection;

	bool32 CanBurst;
	real32 BurstCooldown;
	real32 BurstCooldownOnUse;
	int32 BurstCharges;
	int32 MaxBurstCharges;
	real32 BurstingTimer; //how long until entity can shoot out a burst again

	bool32 IsSplitBullet;
	real32 SplitTimer;
	real32 InitialTimeToSplit;

	real32 BossGunTimer; //timer for how long the boss can fire
	real32 BossEnemySpawnTimer;
	vec3D BossDanmakuDirection;
	real32 DanmakuRotation;

	real32 DeathRingExpansion; //cast to int for current radius

	bool32 ButtonHoveredOver;

};



struct game_collision_info
{
	game_entity* Entity;
	int32 TopEdge;
	int32 BottomEdge;
	int32 LeftEdge;
	int32 RightEdge;
	int32 Active;
};

struct game_collision_pair
{
	game_collision_info* Left;
	game_collision_info* Right;
	int32 CollisionType;
};

struct game_enemy_draw_info
{
	int32 X; //XPos % BitmapBuffer->Width
	int32 Y; // TopY
	uint8 Transparency; //determined on a per enemy basis
};

struct game_state
{
	uint64 EntityIDCounter;
	int32 LastMouseX;
	int32 LastMouseY;

	game_entity* Player;

	game_entity* DebugInfo;

	game_entity* Boss;

	game_entity* Reticle;

	game_entity* StartButton;

	game_entity* LoadButton;

	game_entity* ExitButton;

	game_entity* ScreenText; //PAUSED, YOU LOSE, YOU WIN...

	char CharacterBuffer[GAME_STATE_CHAR_BUFFER];


	void** SortedEntities;
	int32 EntityCount;
	int32 FreeListCount;

	game_entity_bitmap TextureHandles[Number_Of_Textures];
	bool32 EnemiesDrawn; //false only on the first enemy in the game, set to true after batch rendering all enemies

	std::mt19937 PRNG;


	int32 CurrentWave;
	real32 EnemyFireRateMult;
	real32 EnemySpeed;
	real32 EnemyCountPerWave;
	real32 WaveSpawnTimer;

	real32 PlayerFireRateIncreasePerWave;
	real32 EnemyFireRateIncreasePerWave;
	real32 EnemySpeedUpPerWave;

	int32 GameMenuState; //start screen, in-game, paused
	bool32 FightingBoss; //true if the boss has been spawned until it is defeated

	bool32 GamePaused; //only possible to be true during waves
	bool32 GameShouldClose;
	int32 GameEndType;

};

struct game_audio_info
{
	char FileName[256];
	int32 Channels;
	int32 SamplesPerSecond;
	int32 BitsPerSample;
	real32 Volume;
	
	int32 ChunkSize;
	void* AudioData;
};


struct game_arena
{
	void* CurrentAddress;
	void* Memory;
	void* FreeList;
	void* LastInFreeList;
};

struct game_memory
{
	bool32 IsInitialized;

	void* PermanentStorage;
	uint64 PermanentStorageSize;

	void* BitMapArenaStorage;
	uint64 BitMapArenaStorageSize;

	void* EntityArenaStorage;
	uint64 EntityArenaStorageSize;

	void* ScratchArenaStorage;
	uint64 ScratchArenaStorageSize;

	void* AudioDataArenaStorage;
	uint64 AudioDataArenaStorageSize;

	game_arena BitMapArena;

	game_arena EntityArena;

	//cleared at the start of each frame!!!
	game_arena ScratchArena;

	game_arena AudioDataArena;

	//how long the last frame took before SLEEP
	real32 MSForLastFrame;

	//platform function pointers
	bool32 (*PlatformWriteEntireFile)(char* FileName, uint64 MemorySize, void* Memory);
	bool32 (*PlatformWriteToFile)(char* FileName, int32 FileOffsetBytes, uint64 MemorySize, void* Memory);
	void (*PlatformFreeFileMemory)(void* Memory);
	read_file_result (*PlatformReadEntireFile)(char* FileName);
	int32 (*PlatformCompareFileTimes)(char* FileLeft, char* FileRight);
	void (*PlatformFormatAudioBeforePlayback)(game_audio_info* GameAudioInfo, void* (*ArenaAllocate)(game_arena*, uint64), game_arena* Arena);
	void (*PlatformSubmitAudioToPlay)(game_audio_info* GameAudioInfo);

	game_audio_info SoundEffects[GSE_NUMBER_OF_SOUND_EFFECTS];

	bool32 GameRunning;

	real32 BGMLoopTimer;
};


struct game_button
{
	bool32 EndedDown;
	uint32 HalfTransitions;
};


struct game_input
{
	int32 MouseXPosition;
	int32 MouseYPosition;
	game_button MouseButtons[2]; // 0 = Left Mouse Button, 1 = Right Mouse Button

	game_button Up;
	game_button Down;
	game_button Left;
	game_button Right;
	game_button Start;
	game_button Back;
	game_button LeftThumb;
	game_button RightThumb;
	game_button LeftShoulder;
	game_button RightShoulder;
	game_button AButton;
	game_button BButton;
	game_button XButton;
	game_button YButton;

	uint8 LeftTrigger;
	uint8 RightTrigger;

	real32 LeftStickX;
	real32 LeftStickY;
	real32 RightStickX;
	real32 RightStickY;

	bool32 ControllerDetected;
};

//platform functions (game wants something from platform)


//game functions
extern "C" void GameUpdateAndRender(game_memory* Memory, game_input* Input, game_bitmap_buffer* BitmapBuffer, real32 DeltaTime);
//extern "C" void GameAllocateToBufferMemory(game_memory* Memory);

//game function stubs
void (*GameUpdateAndRenderSTUB)(game_memory* , game_input* , game_bitmap_buffer*, real32 ) = {};




#endif