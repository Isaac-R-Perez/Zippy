#include "IGame.h"

#define CLEARED_BITMAP_COLOR ((uint32)(0xFF << 24))

inline int32 FindMax(int32 A, int32 B, int32 C)
{
	int32 Result{};
	Result = (A > B) ? A : B;
	Result = (Result > C) ? Result : C;
	return Result;
}

inline int32 FindMin(int32 A, int32 B, int32 C)
{
	int32 Result{};
	Result = (A < B) ? A : B;
	Result = (Result < C) ? Result : C;
	return Result;
}

inline float FindMax(float A, float B, float C)
{
	float Result{};
	Result = (A > B) ? A : B;
	Result = (Result > C) ? Result : C;
	return Result;
}

inline float FindMin(float A, float B, float C)
{
	float Result{};
	Result = (A < B) ? A : B;
	Result = (Result < C) ? Result : C;
	return Result;
}

inline uint32 uint32ColorfromRGB(uint32 R, uint32 G, uint32 B)
{
	return (0x00FFFFFF & ((R << 16) + (G << 8) + B));
}

/*
inline bool32 CheckEntityFlags(game_entity* Entity, uint64 Flags)
{
	return (Entity->Flags & Flags);
}

inline void DisableEntityFlags(game_entity* Entity, uint64 Flags)
{
	Entity->Flags = Entity->Flags & ~Flags;
}
*/
internal void StringCopy(char* Dest, int32 DestCount, char* Src)
{
	int32 CharCounter = 0;
	while (*Src)
	{
		*Dest++ = *Src++;
		CharCounter++;
		if (CharCounter == DestCount)
		{
			break;
		}
	}
}

internal uint32 SampleTexture(game_entity_bitmap* Texture, uint32 ByteOffset)
{
	uint8* ColorByte = (Texture->Memory + ByteOffset);

	uint32 Color = (*(uint32*)ColorByte) & 0x00FFFFFF;

	/*
		Discards pure white
	*/
	if (Color != 0x00FFFFFF)
	{
		return Color;
	}
	else
	{
		return 0;
	}

}

internal void DrawHorizontalLine(game_bitmap_buffer* BitmapBuffer, int32 XStart, int32 XEnd, int32 Y, uint32 Color, uint32 Transparency)
{
	if (XStart >= 0 && Y >= 0 && Y < BitmapBuffer->Height && XEnd < BitmapBuffer->Width)
	{
		if (Transparency == 255)
		{
			uint32* Pixel = (uint32*)BitmapBuffer->Memory + ((Y * BitmapBuffer->Width) + XStart);
			while (XStart < XEnd)
			{
				*Pixel++ = Color;
				XStart++;
			}
		}
		else
		{
			uint32* Pixel = (uint32*)BitmapBuffer->Memory + ((Y * BitmapBuffer->Width) + XStart);
			real32 T = (real32)Transparency / 255;
			while (XStart < XEnd)
			{
				uint32 PixelA = *Pixel;

				uint32 PixelB = Color;

				uint8 ABlue = (uint8)(PixelA & 0x000000FF);
				uint8 AGreen = (uint8)(PixelA >> 8);
				uint8 ARed = (uint8)(PixelA >> 16);

				uint8 BBlue = (uint8)(PixelB & 0x000000FF);
				uint8 BGreen = (uint8)(PixelB >> 8);
				uint8 BRed = (uint8)(PixelB >> 16);

				uint8 Blue = ABlue + (uint8)((real32)(BBlue - ABlue) * T);
				uint8 Green = AGreen + (uint8)((real32)(BGreen - AGreen) * T);
				uint8 Red = ARed + (uint8)((real32)(BRed - ARed) * T);

				uint32 PixelToDraw = uint32ColorfromRGB(Red, Green, Blue);

				*Pixel++ = PixelToDraw;

				XStart++;
			}
		}
		
	}
}


//pass in Transparency into highest byte of Color
internal void DrawPixel(game_bitmap_buffer* BitmapBuffer, int32 XPos, int32 YPos, uint32 Color)
{
	if (XPos >= 0 && YPos >= 0 && YPos < BitmapBuffer->Height && XPos < BitmapBuffer->Width)
	{
		uint32 Transparency = (Color >> 24);

		if (Transparency == 255)
		{
			uint32* Pixel = (uint32*)BitmapBuffer->Memory + ((YPos * BitmapBuffer->Width) + XPos);
			*Pixel = Color;
		}
		else
		{
			//New Color is not full opacity, blending required
			uint32* Pixel = (uint32*)BitmapBuffer->Memory + ((YPos * BitmapBuffer->Width) + XPos);

			uint32 PixelA = *Pixel;
			real32 T = (real32)Transparency / 255;

			uint32 PixelB = Color;

			uint8 ABlue = (uint8)(PixelA & 0x000000FF);
			uint8 AGreen = (uint8)(PixelA >> 8);
			uint8 ARed = (uint8)(PixelA >> 16);

			uint8 BBlue = (uint8)(PixelB & 0x000000FF);
			uint8 BGreen = (uint8)(PixelB >> 8);
			uint8 BRed = (uint8)(PixelB >> 16);

			uint8 Blue = ABlue + (uint8)((real32)(BBlue - ABlue) * T);
			uint8 Green = AGreen + (uint8)((real32)(BGreen - AGreen) * T);
			uint8 Red = ARed + (uint8)((real32)(BRed - ARed) * T);

			uint32 PixelToDraw = uint32ColorfromRGB(Red, Green, Blue);

			*Pixel = PixelToDraw;
		}

	}

}



real32 EdgeCrossProduct(vec2D* A, vec2D* B, vec2D* P)
{
	vec2D AB { *B - *A };
	vec2D AP { *P - *A };
	return ((AB.x * AP.y) - (AB.y * AP.x));
}


internal void DrawTriangle(game_bitmap_buffer* BitMapBuffer, game_triangle* Triangle, uint32 Transparency)
{
	//bounding box with candidate pixels
	int32 XMin = (int32)FindMin(Triangle->Vertices[0].x, Triangle->Vertices[1].x, Triangle->Vertices[2].x);
	int32 YMin = (int32)FindMin(Triangle->Vertices[0].y, Triangle->Vertices[1].y, Triangle->Vertices[2].y);
	int32 XMax = (int32)FindMax(Triangle->Vertices[0].x, Triangle->Vertices[1].x, Triangle->Vertices[2].x);
	int32 YMax = (int32)FindMax(Triangle->Vertices[0].y, Triangle->Vertices[1].y, Triangle->Vertices[2].y);

	float W0ColDelta = (Triangle->Vertices[1].y - Triangle->Vertices[2].y);
	float W1ColDelta = (Triangle->Vertices[2].y - Triangle->Vertices[0].y);
	float W2ColDelta = (Triangle->Vertices[0].y - Triangle->Vertices[1].y);

	float W0RowDelta = (Triangle->Vertices[2].x - Triangle->Vertices[1].x);
	float W1RowDelta = (Triangle->Vertices[0].x - Triangle->Vertices[2].x);
	float W2RowDelta = (Triangle->Vertices[1].x - Triangle->Vertices[0].x);

	vec2D Point0 { (float)XMin, (float)YMin };
	float W0Row = EdgeCrossProduct(&Triangle->Vertices[1], &Triangle->Vertices[2], &Point0);
	float W1Row = EdgeCrossProduct(&Triangle->Vertices[2], &Triangle->Vertices[0], &Point0);
	float W2Row = EdgeCrossProduct(&Triangle->Vertices[0], &Triangle->Vertices[1], &Point0);



	float InvArea = 1.0f / EdgeCrossProduct(&Triangle->Vertices[0], &Triangle->Vertices[1], &Triangle->Vertices[2]);

	//loop all candidate pixels in the bounding box
	for (int32 y = YMin; y <= YMax; y++)
	{
		float W0 = W0Row;
		float W1 = W1Row;
		float W2 = W2Row;

		for (int32 x = XMin; x <= XMax; x++)
		{
			if (x >= 0 && y >= 0)
			{

				bool32 IsInsideTriangle = (W0 >= 0.0f) && (W1 >= 0.0f) && (W2 >= 0.0f);

				if (IsInsideTriangle)
				{
					float Alpha = W0 * InvArea;
					float Beta = W1 * InvArea;
					float Gamma = W2 * InvArea;

					//interpolate is (alpha)*V0 + (beta)*V1 + (gamma)*V2
					vec2D ScaledUV0 = Alpha * Triangle->UVCoordinates[0];
					vec2D ScaledUV1 = Beta * Triangle->UVCoordinates[1];
					vec2D ScaledUV2 = Gamma * Triangle->UVCoordinates[2];

					vec2D SampleUV = ScaledUV0 + ScaledUV1 + ScaledUV2;

					int32 ScaledWidth = (int32)(SampleUV.x * ((real32)Triangle->TextureHandle->Width));
					int32 ScaledHeight = (int32)(SampleUV.y * ((real32)Triangle->TextureHandle->Height));

					int32 ByteOffset = (ScaledWidth + (ScaledHeight * Triangle->TextureHandle->Width)) * 3;

					uint32 Color = SampleTexture(Triangle->TextureHandle, ByteOffset);
					//draw pixel if pixel is valid
					if (Color != 0)
					{
						Color = (Transparency << 24) | Color;
						DrawPixel(BitMapBuffer, x, y, Color);
					}
				}
			}

			W0 += W0ColDelta;
			W1 += W1ColDelta;
			W2 += W2ColDelta;

		}

		W0Row += W0RowDelta;
		W1Row += W1RowDelta;
		W2Row += W2RowDelta;


	}


}

internal void* MemoryAllocate(game_arena* Arena, uint64 BytesToAllocate)
{
	void* Result{};

	Result = Arena->CurrentAddress;

	uint8* NewArenaAddress = ((uint8*)Arena->CurrentAddress) + BytesToAllocate;

	Arena->CurrentAddress = (void*)NewArenaAddress;

	return Result;
}



internal void FillColor(game_bitmap_buffer* BitmapBuffer, uint32 Color)
{
	//BB GG RR 00
	uint32* Pixel = (uint32*)BitmapBuffer->Memory;
	uint32* EndOfBitmap = Pixel + (BitmapBuffer->Height * BitmapBuffer->Width);

	while (Pixel != EndOfBitmap)
	{
		*Pixel++ = Color;
	}
}

internal void DrawRectangle(game_bitmap_buffer* BitmapBuffer, uint32 XPos, uint32 YPos, int32 Width, int32 Height, game_entity_bitmap* Texture, uint32 Transparency)
{
	if (Width < 1)
	{
		Width = 1;
	}
	if (Height < 1)
	{
		Height = 1;
	}

	int32 X = XPos % BitmapBuffer->Width;

	int32 LeftX = X - (Width / 2);

	int32 RightX = X + (Width / 2);

	int32 TopY = YPos - (Height / 2);

	int32 BottomY = YPos + (Height / 2);

	real32 U = 0.0f; //left-most texture coord
	real32 V = 1.0f; //top-most texture coord initially
	real32 DeltaU = 1.0f / ((real32)Width);
	real32 DeltaV = 1.0f / ((real32)Height);

	for (; TopY <= BottomY; TopY++)
	{
		for (int32 CurrentX = LeftX; CurrentX <= RightX; CurrentX++)
		{
			int32 ScaledWidth = (int32)(U * (real32)Texture->Width);
			int32 ScaledHeight = (int32)(V * (real32)Texture->Height);

			int32 ByteOffset = (ScaledWidth + (ScaledHeight * Texture->Width)) * 3;

			uint32 Color = SampleTexture(Texture, ByteOffset);
			//draw pixel if pixel is valid
			if (Color != 0)
			{
				Color = (Transparency << 24) | Color;
				DrawPixel(BitmapBuffer, CurrentX, TopY, Color);
			}

			U += DeltaU;
		}

		U = 0.0f;
		V -= DeltaV;
	}




}

/*
			
*/

internal void DrawBitMap(game_bitmap_buffer* BitmapBuffer, uint32 XPos, uint32 YPos, int32 Width, int32 Height, game_entity_bitmap* EntityBitMap)
{
	
	int32 LeftX = XPos - (Width / 2);
	int32 RightX = XPos + (Width / 2);
	int32 BottomY = YPos + (Height / 2);
	int32 TopY = YPos - (Height / 2);



	//it's scuffed but I can fix it up later, make images able to scale with texture coordinates!
	//and make it to where I can draw the image without crashing, with the correct sample too


	uint8* ColorByte = EntityBitMap->Memory;

	for (int32 CurrentY = BottomY; CurrentY > TopY; CurrentY--)
	{

		for (int32 ScanLineX = LeftX; ScanLineX < RightX; ScanLineX++)
		{
			uint32* Pixel = (uint32*)BitmapBuffer->Memory;
			Pixel += (CurrentY * BitmapBuffer->Width);
			Pixel += ScanLineX;

			uint32 Blue = *ColorByte++;

			uint32 Green = *ColorByte++;

			uint32 Red = *ColorByte++;

			uint32 Color = uint32ColorfromRGB(Red, Green, Blue);
			
			/*
				REMOVES PURE WHITE


			*/
			if (Color != 0x00FFFFFF)
			{
				*Pixel++ = Color;
			}
			else
			{
				Pixel++;
			}


		}
	}


	/*
	
	for (; BottomY >= TopY; BottomY--)
	{
		uint32* Pixel = (uint32*)BitmapBuffer->Memory;
		Pixel = Pixel + (BottomY * BitmapBuffer->Width); // goes to the corret row

		int32 CurrentX = LeftX;
		Pixel = Pixel + CurrentX;

		for (; CurrentX <= RightX; CurrentX++)
		{
			uint32 Blue = *ColorByte++;
			uint32 Green = *ColorByte++;
			uint32 Red = *ColorByte++;

			*Pixel++ = uint32ColorfromRGB(Red, Green, Blue);

		}

	}
	*/





}



internal void DrawFilledRectangle(game_bitmap_buffer* BitmapBuffer, uint32 XPos, uint32 YPos, int32 Width, int32 Height, uint32 Color)
{
	if (Width < 1)
	{
		Width = 1;
	}
	if (Height < 1)
	{
		Height = 1;
	}

	int32 X = XPos % BitmapBuffer->Width;

	int32 LeftX = X - (Width / 2);

	int32 RightX = X + (Width / 2);

	int32 TopY = YPos - (Height / 2);

	int32 BottomY = YPos + (Height / 2);

	for (; TopY <= BottomY; TopY++)
	{
		uint32* Pixel = (uint32*)BitmapBuffer->Memory;
		Pixel = Pixel + (TopY * BitmapBuffer->Width); // goes to the corret row

		int32 CurrentX = LeftX;
		Pixel = Pixel + CurrentX;

		for (; CurrentX <= RightX; CurrentX++)
		{
			*Pixel++ = Color;

		}

	}


}

internal void UpdateCursor(int32 X, int32 Y, game_entity* Cursor)
{
	Cursor->Position.x = (float)X;
	Cursor->Position.y = (float)Y;

}


internal void PlaceCursor(game_bitmap_buffer* BitmapBuffer, game_entity* Cursor)
{
	//DrawRectangle(BitmapBuffer, (uint32)Cursor->Position.x, (uint32)Cursor->Position.y, Cursor->Radius, Cursor->Radius, Cursor->Color);
}

internal void ClearBitmap(game_bitmap_buffer* BitmapBuffer)
{
	uint32* Pixel = (uint32*)BitmapBuffer->Memory;
	uint32* EndOfBuffer = Pixel + (BitmapBuffer->Height * BitmapBuffer->Width);

	while (Pixel != EndOfBuffer)
	{
		*Pixel++ = 0;
	}

}

//bitmaps MUST BE THE SAME SIZE
internal void CopyBitmap(game_bitmap_buffer* Destination, game_bitmap_buffer* Source)
{
	uint32* DestPixel = (uint32*)Destination->Memory;
	uint32* SrcPixel = (uint32*)Source->Memory;

	uint32* EndOfBuffer = DestPixel + (Destination->Height * Destination->Width);

	while (DestPixel != EndOfBuffer)
	{
		if (*SrcPixel != 0)
		{
			*DestPixel++ = *SrcPixel++;

		}
		else
		{
			DestPixel++;
			SrcPixel++;
		}
	}

}

internal bool32 MouseMoved(int32 MouseX, int32 MouseY, game_state* GameState)
{
	bool32 Result{};

	if (GameState->LastMouseX != MouseX && GameState->LastMouseY != MouseY)
	{
		Result = 1;
	}

	return Result;
}




internal void LoadBMP(game_memory* Memory, game_arena* Arena, char* FileName, game_entity_bitmap* BitMap)
{
	// I need to load a file from a filename.
	// Then I need to pull out the data from it:
	/*
		Offsets are from start of file in bytes.

		-DataOffset, 10 bytes in, indicates the beginning of the bitmap data in the file

		- 0x12 bytes in , Width of bitmap in pixels (32-bits)

		- 0x16 bytes in, Height of bitmap in pixel (32-bits)
		
		- 0x1C bytes in, BitsPerPixel (should be 24 for uncompressed raw pixel data)

		-0x22 bytes in, ImageSize in bytes

		The bitmap contains scanlines of pixels in triplets. (BGR) they are arranged backwards in the data.
		The data starts at the bottom scanline and goes up each row until the top is reached.

	*/
	read_file_result BMPFile = Memory->PlatformReadEntireFile(FileName);
	
	void* StartOfFile = BMPFile.Contents;

	uint8* FilePosition = (uint8*)StartOfFile;

	FilePosition += 10; //go to DataOffset

	uint32 StartOfPixelData = *((uint32*)FilePosition);

	FilePosition += 8; // go to Width

	BitMap->Width = *((uint32*)FilePosition);

	FilePosition += 4; // go to Height;

	BitMap->Height = *((uint32*)FilePosition);

	FilePosition += 6; //go to BitsPerPixel (should be 24)

	uint16 BitsPerPixel = *((uint16*)FilePosition);

	int32 PaddingBytes = BitMap->Width % 4;

	BitMap->SizeInBytes = BitMap->Width * BitMap->Height * 3;

	BitMap->Memory = (uint8*)MemoryAllocate(Arena, BitMap->SizeInBytes);

	//kind of a scuffed fix but it works
	int32 PADDING_BETWEEN_BITMAPS = 100;
	MemoryAllocate(Arena, PADDING_BETWEEN_BITMAPS);

	uint8* FirstPixel = ((uint8*)StartOfFile) + StartOfPixelData;
	uint8* CurrentPixel = FirstPixel;
	
	uint8* EndOfData = CurrentPixel + (((BitMap->Width * 3) + PaddingBytes) * (BitMap->Height));

	uint8* CurrentPixelInEntityBitMap = (uint8*)BitMap->Memory;

	int X = 0;
	while (CurrentPixel != EndOfData)
	{
		

		//B
		*CurrentPixelInEntityBitMap++ = *CurrentPixel++;
		//G
		*CurrentPixelInEntityBitMap++ = *CurrentPixel++;
		//R
		*CurrentPixelInEntityBitMap++ = *CurrentPixel++;
		
		X++;

		if (X == BitMap->Width)
		{
			X = 0;

			//discard padding bytes if necessary (up to 3)
			if (PaddingBytes)
			{
				for (int i = 0; i < PaddingBytes; i++)
				{
					CurrentPixel++;
				}
			}
		}

	}


	Memory->PlatformFreeFileMemory(BMPFile.Contents);
}

inline void SetEntityName(game_entity* Entity, char* Name)
{
	StringCopy(Entity->Name, MAX_ENTITY_NAME_CHAR, Name);
}

internal game_entity* AddEntity(game_memory* Memory, game_state* GameState, uint64 Flags)
{
	//Entity Creation Steps

	//check if the arena's freelist is populated, if it is, travese to the end of the list
	game_entity* FreeListEntity = (game_entity*)Memory->EntityArena.FreeList;

	if (FreeListEntity)
	{

		if (Memory->EntityArena.FreeList == Memory->EntityArena.LastInFreeList)
		{
			//head node IS the last node, clear the free list and use this slot for the new entity
			Memory->EntityArena.FreeList = 0;
			Memory->EntityArena.LastInFreeList = 0;
		}
		else
		{
			//head node is not the last, move the head node to point at the next node in the chain, use the head node for the new entity
			Memory->EntityArena.FreeList = FreeListEntity->NextEntityInFreeList;
		}
	}

	//if FreeListEntity is a valid address, then this is the entity address this new entity will overide
	game_entity* Entity = 0;

	if (FreeListEntity)
	{
		Entity = FreeListEntity;
		Entity->NextEntityInFreeList = 0;
	}
	else
	{
		//no freelist entity available, allocate a new slot
		Entity = (game_entity*)MemoryAllocate(&Memory->EntityArena, sizeof(game_entity));
	}


	//give it a unique ID through GameState
	Entity->ID = GameState->EntityIDCounter++;

	//init vectors and matrices
	Entity->Position = { 0.0f, 0.0f, 0.0f };
	Entity->Scale = IdentityMat4X4;
	Entity->Rotation = IdentityMat4X4;
	Entity->Flags = Flags;
	Entity->Transparency = FULL_OPACITY; //default is full opacity at 255
	Entity->IsSplitBullet = false;
	Entity->DeathRingExpansion = 0.0f;
	Entity->ButtonHoveredOver = false;
	Entity->Radius = 0;

	//make sure the string's message is cleared for this new entity
	for (int j = 0; j < MAX_ENTITY_STRING_CHAR; j++)
	{
		Entity->StringMessage[j] = '\0';
	}
	
	if (Entity->Flags & F_IS_RETICLE)
	{
		SetEntityName(Entity, "Reticle");
		Entity->Flags = Entity->Flags | (F_RENDERABLE | F_DRAW_WITH_RECT);
		Entity->RenderOrder = RO_Debug;
		Entity->TextureBitMap = &GameState->TextureHandles[Reticle_Sprite];
		Entity->Scale = ScaleMat4(RETICLE_SCALE, RETICLE_SCALE, 0.0f);
		Entity->Transparency = RETICLE_TRANSPARENCY;
	}

	if (Entity->Flags & F_IS_PLAYER)
	{
		//LoadBMP(Memory, &Memory->BitMapArena, "W:\\projects\\Zippy\\Pixel_Player.bmp", &Entity->TextureBitMap);
		Entity->TextureBitMap = &GameState->TextureHandles[Player_Sprite];
		SetEntityName(Entity, "Player");
		Entity->Flags = Entity->Flags | (F_RENDERABLE | F_DRAW_WITH_TRIANGLE_QUAD | F_IS_COLLIDABLE);
		Entity->RenderOrder = RO_Player;

		Entity->Scale = ScaleMat4(PLAYER_BASE_SCALE, PLAYER_BASE_SCALE, 0.0f);
		Entity->Color = uint32ColorfromRGB(210, 25, 30);
		Entity->BulletSpawnPoint = { 0.0f, -1.0f,0.0f };
		Entity->Direction = { 0.0f,-1.0f,0.0f };
		Entity->Speed = PLAYER_BASE_SPEED;
		Entity->FireRate = PLAYER_BASE_FIRERATE;
		Entity->Health = PLAYER_BASE_HEALTH;
		Entity->Damage = PLAYER_BASE_DAMAGE;
		Entity->BulletSpread = true;
		Entity->BulletSpreadVariance = BASE_PLAYER_BULLET_SPREAD;
		
		Entity->DashCharges = 0;
		Entity->MaxDashCharges = 1;
		Entity->DashCooldown = 0.0f;
		Entity->DashCooldownOnUse = BASE_PLAYER_DASH_COOLDOWN;

		Entity->CanBurst = true;
		Entity->BurstCharges = 0;
		Entity->MaxBurstCharges = 1;
		Entity->BurstCooldown = 0.0f;
		Entity->BurstCooldownOnUse = BASE_PLAYER_BURST_COOLDOWN;
		Entity->BurstShotCount = BASE_PLAYER_BURST_AMOUNT;
	}

	if (Entity->Flags & F_IS_BOSS)
	{
		Entity->TextureBitMap = &GameState->TextureHandles[Boss_Sprite];
		SetEntityName(Entity, "BOSS");
		Entity->Flags = Entity->Flags | (F_RENDERABLE | F_DRAW_WITH_TRIANGLE_QUAD | F_IS_COLLIDABLE);
		Entity->RenderOrder = RO_Enemy;
		Entity->Scale = ScaleMat4(BOSS_SCALE, BOSS_SCALE, 0.0f);
		Entity->Color = uint32ColorfromRGB(10, 25, 235);
		Entity->Direction = { 0.0f, -1.0f, 0.0f };
		Entity->Speed = BOSS_SPEED;
		Entity->FireRate = BOSS_FIRERATE;
		Entity->Health = BOSS_HEALTH;
		Entity->Damage = 1;

		Entity->EnemyBehavior = BP_Gun;
		Entity->BossGunTimer = BOSS_BURST_FIRE_COOLDOWN;
		Entity->BossDanmakuDirection = { 0.0f, -1.0f, 0.0f };
		Entity->BulletSpreadVariance = 1.65f;
	}

	if (Entity->Flags & F_IS_STRING)
	{
		SetEntityName(Entity, "String");
		Entity->Flags = Entity->Flags | (F_RENDERABLE | F_DRAW_WITH_STRING);
		Entity->RenderOrder = RO_Debug;
	}

	if (Entity->Flags & F_IS_BULLET)
	{
		SetEntityName(Entity, "Bullet");
		Entity->Flags = Entity->Flags | (F_RENDERABLE | F_DRAW_WITH_FILLED_CIRCLE | F_IS_COLLIDABLE);
		Entity->RenderOrder = RO_Bullet;
	}

	if (Entity->Flags & F_IS_ENEMY)
	{
		//LoadBMP(Memory, &Memory->BitMapArena, "W:\\projects\\Zippy\\Enemy.bmp", &Entity->TextureBitMap);
		Entity->TextureBitMap = &GameState->TextureHandles[Enemy_Sprite];
		SetEntityName(Entity, "Enemy");
		Entity->Flags |= (F_RENDERABLE | F_DRAW_WITH_RECT | F_IS_COLLIDABLE);
		Entity->RenderOrder = RO_Enemy;
		Entity->FireRate = ENEMY_BASE_FIRERATE;
		Entity->Speed = ENEMY_BASE_SPEED;
		Entity->Scale = ScaleMat4(ENEMY_BASE_SCALE_WIDTH, ENEMY_BASE_SCALE_HEIGHT, 0.0f);
		Entity->Direction = { 0.0f, -1.0f, 0.0f };
		Entity->Health = ENEMY_BASE_HEALTH;
		Entity->Damage = ENEMY_BASE_CONTACT_DAMAGE;
		Entity->BulletSpread = true;
		Entity->BulletSpreadVariance = BASE_ENEMY_BULLET_SPREAD;
	}

	if (Entity->Flags & F_IS_UI_BAR)
	{
		SetEntityName(Entity, "UI_Bar");
		Entity->Flags |= (F_RENDERABLE | F_DRAW_WITH_FILLED_RECT);
		Entity->RenderOrder = RO_UI;
	}

	if (Entity->Flags & F_IS_PLAYER_HEALTH)
	{
		SetEntityName(Entity, "Player_Health");
		Entity->Flags |= (F_RENDERABLE);
		Entity->RenderOrder = RO_UI;
	}

	if (Entity->Flags & F_IS_PLAYER_GUN_BAR)
	{
		SetEntityName(Entity, "Player_Gun_Bar");
		Entity->Flags |= (F_RENDERABLE);
		Entity->RenderOrder = RO_UI;
	}

	if (Entity->Flags & F_IS_PLAYER_DASH_COUNTER)
	{
		SetEntityName(Entity, "Player_Dash_Counter");
		Entity->Flags |= (F_RENDERABLE | F_DRAW_WITH_STRING);
		Entity->RenderOrder = RO_UI;
		Entity->Scale = ScaleMat4(0.42f, 0.42f, 0.0f);
		StringCopy(Entity->StringMessage, 7, "Dash: ");
	}

	if (Entity->Flags & F_IS_PLAYER_BURST_COUNTER)
	{
		SetEntityName(Entity, "Player_Burst_Counter");
		Entity->Flags |= (F_RENDERABLE | F_DRAW_WITH_STRING);
		Entity->RenderOrder = RO_UI;
		Entity->Scale = ScaleMat4(0.42f, 0.42f, 0.0f);
		StringCopy(Entity->StringMessage, 8, "Burst: ");
	}

	if (Entity->Flags & F_IS_WAVE_COUNTER)
	{
		SetEntityName(Entity, "Wave_Counter");
		Entity->Flags |= (F_RENDERABLE | F_DRAW_WITH_STRING);
		Entity->RenderOrder = RO_UI;
		Entity->Scale = ScaleMat4(0.6f, 0.6f, 0.0f);
		StringCopy(Entity->StringMessage, 7, "Wave: ");
	}

	if (Entity->Flags & F_IS_DEATH_RING)
	{
		SetEntityName(Entity, "Death_Ring");
		Entity->Flags |= (F_RENDERABLE | F_DRAW_WITH_CIRCLE);
		Entity->RenderOrder = RO_Background;
		Entity->Transparency = DEATH_RING_TRANSPARENCY;
		Entity->LifeTime = DEATH_RING_LINGER_TIME;
		
		if (Entity->Flags & F_PLAYER_OWNED)
		{
			Entity->Color = PLAYER_BULLET_COLOR;
		}

		if (Entity->Flags & F_ENEMY_OWNED)
		{
			Entity->Color = uint32ColorfromRGB(225, 21, 50);
		}

		if (Entity->Flags & F_BOSS_OWNED)
		{
			Entity->Color = uint32ColorfromRGB(21, 50, 225);
		}

		if (Entity->Flags & F_BULLET_OWNED)
		{
			Entity->Color = uint32ColorfromRGB(255, 255, 255);
		}
	}

	if (Entity->Flags & F_IS_BUTTON)
	{
		real32 ButtonInitialHeight = 500.0f;

		Entity->Flags |= (F_RENDERABLE);
		Entity->RenderOrder = RO_UI;
		Entity->Scale = ScaleMat4(BUTTON_WIDTH, BUTTON_HEIGHT, 0.0f);
		Entity->Color = uint32ColorfromRGB(1, 1, 1);

		if (Entity->Flags & F_IS_START_BUTTON)
		{
			SetEntityName(Entity, "Start_Button");
			Entity->Position = { 960.0f, ButtonInitialHeight,0.0f };
			StringCopy(Entity->StringMessage, 6, "START");
		}
		else if (Entity->Flags & F_IS_LOAD_BUTTON)
		{
			SetEntityName(Entity, "Load_Button");
			Entity->Position = { 960.0f, ButtonInitialHeight + (2.0f * BUTTON_HEIGHT), 0.0f };
			StringCopy(Entity->StringMessage, 5, "LOAD");
		}
		else if (Entity->Flags & F_IS_EXIT_BUTTON)
		{
			SetEntityName(Entity, "Exit_Button");
			StringCopy(Entity->StringMessage, 5, "EXIT");

			if (GameState->GamePaused)
			{
				Entity->Position = { 960.0f, 540.0f, 0.0f };
			}
			else
			{
				Entity->Position = { 960.0f, ButtonInitialHeight + (4.0f * BUTTON_HEIGHT), 0.0f };
			}
		}

	}


	return Entity;
}


internal game_quad CreateRenderQuad(game_entity* Entity)
{
	game_quad Quad{};
	//left triangle
	Quad.Triangles[0].Vertices[0] = {-1.0f,-1.0f};
	Quad.Triangles[0].Vertices[1] = { 1.0f,-1.0f };
	Quad.Triangles[0].Vertices[2] = { -1.0f,1.0f };

	//scale triangle verts to entity scale
	Quad.Triangles[0].Vertices[0] = (Entity->Scale * Quad.Triangles[0].Vertices[0]);
	Quad.Triangles[0].Vertices[1] = (Entity->Scale * Quad.Triangles[0].Vertices[1]);
	Quad.Triangles[0].Vertices[2] = (Entity->Scale * Quad.Triangles[0].Vertices[2]);

	//rotate
	Quad.Triangles[0].Vertices[0] = (Entity->Rotation * Quad.Triangles[0].Vertices[0]);
	Quad.Triangles[0].Vertices[1] = (Entity->Rotation * Quad.Triangles[0].Vertices[1]);
	Quad.Triangles[0].Vertices[2] = (Entity->Rotation * Quad.Triangles[0].Vertices[2]);

	//move
	mat4x4 Translate = TranslateMat4(Entity->Position.x, Entity->Position.y, Entity->Position.z);
	Quad.Triangles[0].Vertices[0] = (Translate * Quad.Triangles[0].Vertices[0]);
	Quad.Triangles[0].Vertices[1] = (Translate * Quad.Triangles[0].Vertices[1]);
	Quad.Triangles[0].Vertices[2] = (Translate * Quad.Triangles[0].Vertices[2]);

	
	Quad.Triangles[0].UVCoordinates[0] = {0.0f,1.0f};
	Quad.Triangles[0].UVCoordinates[1] = {1.0f,1.0f};
	Quad.Triangles[0].UVCoordinates[2] = {0.0f,0.0f};


	//right triangle
	Quad.Triangles[1].Vertices[0] = { 1.0f,-1.0f };
	Quad.Triangles[1].Vertices[1] = { 1.0f, 1.0f };
	Quad.Triangles[1].Vertices[2] = { -1.0f,1.0f };

	//scale triangle verts to entity scale
	Quad.Triangles[1].Vertices[0] = (Entity->Scale * Quad.Triangles[1].Vertices[0]);
	Quad.Triangles[1].Vertices[1] = (Entity->Scale * Quad.Triangles[1].Vertices[1]);
	Quad.Triangles[1].Vertices[2] = (Entity->Scale * Quad.Triangles[1].Vertices[2]);

	//rotate
	Quad.Triangles[1].Vertices[0] = (Entity->Rotation * Quad.Triangles[1].Vertices[0]);
	Quad.Triangles[1].Vertices[1] = (Entity->Rotation * Quad.Triangles[1].Vertices[1]);
	Quad.Triangles[1].Vertices[2] = (Entity->Rotation * Quad.Triangles[1].Vertices[2]);

	//move
	Quad.Triangles[1].Vertices[0] = (Translate * Quad.Triangles[1].Vertices[0]);
	Quad.Triangles[1].Vertices[1] = (Translate * Quad.Triangles[1].Vertices[1]);
	Quad.Triangles[1].Vertices[2] = (Translate * Quad.Triangles[1].Vertices[2]);


	Quad.Triangles[1].UVCoordinates[0] = { 1.0f,1.0f };
	Quad.Triangles[1].UVCoordinates[1] = { 1.0f,0.0f };
	Quad.Triangles[1].UVCoordinates[2] = { 0.0f,0.0f };

	Quad.Triangles[0].TextureHandle = Entity->TextureBitMap;
	Quad.Triangles[1].TextureHandle = Entity->TextureBitMap;

	return Quad;
}

internal void DrawQuad(game_bitmap_buffer* BitmapBuffer, game_entity* Entity)
{
	game_quad Quad = CreateRenderQuad(Entity);


	//draw left triangle
	DrawTriangle(BitmapBuffer, &Quad.Triangles[0], Entity->Transparency);

	//draw right triangle
	DrawTriangle(BitmapBuffer, &Quad.Triangles[1], Entity->Transparency);

}

internal bool32 SampleFont(game_entity_bitmap* Font, real32 UCoordinate, real32 VCoordinate)
{

	//font width and height are 25 pixels... if you want different fonts gotta save that somewhere
	int32 Y = (int32)(VCoordinate * 24.0f) * Font->Width;
	int32 X = (int32)(UCoordinate * 25.0f);

	uint8* ColorByte = Font->Memory + ((Y + X) * 3);

	uint32 Color = (*(uint32*)ColorByte) & 0x00FFFFFF;

	if (Color == 0x0)
	{
		return true;
	}

	return false;
}

internal real32 GetDrawnCharacterSpacing(char Character)
{
	real32 SpacingFactor = 1.0f;
	switch (Character)
	{
	case ' ':
	{
		SpacingFactor = 1.0f;
		break;
	}
	case '\t':
	{
		SpacingFactor = 5.0f;
		break;
	}
	case '!':
	{
		SpacingFactor = 0.6f;
		break;
	}
	case '\"':
	{
		SpacingFactor = 0.85f;
		break;
	}
	case '\'':
	{
		SpacingFactor = 0.55f;
		break;
	}
	case '(':
	{
		SpacingFactor = 0.8f;
		break;
	}
	case ')':
	{
		SpacingFactor = 0.525f;
		break;
	}
	case '*':
	{
		SpacingFactor = 0.7f;
		break;
	}
	case ',':
	{
		SpacingFactor = 0.55f;
		break;
	}
	case '+':
	{
		SpacingFactor = 0.78f;
		break;
	}
	case '-':
	{
		SpacingFactor = 0.78f;
		break;
	}
	case '.':
	{
		SpacingFactor = 0.5f;
		break;
	}
	case '/':
	{
		SpacingFactor = 0.5f;
		break;
	}
	case '0':
	{
		SpacingFactor = 0.7f;
		break;
	}
	case '1':
	{
		SpacingFactor = 0.7f;
		break;
	}
	case '2':
	{
		SpacingFactor = 0.75f;
		break;
	}
	case '3':
	{
		SpacingFactor = 0.7f;
		break;
	}
	case '4':
	{
		SpacingFactor = 0.75f;
		break;
	}
	case '5':
	{
		SpacingFactor = 0.7f;
		break;
	}
	case '6':
	{
		SpacingFactor = 0.7f;
		break;
	}
	case '7':
	{
		SpacingFactor = 0.8f;
		break;
	}
	case '8':
	{
		SpacingFactor = 0.8f;
		break;
	}
	case '9':
	{
		SpacingFactor = 0.65f;
		break;
	}
	case ':':
	{
		SpacingFactor = 0.6f;
		break;
	}
	case ';':
	{
		SpacingFactor = 0.5f;
		break;
	}
	case '<':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case '=':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case '>':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case '?':
	{
		SpacingFactor = 0.8f;
		break;
	}
	case 'A':
	{
		SpacingFactor = 0.95f;
		break;
	}
	case 'B':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'C':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'D':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'E':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'F':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'G':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'H':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'K':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'L':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'N':
	{
		SpacingFactor = 0.85f;
		break;
	}
	case 'O':
	{
		SpacingFactor = 0.8f;
		break;
	}
	case 'P':
	{
		SpacingFactor = 0.8f;
		break;
	}
	case 'Q':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'R':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'S':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'U':
	{
		SpacingFactor = 0.925f;
		break;
	}
	case 'V':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'X':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'Y':
	{
		SpacingFactor = 0.95f;
		break;
	}
	case 'Z':
	{
		SpacingFactor = 0.95f;
		break;
	}
	case '[':
	{
		SpacingFactor = 1.0f;
		break;
	}
	case '\\':
	{
		SpacingFactor = 0.7f;
		break;
	}
	case ']':
	{
		SpacingFactor = 0.8f;
		break;
	}
	case '^':
	{
		SpacingFactor = 0.8f;
		break;
	}
	case '_':
	{
		SpacingFactor = 0.78f;
		break;
	}
	case '`':
	{
		SpacingFactor = 0.65f;
		break;
	}
	case 'a':
	{
		SpacingFactor = 0.85f;
		break;
	}
	case 'b':
	{
		SpacingFactor = 0.75f;
		break;
	}
	case 'c':
	{
		SpacingFactor = 0.7f;
		break;
	}
	case 'd':
	{
		SpacingFactor = 0.85f;
		break;
	}
	case 'e':
	{
		SpacingFactor = 0.75f;
		break;
	}
	case 'f':
	{
		SpacingFactor = 0.85f;
		break;
	}
	case 'g':
	{
		SpacingFactor = 0.715f;
		break;
	}
	case 'h':
	{
		SpacingFactor = 0.75f;
		break;
	}
	case 'i':
	{
		SpacingFactor = 0.6f;
		break;
	}
	case 'j':
	{
		SpacingFactor = 0.575f;
		break;
	}
	case 'k':
	{
		SpacingFactor = 0.75f;
		break;
	}
	case 'l':
	{
		SpacingFactor = 0.6f;
		break;
	}
	case 'm':
	{
		SpacingFactor = 0.825f;
		break;
	}
	case 'n':
	{
		SpacingFactor = 0.725f;
		break;
	}
	case 'o':
	{
		SpacingFactor = 0.65f;
		break;
	}
	case 'p':
	{
		SpacingFactor = 0.75f;
		break;
	}
	case 'q':
	{
		SpacingFactor = 0.75f;
		break;
	}
	case 'r':
	{
		SpacingFactor = 0.775f;
		break;
	}
	case 's':
	{
		SpacingFactor = 0.75f;
		break;
	}
	case 't':
	{
		SpacingFactor = 0.85f;
		break;
	}
	case 'u':
	{
		SpacingFactor = 0.885f;
		break;
	}
	case 'v':
	{
		SpacingFactor = 0.85f;
		break;
	}
	case 'x':
	{
		SpacingFactor = 0.9f;
		break;
	}
	case 'y':
	{
		SpacingFactor = 0.8f;
		break;
	}
	case 'z':
	{
		SpacingFactor = 0.85f;
		break;
	}
	case '{':
	{
		SpacingFactor = 0.75f;
		break;
	}
	case '|':
	{
		SpacingFactor = 0.75f;
		break;
	}
	case '}':
	{
		SpacingFactor = 0.75f;
		break;
	}
	case '~':
	{
		SpacingFactor = 1.0f;
		break;
	}

	default:
	{
		break;
	}
	}

	return SpacingFactor;
}


//Width and Height are in PIXELS
internal void DrawString(game_bitmap_buffer* BitmapBuffer, game_entity_bitmap* Font, real32 X, real32 Y, real32 Width, real32 Height, char* Text, uint32 Color, uint32 Transparency)
{
	/*
			How to Draw String?
			Need X,Y coordinates in screen space, each character drawn will be shifted right and possible shifted down
	*/
	real32 CharacterWidth = 25.0f * Width;
	real32 CharacterHeight = 25.0f * Height;

	real32 ShiftDownFactor = 0.525f;

	int32 Left = (int32)(X - CharacterWidth);
	int32 Right = (int32)(X + CharacterWidth);
	int32 Top = (int32)(Y - CharacterHeight);
	int32 Bottom = (int32)(Y + CharacterHeight);

	bool32 FirstCharacterDrawn{false};

	while (*Text)
	{
		char CharToPrint = *Text;
		//is this a spacing character? if it is, skip it and progress the draw position a uniform slot
		real32 WidthSpaceScaling = GetDrawnCharacterSpacing(CharToPrint);

		bool32 ShiftDown = (CharToPrint == 'g') || (CharToPrint == 'j') || (CharToPrint == 'p') || (CharToPrint == 'q') || (CharToPrint == 'y');

		if (ShiftDown)
		{
			Top += (int32)(CharacterHeight * ShiftDownFactor);
			Bottom += (int32)(CharacterHeight * ShiftDownFactor);
		}

		if (FirstCharacterDrawn)
		{
			//the character advances for ITSELF before being drawn
			Left += (int32)(2.0f * CharacterWidth * WidthSpaceScaling);
			Right += (int32)(2.0f * CharacterWidth * WidthSpaceScaling);
		}
		else
		{
			FirstCharacterDrawn = true;
		}
		bool32 IsSpacingCharacter = (CharToPrint == ' ' || CharToPrint == '\t');

		if (!IsSpacingCharacter)
		{


			//determine if this character needs to be shifted down

			// ! is the first character at 0.0f , ~ is the last at 93.0f
			real32 U{ (real32)*Text - 33.0f };
			real32 V{ 1.0f };

			real32 UDelta = 1.0f / ((float)Right - (float)Left);
			real32 VDelta = 1.0f / ((float)Bottom - (float)Top);

			//should probably have an offset function to get the offset positions for the next character

			for (int32 CurrentX = Left; CurrentX < Right; CurrentX++)
			{
				V = 1.0f;
				for (int32 CurrentY = Top; CurrentY <= Bottom; CurrentY++)
				{

					if (SampleFont(Font, U, V))
					{
						Color = (Transparency << 24) | Color;
						DrawPixel(BitmapBuffer, CurrentX, CurrentY, Color);
					}

					V -= VDelta;
				}

				U += UDelta;
			}

		}

		if (ShiftDown)
		{
			Top -= (int32)(CharacterHeight * ShiftDownFactor);
			Bottom -= (int32)(CharacterHeight * ShiftDownFactor);
		}


		Text++;
	}
	
}



internal void DrawCircle(game_bitmap_buffer* BitmapBuffer, game_arena* ScratchArena, int32 CenterX, int32 CenterY, int32 Radius, uint32 Color, bool32 Filled, uint32 Transparency)
{

	if (Filled)
	{
		Transparency = 255;
	}

	int32 X = 0;
	int32 Y = -Radius;

	int32 P = -4 * Radius + 1;

	while (X < -Y)
	{
		if (P >= 0)
		{
			Y++;
			P += (8 * (X + Y)) + 4;
		}
		else
		{
			P += (8 * X) + 4;
		}

		if (Filled)
		{
			int32 LeftX = CenterX - X;
			int32 RightX = CenterX + X;

			if (LeftX < 0) { LeftX = 0; }
			if (RightX > (BitmapBuffer->Width - 1)) { RightX = (BitmapBuffer->Width - 1); }

			DrawHorizontalLine(BitmapBuffer, LeftX, RightX, CenterY + Y, Color, Transparency);
			DrawHorizontalLine(BitmapBuffer, LeftX, RightX, CenterY - Y, Color, Transparency);

			LeftX = CenterX + Y;
			RightX = CenterX - Y;

			if (LeftX < 0) { LeftX = 0; }
			if (RightX > (BitmapBuffer->Width - 1)) { RightX = (BitmapBuffer->Width - 1); }
			DrawHorizontalLine(BitmapBuffer, LeftX, RightX, CenterY + X, Color, Transparency);
			DrawHorizontalLine(BitmapBuffer, LeftX, RightX, CenterY - X, Color, Transparency);
		}
		else
		{
			//NOT FILLED (only outer edges)
			Color = (Transparency << 24) | Color;
			DrawPixel(BitmapBuffer, CenterX + X, CenterY + Y, Color);
			DrawPixel(BitmapBuffer, CenterX - X, CenterY + Y, Color);

			DrawPixel(BitmapBuffer, CenterX + X, CenterY - Y, Color);
			DrawPixel(BitmapBuffer, CenterX - X, CenterY - Y, Color);

			DrawPixel(BitmapBuffer, CenterX + Y, CenterY + X, Color);
			DrawPixel(BitmapBuffer, CenterX + Y, CenterY - X, Color);

			DrawPixel(BitmapBuffer, CenterX - Y, CenterY + X, Color);
			DrawPixel(BitmapBuffer, CenterX - Y, CenterY - X, Color);
		}



		X++;
	}
}


internal game_entity* FindEntityWithID(game_arena* EntityArena, uint64 ID)
{
	game_entity* Result = 0;

	//returns the entity pointer using a linear search, doesn't return if entity is marked deleted or list is empty
	if (EntityArena->Memory == EntityArena->CurrentAddress)
	{
		return Result;
	}

	Result = (game_entity*)EntityArena->Memory;

	while (!(Result->ID == ID))
	{
		++Result;
	}

	return Result;
}

internal game_entity* FindEntity(game_arena* EntityArena, uint64 FLAG)
{
	game_entity* Result = 0;

	if (EntityArena->Memory == EntityArena->CurrentAddress)
	{
		return Result;
	}

	int64 EntityCount = (game_entity*)EntityArena->CurrentAddress - (game_entity*)EntityArena->Memory;

	Result = (game_entity*)EntityArena->Memory;

	for (int32 i = 0; i < EntityCount; i++)
	{
		if (Result->Flags & FLAG)
		{
			return Result;
		}
		++Result;
	}

	return 0;
}

internal void MarkEntityDeleted(game_arena* EntityArena, game_entity* Entity)
{
	Entity->Flags = Entity->Flags | F_TO_BE_DELETED;
	game_entity* HeadEntity = (game_entity*)EntityArena->FreeList;

	if (HeadEntity)
	{
		game_entity* LastEntity = (game_entity*)EntityArena->LastInFreeList;
		LastEntity->NextEntityInFreeList = Entity;
		EntityArena->LastInFreeList = Entity;

	}
	else
	{
		//this is the first entity in the freelist
		HeadEntity = Entity;
		HeadEntity->NextEntityInFreeList = 0;
		EntityArena->FreeList = HeadEntity;
		EntityArena->LastInFreeList = HeadEntity;

	}
}


internal void ClearSortedEntitiesArray(game_entity* SortedEntities)
{
	void* LastByte = SortedEntities + (MAX_ENTITY_COUNT);
	uint64* StartByte = (uint64*)SortedEntities;
	while (StartByte++ != LastByte)
	{
		*StartByte = 0;
	}
}

internal int32 Partition(game_collision_info* Array, int32 Low, int32 High)
{
	int32 Pivot = Low;
	int32 PivotValue = Array[Pivot].LeftEdge;
	int32 ItemLeft = Pivot + 1;

	for (int32 i = ItemLeft; i <= High; i++)
	{
		if (Array[i].LeftEdge < PivotValue)
		{
			game_collision_info TEMP = Array[ItemLeft];
			Array[ItemLeft] = Array[i];
			Array[i] = TEMP;
			ItemLeft++;
		}
	}
	ItemLeft--;
	game_collision_info TEMP = Array[ItemLeft];
	Array[ItemLeft] = Array[Pivot];
	Array[Pivot] = TEMP;
	return ItemLeft;
}

internal void QuickSort(game_collision_info* Array, int32 Low, int32 High)
{
	if (Low < High)
	{
		int32 Pivot = Partition(Array, Low, High);

		if (Pivot != Low)
		{
			QuickSort(Array, Low, Pivot);
		}

		if ((Pivot + 1) != High)
		{
			QuickSort(Array, Pivot + 1, High);
		}

	}
}

internal int32 ValidCollision(game_collision_info* Left, game_collision_info* Right)
{

	if ((Left->Entity->Flags & F_IS_PLAYER) && ((Right->Entity->Flags & F_IS_BULLET)) && ((Right->Entity->Flags & F_ENEMY_OWNED)))
	{
		return CT_Bullet_Player;
	}

	if (((Left->Entity->Flags & F_IS_BULLET)) && ((Left->Entity->Flags & F_ENEMY_OWNED)) && (Right->Entity->Flags & F_IS_PLAYER))
	{
		return CT_Bullet_Player;
	}

	if ((Left->Entity->Flags & (F_IS_ENEMY | F_IS_BOSS)) && ((Right->Entity->Flags & F_IS_BULLET)) && ((Right->Entity->Flags & F_PLAYER_OWNED)) )
	{
		return CT_Bullet_Enemy;
	}

	if (((Left->Entity->Flags & F_IS_BULLET)) && ((Left->Entity->Flags & F_PLAYER_OWNED)) && (Right->Entity->Flags & (F_IS_ENEMY | F_IS_BOSS)))
	{
		return CT_Bullet_Enemy;
	}

	return CT_INVALID;
}


internal void ProcessCollisions(game_memory* Memory, game_state* GameState, game_arena* EntityArena, game_arena* ScratchArena)
{
	//collisions needs Entity pointer, left edge, right edge, top, and bottom edges
	//for sorting... quicksort would work, sort against left edge (x-axis)
	game_collision_info Collidables[MAX_ENTITY_COUNT]{};
	int32 CollidableCount = 0;

	game_entity* CurrentEntity = (game_entity*)EntityArena->Memory;

	if (CurrentEntity)
	{
		//an entity exists
		//loop through all entities, skip if marked deleted or NOT collidable
		while (CurrentEntity < EntityArena->CurrentAddress)
		{
			if (!(CurrentEntity->Flags & F_TO_BE_DELETED) && (CurrentEntity->Flags & F_IS_COLLIDABLE))
			{
				//careful with truncation
				int32 HalfWidth = 0;
				int32 HalfHeight = 0;

				if (CurrentEntity->Flags & F_IS_BULLET)
				{
					HalfWidth = CurrentEntity->Radius;
					HalfHeight = HalfWidth;
				}
				else
				{
					if (CurrentEntity->Flags & F_IS_PLAYER)
					{
						if (CurrentEntity->Dashing)
						{
							real32 HitBoxScaling = 3.5f;
							HalfWidth = (int32)(CurrentEntity->Scale.a0 * HitBoxScaling);
							HalfHeight = (int32)(CurrentEntity->Scale.b1 * HitBoxScaling);
						}
						else
						{
							real32 HitBoxScaling = 0.4f;
							HalfWidth = (int32)(CurrentEntity->Scale.a0 * HitBoxScaling);
							HalfHeight = (int32)(CurrentEntity->Scale.b1 * HitBoxScaling);
						}
					}

					if (CurrentEntity->Flags & F_IS_ENEMY)
					{
						real32 HitBoxScaling = 0.55f;
						HalfWidth = (int32)(CurrentEntity->Scale.a0 * HitBoxScaling);
						HalfHeight = (int32)(CurrentEntity->Scale.b1 * HitBoxScaling);
					}

					if (CurrentEntity->Flags & F_IS_BOSS)
					{
						real32 HitBoxScaling = 0.76f;
						HalfWidth = (int32)(CurrentEntity->Scale.a0 * HitBoxScaling);
						HalfHeight = (int32)(CurrentEntity->Scale.b1 * HitBoxScaling);
					}

				}

				//valid entity for collision check, add it to array
				Collidables[CollidableCount] =
				{
					CurrentEntity,
					(int32)CurrentEntity->Position.y + HalfHeight,
					(int32)CurrentEntity->Position.y - HalfHeight,
					(int32)CurrentEntity->Position.x - HalfWidth,
					(int32)CurrentEntity->Position.x + HalfWidth,
					1
				};

				CollidableCount++;
			}

			CurrentEntity++;
		}
	}

	int32 QuickSortThreshold = 200;
	if (CollidableCount < QuickSortThreshold)
	{
		//perform simple selection sort on collidables
		for (int i = 0; i < CollidableCount; i++)
		{
			int32 Leftmost = i;
			for (int j = i + 1; j < CollidableCount; j++)
			{
				if (Collidables[j].LeftEdge < Collidables[Leftmost].LeftEdge)
				{
					Leftmost = j;
				}
			}

			if (Leftmost != i)
			{
				game_collision_info TEMP = Collidables[i];
				Collidables[i] = Collidables[Leftmost];
				Collidables[Leftmost] = TEMP;
			}
		}
	}
	else
	{
		//perform a more intensive quicksort (faster on larger arrays)
		
		QuickSort(Collidables, 0, CollidableCount - 1);

	}

	//instead of having to delete or keep a list, just loop through all active intervals each time, and only check if the interval's Active variable is set
	//an invalid interval will have it's Active integer set to 0
	int32 MAX_COLLISION_PAIRS = 50000;
	int32 EndOfActiveInterval = 0;
	int32 CurrentPotentialCollision = 0;

	game_collision_info* ActiveIntervals = (game_collision_info*)MemoryAllocate(ScratchArena, (sizeof(game_collision_info) * MAX_ENTITY_COUNT));

	game_collision_pair* PotentialCollisions = (game_collision_pair*)MemoryAllocate(ScratchArena, (sizeof(game_collision_pair) * MAX_COLLISION_PAIRS));

	if (CollidableCount > 1)
	{
		ActiveIntervals[EndOfActiveInterval] = Collidables[0];
		EndOfActiveInterval++;

		for (int32 i = 1; i < CollidableCount; i++)
		{
			int32 FrozenEndOfActiveInterval = EndOfActiveInterval;

			//check against each interval in the active intervals
			for (int32 j = 0; j < FrozenEndOfActiveInterval; j++)
			{
				if (ActiveIntervals[j].Active)
				{
					//valid collision? Player against enemy bullet, Enemy against Player Bullet, Enemy against Player, both ways for each
					game_collision_info* Left = &ActiveIntervals[j];
					game_collision_info* Right = &Collidables[i];

					int32 CollisionType = ValidCollision(Left, Right);
					
					if (CollisionType)
					{
						if (((Right->LeftEdge >= Left->LeftEdge) && (Right->LeftEdge <= Left->RightEdge)) || 
							((Left->LeftEdge >= Right->LeftEdge) && (Left->LeftEdge <= Right->RightEdge)))
						{
							//right is within interval, add right to active interval and add to potential hit list

							PotentialCollisions[CurrentPotentialCollision] = { Left, Right, CollisionType };
							CurrentPotentialCollision++;
						}
						else
						{
							//this collidable did NOT hit the interval currently being checked in active interval, deactivate this interval
							ActiveIntervals[j].Active = 0;
						}

					}
				}
			}


			//no matter what, this new collidable is added into the active interval list
			ActiveIntervals[EndOfActiveInterval] = Collidables[i];
			EndOfActiveInterval++;

		}

	}


	int32 PotentialCollisionCount = CurrentPotentialCollision;


	for (int32 i = 0; i < PotentialCollisionCount; i++)
	{
		game_collision_info* Left = PotentialCollisions[i].Left;
		game_collision_info* Right = PotentialCollisions[i].Right;
		int32 CollisionType = PotentialCollisions[i].CollisionType;
		bool32 ActualCollision = false;

		//check if actual collision occurred first before doing anything else
		if ((Right->BottomEdge >= Left->BottomEdge) && (Right->BottomEdge <= Left->TopEdge))
		{
			//right is within interval, this is a real collision
			ActualCollision = true;
		}

		if (ActualCollision)
		{

			switch (CollisionType)
			{
			case CT_Bullet_Player:
			{
				game_collision_info* Player{};
				game_collision_info* Bullet{};

				if (Left->Entity->Flags & F_IS_PLAYER)
				{
					Player = Left;
					Bullet = Right;
				}
				else
				{
					Player = Right;
					Bullet = Left;
				}

				//the player was hit by a bullet, apply the bullet's damage to the player, and mark the bullet destroyed (and whatever else)
				
				if (Player->Entity->Dashing)
				{
					Bullet->Entity->Flags ^= F_ENEMY_OWNED;
					Bullet->Entity->Flags |= F_PLAYER_OWNED;
					Bullet->Entity->Color = PLAYER_BULLET_COLOR;
					Bullet->Entity->Direction = vec3D{ -Bullet->Entity->Direction.x, -Bullet->Entity->Direction.y, 0.0f };
					Bullet->Entity->LifeTime += PLAYER_DASH_BULLET_LIFETIME_BOOST;
					Bullet->Entity->Speed *= 2.0f;
					Bullet->Entity->Radius += 3;
				}
				else
				{
					Player->Entity->Health -= Bullet->Entity->Damage;

					game_entity* HitMarker = AddEntity(Memory, GameState, (F_IS_DEATH_RING | F_BULLET_OWNED));
					HitMarker->Position = Bullet->Entity->Position;

					Memory->PlatformSubmitAudioToPlay(&Memory->SoundEffects[GSE_Hit_Sound]);

					MarkEntityDeleted(EntityArena, Bullet->Entity);
				}
				

				break;
			}
			case CT_Bullet_Enemy:
			{
				game_collision_info* Enemy{};
				game_collision_info* Bullet{};

				if (Left->Entity->Flags & (F_IS_ENEMY | F_IS_BOSS))
				{
					Enemy = Left;
					Bullet = Right;
				}
				else
				{
					Enemy = Right;
					Bullet = Left;
				}

				//an enemy was hit by a bullet, apply the bullet's damage to the enemy, and mark the bullet destroyed (and whatever else)
				//probably add code later for special enemies, like the final boss idk
				if (!(Bullet->Entity->Flags & F_TO_BE_DELETED))
				{
					Enemy->Entity->Health -= Bullet->Entity->Damage;

					game_entity* HitMarker = AddEntity(Memory, GameState, (F_IS_DEATH_RING | F_BULLET_OWNED));
					HitMarker->Position = Bullet->Entity->Position;

					Memory->PlatformSubmitAudioToPlay(&Memory->SoundEffects[GSE_Hit_Sound]);

					MarkEntityDeleted(EntityArena, Bullet->Entity);
				}

				break;
			}
			default:
			{
				//???
				break;
			}
			}
		}

	}



	/*
	
	
			//check if quicksort is working correctly
	int32 Sorted = true;
	for (int i = 0; i < CollidableCount - 1; i++)
	{
		if (Collidables[i].LeftEdge > Collidables[i + 1].LeftEdge)
		{
			Sorted = false;
			break;
		}
	}
	
	if (!Sorted)
	{
		*(int*)0 = 0;
	}
	*/
	

	
	


}

internal void** PopulateSortedEntitiesArray(void** SortedEntities, game_arena* EntityArena, game_arena* ScratchArena, int32* NumberOfEntites, int32* NumberInFreeList)
{
	//clear the array first
	//ClearSortedEntitiesArray(*SortedEntities);
	
	//go through each entity in the arena, if it is not marked to be overriden (by TO_BE_DELETED flag) add it to the array
	//then sort that array with selection sort (maybe quicksort later), this will be the update order for the game's frame
	game_entity* CurrentEntity = (game_entity*)EntityArena->Memory;
	int32 EntityCounter{0};
	int32 FreeListCounter{ 0 };

	void* BackgroundBucket[10000];
	int32 BackgroundCounter{ 0 };

	void* BulletBucket[20000];
	int32 BulletCounter{ 0 };

	void* EnemyBucket[15000];
	int32 EnemyCounter{ 0 };

	void* PlayerBucket[1000];
	int32 PlayerCounter{ 0 };

	void* UIBucket[15000];
	int32 UICounter{ 0 };

	void* DebugBucket[10000];
	int32 DebugCounter{ 0 };


	if (CurrentEntity)
	{
		while (CurrentEntity != EntityArena->CurrentAddress)
		{
			if (!(CurrentEntity->Flags & F_TO_BE_DELETED))
			{
				if (CurrentEntity->RenderOrder == RO_Background)
				{
					BackgroundBucket[BackgroundCounter] = CurrentEntity;
					BackgroundCounter++;
				}

				if (CurrentEntity->RenderOrder == RO_Bullet)
				{
					BulletBucket[BulletCounter] = CurrentEntity;
					BulletCounter++;
				}

				if (CurrentEntity->RenderOrder == RO_Enemy)
				{
					EnemyBucket[EnemyCounter] = CurrentEntity;
					EnemyCounter++;
				}

				if (CurrentEntity->RenderOrder == RO_Player)
				{
					PlayerBucket[PlayerCounter] = CurrentEntity;
					PlayerCounter++;
				}

				if (CurrentEntity->RenderOrder == RO_UI)
				{
					UIBucket[UICounter] = CurrentEntity;
					UICounter++;
				}

				if (CurrentEntity->RenderOrder == RO_Debug)
				{
					DebugBucket[DebugCounter] = CurrentEntity;
					DebugCounter++;
				}

				EntityCounter++;
			}
			else
			{
				FreeListCounter++;
			}

			CurrentEntity++;
		}
	}


	SortedEntities = (void**)MemoryAllocate(ScratchArena, sizeof(void*) * EntityCounter);
	int32 CurrentSortedEntity = 0;

	if (EntityCounter)
	{
		for (int i = 0; i < BackgroundCounter; i++)
		{
			SortedEntities[CurrentSortedEntity] = BackgroundBucket[i];
			CurrentSortedEntity++;
		}

		for (int i = 0; i < BulletCounter; i++)
		{
			SortedEntities[CurrentSortedEntity] = BulletBucket[i];
			CurrentSortedEntity++;
		}

		for (int i = 0; i < EnemyCounter; i++)
		{
			SortedEntities[CurrentSortedEntity] = EnemyBucket[i];
			CurrentSortedEntity++;
		}

		for (int i = 0; i < PlayerCounter; i++)
		{
			SortedEntities[CurrentSortedEntity] = PlayerBucket[i];
			CurrentSortedEntity++;
		}

		for (int i = 0; i < UICounter; i++)
		{
			SortedEntities[CurrentSortedEntity] = UIBucket[i];
			CurrentSortedEntity++;
		}

		for (int i = 0; i < DebugCounter; i++)
		{
			SortedEntities[CurrentSortedEntity] = DebugBucket[i];
			CurrentSortedEntity++;
		}
		

	}

	//how many entities that are updateable
	*NumberOfEntites = EntityCounter;
	*NumberInFreeList = FreeListCounter;
	return SortedEntities;
}

internal void StrConCat(char* LHS, int32 LHSLength, char* RHS, int32 RHSLength)
{
	for (int32 i = LHSLength; i < (LHSLength + RHSLength); i++)
	{
		LHS[i] = *RHS;
		RHS++;
	}
}

internal void UpdateTextureHandles(game_memory* Memory, game_state* GameState)
{
	//update all entities to use these new handles
	if ((GameState->EntityCount + GameState->FreeListCount) > 0)
	{
		game_entity* CurrentEntity = (game_entity*)Memory->EntityArena.Memory;

		for (int32 i = 0; i < (GameState->EntityCount + GameState->FreeListCount); i++)
		{

			if (CurrentEntity->Flags & F_IS_PLAYER)
			{
				CurrentEntity->TextureBitMap = &GameState->TextureHandles[Player_Sprite];
			}

			if (CurrentEntity->Flags & F_IS_ENEMY)
			{
				CurrentEntity->TextureBitMap = &GameState->TextureHandles[Enemy_Sprite];
			}

			if (CurrentEntity->Flags & F_IS_BOSS)
			{
				CurrentEntity->TextureBitMap = &GameState->TextureHandles[Boss_Sprite];
			}

			if (CurrentEntity->Flags & F_IS_RETICLE)
			{
				CurrentEntity->TextureBitMap = &GameState->TextureHandles[Reticle_Sprite];
			}


			CurrentEntity++;
		}

	}
}

internal void LoadBitmaps(game_memory* Memory, game_arena* BitMapArena, game_state* GameState)
{
	char PathString[128] = "Assets\\";

	for (int32 i = 0; i < Number_Of_Textures; i++)
	{
		switch (i)
		{
		case Player_Sprite:
		{
			StrConCat(PathString, 7, "Zippy.bmp", ARRAY_LENGTH("Zippy.bmp"));
			LoadBMP(Memory, BitMapArena, PathString, &GameState->TextureHandles[Player_Sprite]);
			break;
		}
		
		case Font_Sprite:
		{
			StrConCat(PathString, 7, "Font.bmp", ARRAY_LENGTH("Font.bmp"));
			LoadBMP(Memory, BitMapArena, PathString, &GameState->TextureHandles[Font_Sprite]);
			break;
		}

		case Enemy_Sprite:
		{
			StrConCat(PathString, 7, "Enemy.bmp", ARRAY_LENGTH("Enemy.bmp"));
			LoadBMP(Memory, BitMapArena, PathString, &GameState->TextureHandles[Enemy_Sprite]);
			break;
		}


		case Boss_Sprite:
		{
			StrConCat(PathString, 7, "Zappy.bmp", ARRAY_LENGTH("Zappy.bmp"));
			LoadBMP(Memory, BitMapArena, PathString, &GameState->TextureHandles[Boss_Sprite]);
			break;
		}

		case Reticle_Sprite:
		{
			StrConCat(PathString, 7, "Reticle.bmp", ARRAY_LENGTH("Reticle.bmp"));
			LoadBMP(Memory, BitMapArena, PathString, &GameState->TextureHandles[Reticle_Sprite]);
			break;
		}

		default:
		{
			//???
			break;
		}
		}
	}


}




internal void DrawAllEnemies(game_bitmap_buffer* BitmapBuffer, game_enemy_draw_info* EnemyInfo, int32 NumberOfEnemies, int32 Width, int32 Height, game_entity_bitmap* Texture)
{

	if (Width < 1)
	{
		Width = 1;
	}
	if (Height < 1)
	{
		Height = 1;
	}

	int32 LeftX = -(Width / 2);

	int32 RightX = (Width / 2);

	int32 TopY = -(Height / 2);

	int32 BottomY = (Height / 2);

	real32 U = 0.0f; //left-most texture coord
	real32 V = 1.0f; //top-most texture coord initially
	real32 DeltaU = 1.0f / ((real32)Width);
	real32 DeltaV = 1.0f / ((real32)Height);

	for (; TopY <= BottomY; TopY++)
	{
		for (int32 CurrentX = LeftX; CurrentX <= RightX; CurrentX++)
		{
			int32 ScaledWidth = (int32)(U * (real32)(Texture->Width));
			int32 ScaledHeight = (int32)(V * (real32)(Texture->Height));

			int32 ByteOffset = (ScaledWidth + (ScaledHeight * Texture->Width)) * 3;

			uint32 Color = SampleTexture(Texture, ByteOffset);
			//draw pixel if pixel is valid
			if (Color != 0)
			{
				for (int32 i = 0; i < NumberOfEnemies; i++)
				{
					Color = (EnemyInfo[i].Transparency << 24) | Color;
					DrawPixel(BitmapBuffer, EnemyInfo[i].X + CurrentX, EnemyInfo[i].Y + TopY, Color);
				}
			}

			U += DeltaU;
		}

		U = 0.0f;
		V -= DeltaV;
	}




}

/*
	Credit to LearnCPP.com for this PRNG mersenne twister initialization.
*/
internal std::mt19937 SeedPRNG()
{
	std::random_device RD{};

	std::seed_seq SeedSequence{ RD(), RD(), RD(), static_cast<std::seed_seq::result_type>(std::chrono::steady_clock::now().time_since_epoch().count()), RD(), RD(), RD(), RD() };

	return std::mt19937{ SeedSequence };
}

inline int32 GetRandomInt(int32 Min, int32 Max, std::mt19937* PRNG)
{
	return std::uniform_int_distribution<int32>{Min, Max}(*PRNG);
}

inline real32 GetRandomReal(real32 Min, real32 Max, std::mt19937* PRNG)
{
	return std::uniform_real_distribution<real32>{Min, Max}(*PRNG);
}

internal game_entity* CreateEnemy(game_memory* Memory, game_state* GameState, real32 XPos, real32 YPos, int32 Behavior, int32 Weapon)
{
	game_entity* Enemy = AddEntity(Memory, GameState, F_IS_ENEMY);
	Enemy->Position = { XPos, YPos, 0.0f };
	Enemy->EnemyBehavior = Behavior;
	Enemy->EnemyWeapon = Weapon;

	switch (Enemy->EnemyBehavior)
	{
	case EB_Stationary:
	{

		break;
	}

	case EB_Random:
	{
		
		break;
	}
	case EB_Circling:
	{
		Enemy->BulletSpawnPoint = Enemy->Position - vec3D{Enemy->Scale.a0 * 1.45f, 0.0f, 0.0f};
		
		Enemy->FlippedCircling = GetRandomInt(0, 1, &GameState->PRNG);
		
		break;
	}

	case EB_Pursuit:
	{
		Enemy->Speed = GameState->EnemySpeed * 1.5f;
		break;
	}
	case EB_Afraid:
	{
		Enemy->Speed = GameState->EnemySpeed * 2.1f;

		break;
	}

	default:
	{
		break;
	}
	}

	switch (Enemy->EnemyWeapon)
	{
	case EW_Single:
	{
		Enemy->FireRate = GameState->EnemyFireRateMult;
		Enemy->BulletSpread = false;
		break;
	}

	case EW_Double:
	{
		Enemy->FireRate = GameState->EnemyFireRateMult * 2.5f;
		Enemy->BulletSpreadVariance = ENEMY_DOUBLE_SHOT_SPREAD;
		break;
	}
	case EW_Burst:
	{
		Enemy->FireRate = GameState->EnemyFireRateMult * ENEMY_BURST_COOLDOWN;
		Enemy->BulletSpreadVariance = ENEMY_BURST_SHOT_SPREAD;
		Enemy->BurstShotCount = ENEMY_BURST_SHOT_AMOUNT;

		break;
	}

	case EW_Triple:
	{
		Enemy->FireRate = GameState->EnemyFireRateMult * 3.0f;
		Enemy->BulletSpreadVariance = ENEMY_TRIPLE_SHOT_SPREAD;
		break;
	}
	case EW_Mega:
	{
		Enemy->FireRate = GameState->EnemyFireRateMult * 7.0f;
		Enemy->BulletSpreadVariance = ENEMY_MEGA_SHOT_SPREAD;
		break;
	}
	case EW_AOE:
	{
		Enemy->FireRate = GameState->EnemyFireRateMult * ENEMY_AOE_SHOT_FIRERATE;
		Enemy->BulletSpread = false;
		break;
	}

	default:
	{
		break;
	}
	}

	return Enemy;
}

internal void AddNumberToEntityString(game_entity* Entity, int32 NumberToAdd, int32 StartIndex)
{
	const int32 Digits = 33;
	char NumberString[Digits];
	_itoa_s(NumberToAdd, NumberString, 10);
	int32 l = 0;
	for (int32 k = StartIndex; k < StartIndex + Digits; k++)
	{
		Entity->StringMessage[k] = NumberString[l++];
		
		if (NumberString[l] == '\0')
		{
			break;
		}
	}
}

internal void ResetArena(game_arena* Arena)
{
	//zero out arena memory first
	
	uint64* ClearPointer = (uint64*)Arena->Memory;
	uint64* EndPoint = (uint64*)Arena->CurrentAddress;

	while (ClearPointer++ != EndPoint)
	{
		*ClearPointer = 0;
	}
	


	//now set to default
	Arena->CurrentAddress = Arena->Memory;
	Arena->FreeList = 0;
	Arena->LastInFreeList = 0;
}

internal inline void ToggleRenderable(game_entity* Entity)
{
	Entity->Flags ^= (F_RENDERABLE);
}

//only check in-game and NOT during the boss battle
internal bool32 CheckIfWaveDefeated(game_arena* EntityArena)
{
	game_entity* CurrentEntity = (game_entity*)EntityArena->Memory;

	//searches through all entities, if it's an enemy and NOT deleted, then the wave is still on-going
	while (CurrentEntity++ != EntityArena->CurrentAddress)
	{
		if ((CurrentEntity->Flags & F_IS_ENEMY) && !(CurrentEntity->Flags & F_TO_BE_DELETED))
		{
			return false;
		}
	}

	return true;
}

internal void SetupStartScreen(game_memory* Memory, game_state* GameState)
{
	ResetArena(&Memory->EntityArena);

	GameState->GameMenuState = GMS_Start_Screen;
	GameState->GamePaused = false;
	GameState->FightingBoss = false;
	GameState->GameEndType = GES_Game_Ongoing;

	GameState->DebugInfo = AddEntity(Memory, GameState, (F_IS_STRING | F_IS_DEBUG_INFO));
	GameState->DebugInfo->Position = { 15.0f, 1050.0f, 0.0f };
	GameState->DebugInfo->Scale = ScaleMat4(0.5f, 0.5f, 1.0);
	GameState->DebugInfo->Color = uint32ColorfromRGB(230, 5, 199);
	GameState->DebugInfo->Transparency = 185;
	ToggleRenderable(GameState->DebugInfo);

	//follows mouse, hidden if controller is active (DURING WAVES)
	GameState->Reticle = AddEntity(Memory, GameState, F_IS_RETICLE);

	GameState->StartButton = AddEntity(Memory, GameState, (F_IS_BUTTON | F_IS_START_BUTTON));
	GameState->LoadButton = AddEntity(Memory, GameState, (F_IS_BUTTON | F_IS_LOAD_BUTTON));
	GameState->ExitButton = AddEntity(Memory, GameState, (F_IS_BUTTON | F_IS_EXIT_BUTTON));

	game_entity* GameTitle = AddEntity(Memory, GameState, F_IS_STRING);
	GameTitle->Color = PLAYER_BULLET_COLOR;
	GameTitle->Position = { 675.0f, 215.0f,0.0f };
	GameTitle->Scale = ScaleMat4(GAME_TITLE_SCALE_X, GAME_TITLE_SCALE_Y, 0.0f);
	StringCopy(GameTitle->StringMessage, 7, "ZIPPY!");


}


internal void SpawnBoss(game_memory* Memory, game_state* GameState, game_bitmap_buffer* BitmapBuffer)
{
	game_entity* NewEntity = AddEntity(Memory, GameState, F_IS_BOSS);
	NewEntity->Position = { (real32)(BitmapBuffer->Width / 2), (real32)(BitmapBuffer->Height / 2), 0.0f };
	GameState->Boss = NewEntity;

	NewEntity = AddEntity(Memory, GameState, (F_IS_UI_BAR | F_BOSS_OWNED));
	NewEntity->Position = { BOSS_HP_BAR_XPOS, BOSS_HP_BAR_YPOS, 0.0f };
	NewEntity->Scale = ScaleMat4(BOSS_HP_BAR_WIDTH, BOSS_HP_BAR_HEIGHT, 0.0f);
	NewEntity->Color = uint32ColorfromRGB(225, 5, 15);

	NewEntity = AddEntity(Memory, GameState, (F_IS_STRING));
	char BOSS_HP_BAR_NAME[] = "<ZAPPY>";
	NewEntity->Scale = ScaleMat4(0.65f, 0.65f, 0.0f);
	StringCopy(NewEntity->StringMessage, MAX_ENTITY_STRING_CHAR, BOSS_HP_BAR_NAME);
	NewEntity->Color = uint32ColorfromRGB(200, 5, 15);
	NewEntity->Position = { BOSS_HP_BAR_XPOS - 80.0f, BOSS_HP_BAR_YPOS - 30.0f, 0.0f };

}

internal void SpawnPlayer(game_memory* Memory, game_state* GameState)
{
	GameState->Player = AddEntity(Memory, GameState, F_IS_PLAYER);
	GameState->Player->Position = { 500.0f, 200.0f, 0.0f };

	game_entity* NewEntity = AddEntity(Memory, GameState, F_IS_PLAYER_HEALTH);
	NewEntity->Position = { UI_OFFSET_PIXELS, UI_OFFSET_PIXELS, 0.0f };
	NewEntity->Color = uint32ColorfromRGB(45, 245, 45);

	NewEntity = AddEntity(Memory, GameState, F_IS_PLAYER_GUN_BAR);
	NewEntity->Color = uint32ColorfromRGB(100, 100, 100);
	NewEntity->Position = { UI_OFFSET_PIXELS, 2.5f * UI_OFFSET_PIXELS, 0.0f };

	NewEntity = AddEntity(Memory, GameState, F_IS_PLAYER_DASH_COUNTER);
	NewEntity->Color = uint32ColorfromRGB(45, 245, 45);
	NewEntity->Position = { UI_OFFSET_PIXELS - 5.0f, 4.0f * UI_OFFSET_PIXELS, 0.0f };

	NewEntity = AddEntity(Memory, GameState, F_IS_PLAYER_BURST_COUNTER);
	NewEntity->Color = uint32ColorfromRGB(45, 245, 45);
	NewEntity->Position = { UI_OFFSET_PIXELS - 5.0f, 5.5f * UI_OFFSET_PIXELS, 0.0f };

}

internal void SpawnWave(game_memory* Memory, game_state* GameState, game_bitmap_buffer* BitMap)
{
	for (int32 i = 0; i < GameState->CurrentWave; i++)
	{
		int32 RandomBehavior = GetRandomInt(EB_Stationary, EB_Afraid, &GameState->PRNG);
		int32 RandomWeapon = GetRandomInt(EW_Single, EW_AOE, &GameState->PRNG);
		int32 EnemiesPerGroup = (int32)(GameState->EnemyCountPerWave * (real32)GameState->CurrentWave);

		vec3D GroupSpawnPosition = { GetRandomReal(0.0f, (real32)BitMap->Width - ENEMY_GROUP_POSITION_VARIANCE, &GameState->PRNG),
									 GetRandomReal(0.0f, (real32)BitMap->Height - ENEMY_GROUP_POSITION_VARIANCE, &GameState->PRNG),
									 0.0f
		};

		//don't let the group spawn too close to the player
		while (DistanceSquared(GroupSpawnPosition, GameState->Player->Position) < ENEMY_GROUP_SPAWN_DISTANCE)
		{
			GroupSpawnPosition = { GetRandomReal(0.0f, (real32)BitMap->Width - ENEMY_GROUP_POSITION_VARIANCE, &GameState->PRNG),
								   GetRandomReal(0.0f, (real32)BitMap->Height - ENEMY_GROUP_POSITION_VARIANCE, &GameState->PRNG),
								   0.0f
			};
		}


		for (int32 j = 0; j < EnemiesPerGroup; j++)
		{
			real32 PositionVariance = GetRandomReal(-ENEMY_GROUP_POSITION_VARIANCE, ENEMY_GROUP_POSITION_VARIANCE, &GameState->PRNG);
			CreateEnemy(Memory, GameState, GroupSpawnPosition.x + PositionVariance, GroupSpawnPosition.y + PositionVariance, RandomBehavior, RandomWeapon);
		}
	}
}


internal void BeginWaves(game_memory* Memory, game_state* GameState, game_bitmap_buffer* BitmapBuffer)
{
	ResetArena(&Memory->EntityArena);

	GameState->DebugInfo = AddEntity(Memory, GameState, (F_IS_STRING | F_IS_DEBUG_INFO));
	GameState->DebugInfo->Position = { 15.0f, 1050.0f, 0.0f };
	GameState->DebugInfo->Scale = ScaleMat4(0.5f, 0.5f, 1.0);
	GameState->DebugInfo->Color = uint32ColorfromRGB(230, 5, 199);
	GameState->DebugInfo->Transparency = 185;
	ToggleRenderable(GameState->DebugInfo);

	GameState->Reticle = AddEntity(Memory, GameState, F_IS_RETICLE);

	SpawnPlayer(Memory, GameState);

	game_entity* NewEntity = AddEntity(Memory, GameState, F_IS_WAVE_COUNTER);
	NewEntity->Color = uint32ColorfromRGB(252, 252, 252);
	NewEntity->Position = { (real32)(BitmapBuffer->Width / 2) - 100.0f, 30.0f, 0.0f };

	GameState->CurrentWave = 1;
	GameState->EnemyFireRateMult = 1.0f;
	GameState->EnemySpeed = ENEMY_BASE_SPEED;

	GameState->PlayerFireRateIncreasePerWave = ((PLAYER_BASE_FIRERATE - MAX_PLAYER_FIRERATE) / (real32)WAVES_BEFORE_BOSS);
	GameState->EnemyFireRateIncreasePerWave = ((ENEMY_BASE_FIRERATE - ENEMY_MAX_FIRERATE_MULT) / (real32)WAVES_BEFORE_BOSS);
	GameState->EnemySpeedUpPerWave = ((ENEMY_MAX_SPEED - ENEMY_BASE_SPEED) / (real32)WAVES_BEFORE_BOSS);

	GameState->EnemyCountPerWave = (real32)MAX_ENEMIES_PER_GROUP / (real32)WAVES_BEFORE_BOSS;

	//spawn enemies for the first wave
	SpawnWave(Memory, GameState, BitmapBuffer);
	GameState->WaveSpawnTimer = TIME_BETWEEN_WAVES;

	GameState->GameMenuState = GMS_In_Game;
	GameState->FightingBoss = false;
}

internal void AdvanceWave(game_memory* Memory, game_state* GameState, game_bitmap_buffer* BitMapBuffer)
{
	GameState->CurrentWave++;
	GameState->Player->FireRate -= GameState->PlayerFireRateIncreasePerWave;
	GameState->EnemyFireRateMult -= GameState->EnemyFireRateIncreasePerWave;
	GameState->EnemySpeed += GameState->EnemySpeedUpPerWave;


	if (GameState->CurrentWave % 2 == 0)
	{
		GameState->Player->BurstShotCount += 7;
		GameState->Player->MaxBurstCharges++;
		GameState->Player->BurstCooldown -= 0.225f;
		GameState->Player->Health++;
		GameState->Player->MaxDashCharges++;
		GameState->Player->DashCooldown -= 0.175f;
	}


	//spawn enemies here, or go into boss battle
	if (GameState->CurrentWave <= WAVES_BEFORE_BOSS)
	{
		SpawnWave(Memory, GameState, BitMapBuffer);
	}
	else
	{
		SpawnBoss(Memory, GameState, BitMapBuffer);
		GameState->FightingBoss = true;
	}
}

internal void SaveGame(game_memory* Memory, char* FileName)
{
	//so, I have a way to partially write to a file now, go sequentially through memory, following void pointers and writing their contents to the file
	int32 FileOffsetBytes = 0;

	Memory->PlatformWriteToFile(FileName, FileOffsetBytes, sizeof(Memory->IsInitialized), &Memory->IsInitialized);
	FileOffsetBytes += sizeof(Memory->IsInitialized);

	Memory->PlatformWriteToFile(FileName, FileOffsetBytes, Memory->PermanentStorageSize, Memory->PermanentStorage);
	FileOffsetBytes += (int32)Memory->PermanentStorageSize;

	int64 EntityBytes = (uint8*)Memory->EntityArena.CurrentAddress - (uint8*)Memory->EntityArena.Memory;
	
	Memory->PlatformWriteToFile(FileName, FileOffsetBytes, sizeof(EntityBytes), &EntityBytes);
	FileOffsetBytes += sizeof(EntityBytes);

	Memory->PlatformWriteToFile(FileName, FileOffsetBytes, EntityBytes, Memory->EntityArenaStorage);
	FileOffsetBytes += (int32)EntityBytes;

	Memory->PlatformWriteToFile(FileName, FileOffsetBytes, sizeof(Memory->EntityArena), &Memory->EntityArena);
	FileOffsetBytes += sizeof(Memory->EntityArena);

	Memory->PlatformWriteToFile(FileName, FileOffsetBytes, sizeof(Memory->MSForLastFrame), &Memory->MSForLastFrame);
	FileOffsetBytes += sizeof(Memory->MSForLastFrame);

	Memory->PlatformWriteToFile(FileName, FileOffsetBytes, sizeof(Memory->GameRunning), &Memory->GameRunning);
	FileOffsetBytes += sizeof(Memory->GameRunning);

}

internal void MemCopy(void* Destination, uint64 BytesToWrite, void* Source)
{
	uint8* DestBytePointer = (uint8*)Destination;
	uint8* SrcBytePointer = (uint8*)Source;

	for (int64 i = 0; i < (int64)BytesToWrite; i++)
	{
		*DestBytePointer = *SrcBytePointer;
		DestBytePointer++;
		SrcBytePointer++;
	}
}

internal void RepairFreeList(void* OldArenaBeginning, game_arena* EntityArena)
{
	int64 ByteShift = 0;
	game_entity* CurrentEntity = (game_entity*)EntityArena->Memory;
	uint8* ByteOldArenaBeginning = (uint8*)OldArenaBeginning;

	int64 EntityCount = (game_entity*)EntityArena->CurrentAddress - (game_entity*)EntityArena->Memory;

	for (int32 i = 0; i < EntityCount; i++)
	{
		if (CurrentEntity->NextEntityInFreeList)
		{
			ByteShift = (uint8*)CurrentEntity->NextEntityInFreeList - ByteOldArenaBeginning;
			CurrentEntity->NextEntityInFreeList = (game_entity*)((uint8*)EntityArena->Memory + ByteShift);
		}

		CurrentEntity++;
	}

	ByteShift = (uint8*)EntityArena->FreeList - ByteOldArenaBeginning;
	EntityArena->FreeList = (uint8*)EntityArena->Memory + ByteShift;


	ByteShift = (uint8*)EntityArena->LastInFreeList - ByteOldArenaBeginning;
	EntityArena->LastInFreeList = (uint8*)EntityArena->Memory + ByteShift;


}

internal void LoadGame(game_memory* Memory, game_state* GameState, char* FileName)
{
	read_file_result FileResult = Memory->PlatformReadEntireFile(FileName);

	if (FileResult.ContentsSize != SAVE_FILE_EMPTY_CONTENT_SIZE)
	{


		uint8* FileResultByte = (uint8*)FileResult.Contents;


		Memory->IsInitialized = *((bool32*)FileResultByte);
		FileResultByte += sizeof(Memory->IsInitialized);


		MemCopy(Memory->PermanentStorage, Memory->PermanentStorageSize, FileResultByte);
		FileResultByte += Memory->PermanentStorageSize;

		int64 EntityBytes{};

		MemCopy(&EntityBytes, sizeof(EntityBytes), FileResultByte);
		FileResultByte += sizeof(EntityBytes);

		MemCopy(Memory->EntityArenaStorage, EntityBytes, FileResultByte);
		FileResultByte += EntityBytes;

		MemCopy(&Memory->EntityArena, sizeof(Memory->EntityArena), FileResultByte);
		FileResultByte += sizeof(Memory->EntityArena);

		MemCopy(&Memory->MSForLastFrame, sizeof(Memory->MSForLastFrame), FileResultByte);
		FileResultByte += sizeof(Memory->MSForLastFrame);

		MemCopy(&Memory->GameRunning, sizeof(Memory->GameRunning), FileResultByte);
		FileResultByte += sizeof(Memory->GameRunning);

		
		GameState = (game_state*)Memory->PermanentStorage;


		uint64 BytesToAdd = (uint8*)Memory->EntityArena.CurrentAddress - (uint8*)Memory->EntityArena.Memory;
		void* OldArenaBeginning = Memory->EntityArena.Memory;

		
		Memory->EntityArena.Memory = Memory->EntityArenaStorage;
		Memory->EntityArena.CurrentAddress = (uint8*)Memory->EntityArena.Memory + BytesToAdd;

		Memory->ScratchArena.Memory = Memory->ScratchArenaStorage;
		Memory->ScratchArena.CurrentAddress = Memory->ScratchArena.Memory;

		RepairFreeList(OldArenaBeginning, &Memory->EntityArena);

		
		GameState->Player = FindEntity(&Memory->EntityArena, F_IS_PLAYER);

		GameState->DebugInfo = FindEntity(&Memory->EntityArena, F_IS_DEBUG_INFO);

		GameState->Boss = FindEntity(&Memory->EntityArena, F_IS_BOSS);

		GameState->Reticle = FindEntity(&Memory->EntityArena, F_IS_RETICLE);

		GameState->StartButton = FindEntity(&Memory->EntityArena, F_IS_START_BUTTON);

		GameState->LoadButton = FindEntity(&Memory->EntityArena, F_IS_LOAD_BUTTON);

		GameState->ExitButton = FindEntity(&Memory->EntityArena, F_IS_EXIT_BUTTON);
		
		Memory->BitMapArena.Memory = Memory->BitMapArenaStorage;
		Memory->BitMapArena.CurrentAddress = Memory->BitMapArena.Memory;

		LoadBitmaps(Memory, &Memory->BitMapArena, GameState);
		UpdateTextureHandles(Memory, GameState);
	}

	//don't forget to free the loaded memory from reading the file!!!
	Memory->PlatformFreeFileMemory(FileResult.Contents);


}

internal void InitializeSoundEffects(game_memory* Memory)
{
	Memory->SoundEffects[GSE_BGM] = { "Assets\\Vivaldi - Violin Concerto in F minor - Winter - Op. 8, No. 4 - RV 297.wav", 2, 44100, 24, BACKGROUND_MUSIC_VOLUME};

	Memory->PlatformFormatAudioBeforePlayback(&Memory->SoundEffects[GSE_BGM], MemoryAllocate, &Memory->AudioDataArena);

	

	Memory->SoundEffects[GSE_Entity_Death] = { "Assets\\107788__leviclaassen__hit_001.wav", 2, 44100, 24, ENTITY_DEATH_SOUND_VOLUME };
	Memory->PlatformFormatAudioBeforePlayback(&Memory->SoundEffects[GSE_Entity_Death], MemoryAllocate, &Memory->AudioDataArena);

	Memory->SoundEffects[GSE_Hit_Sound] = { "Assets\\49682__ejfortin__nano-blade-3.wav", 2, 44100, 24, HIT_SOUND_VOLUME };
	Memory->PlatformFormatAudioBeforePlayback(&Memory->SoundEffects[GSE_Hit_Sound], MemoryAllocate, &Memory->AudioDataArena);

}

/*
extern "C" void  GameAllocateToBufferMemory(game_arena* Arena, uint64 BytesToAllocate)
{
	MemoryAllocate(Arena, BytesToAllocate);
}
*/


extern "C" void GameUpdateAndRender(game_memory* Memory, game_input* Input, game_bitmap_buffer* BitmapBuffer, real32 DeltaTime)
{

	game_state* GameState = (game_state*)Memory->PermanentStorage;

	if (!Memory->IsInitialized)
	{
		Memory->GameRunning = true;

		GameState->EntityIDCounter = 0;

		GameState->PRNG = SeedPRNG();

		FillColor(BitmapBuffer, uint32ColorfromRGB(15, 20, 34));


		Memory->BitMapArena.Memory = Memory->BitMapArenaStorage;
		Memory->BitMapArena.CurrentAddress = Memory->BitMapArena.Memory;

		//initialize Entity Area to point at the buffer in memory
		Memory->EntityArena.Memory = Memory->EntityArenaStorage;
		Memory->EntityArena.CurrentAddress = Memory->EntityArena.Memory;

		Memory->ScratchArena.Memory = Memory->ScratchArenaStorage;
		Memory->ScratchArena.CurrentAddress = Memory->ScratchArena.Memory;

		Memory->AudioDataArena.Memory = Memory->AudioDataArenaStorage;
		Memory->AudioDataArena.CurrentAddress = Memory->AudioDataArena.Memory;

		//load all bitmaps into gamestate variables (to refer to later during entity creation)
		LoadBitmaps(Memory, &Memory->BitMapArena, GameState);

		//StringCopy(Temp->StringMessage, MAX_ENTITY_STRING_CHAR, "This should display over everything else...");

		GameState->GameMenuState = GMS_Start_Screen;

		SetupStartScreen(Memory, GameState);

		read_file_result SaveFileResult = Memory->PlatformReadEntireFile("SaveFile");

		if (SaveFileResult.ContentsSize == 0)
		{
			//no save file present, create one
			char DummySaveFileText[2] = "";
			Memory->PlatformWriteEntireFile("SaveFile", sizeof(DummySaveFileText), DummySaveFileText);
		}


		InitializeSoundEffects(Memory);

		Memory->PlatformSubmitAudioToPlay(&Memory->SoundEffects[GSE_BGM]);

		Memory->BGMLoopTimer = BACKGROUND_MUISC_LOOP_TIME;

		Memory->PlatformFreeFileMemory(SaveFileResult.Contents);

		Memory->IsInitialized = true;
	}

	//RESETS SCRATCH ARENA!! EVERY FRAME
	Memory->ScratchArena.CurrentAddress = Memory->ScratchArena.Memory;

	Memory->BGMLoopTimer -= DeltaTime;
	if (Memory->BGMLoopTimer <= 0.0f)
	{
		Memory->PlatformSubmitAudioToPlay(&Memory->SoundEffects[GSE_BGM]);
		Memory->BGMLoopTimer = BACKGROUND_MUISC_LOOP_TIME;
	}


	FillColor(BitmapBuffer, uint32ColorfromRGB(15, 20, 34));



	//create sorted list of entities
	GameState->SortedEntities = PopulateSortedEntitiesArray(GameState->SortedEntities, &Memory->EntityArena, &Memory->ScratchArena, &GameState->EntityCount, &GameState->FreeListCount);


	//mouse stuff
	/*
	Processes the first frame left click was released
	!Input->MouseButtons[0].EndedDown && Input->MouseButtons[0].HalfTransitions == 1

	Processes the first frame left click was pressed
	Input->MouseButtons[0].EndedDown && Input->MouseButtons[0].HalfTransitions == 1

	Processes a held left click (repeats each frame left click is held)
	Input->MouseButtons[0].EndedDown
	*/

	//keyboard and mouse control logic
	if (!Input->ControllerDetected)
	{
		Input->LeftStickX = 0.0f;
		Input->LeftStickY = 0.0f;

		if (Input->Up.EndedDown)
		{
			Input->LeftStickY = 1.0f;
		}
		if (Input->Down.EndedDown)
		{
			Input->LeftStickY -= 1.0f;
		}
		if (Input->Left.EndedDown)
		{
			Input->LeftStickX = -1.0f;
		}
		if (Input->Right.EndedDown)
		{
			Input->LeftStickX += 1.0f;
		}

		if (Input->MouseButtons[0].EndedDown)
		{
			Input->RightShoulder.EndedDown = Input->MouseButtons[0].EndedDown;
			Input->RightShoulder.HalfTransitions = Input->MouseButtons[0].HalfTransitions;
		}
		else
		{
			Input->RightShoulder.EndedDown = false;
			Input->RightShoulder.HalfTransitions = 0;
		}

		if (Input->MouseButtons[1].EndedDown)
		{
			Input->LeftShoulder.EndedDown = Input->MouseButtons[1].EndedDown;
			Input->LeftShoulder.HalfTransitions = Input->MouseButtons[1].HalfTransitions;
		}
		else
		{
			Input->LeftShoulder.EndedDown = false;
			Input->LeftShoulder.HalfTransitions = 0;
		}


		//check if in a wave, if yes, make sure cursor is visible
		if (GameState->GameMenuState == GMS_In_Game)
		{
			vec3D DigitalRightStick = Normalize({ (real32)GameState->LastMouseX - GameState->Player->Position.x, (real32)GameState->LastMouseY - GameState->Player->Position.y, 0.0f });
			Input->RightStickX = DigitalRightStick.x;
			Input->RightStickY = -DigitalRightStick.y;

			if (!(GameState->Reticle->Flags & F_RENDERABLE))
			{
				ToggleRenderable(GameState->Reticle);
			}
		}

	}
	else
	{
		//CONTROLLER CONNECTED, hide reticle, if not paused
		if (GameState->GameMenuState == GMS_In_Game && !GameState->GamePaused)
		{
			if (GameState->GameEndType == GES_Game_Ongoing && (GameState->Reticle->Flags & F_RENDERABLE))
			{
				ToggleRenderable(GameState->Reticle);
			}
			if (GameState->GameEndType != GES_Game_Ongoing)
			{
				if (!(GameState->Reticle->Flags & F_RENDERABLE))
				{
					ToggleRenderable(GameState->Reticle);
				}
			}
		}
		if (GameState->GameMenuState == GMS_In_Game && GameState->GamePaused)
		{
			if (!(GameState->Reticle->Flags & F_RENDERABLE))
			{
				ToggleRenderable(GameState->Reticle);
			}
		}
	}


	if (Input->Back.EndedDown && Input->Back.HalfTransitions == 1)
	{

		ToggleRenderable(GameState->DebugInfo);
	}

	//PAUSE TOGGLE
	if (Input->Start.EndedDown && Input->Start.HalfTransitions == 1)
	{
		if (GameState->GameEndType == GES_Game_Ongoing)
		{
			if (GameState->GameMenuState == GMS_In_Game && GameState->GamePaused)
			{
				MarkEntityDeleted(&Memory->EntityArena, GameState->ScreenText);
				if (!(GameState->ExitButton->Flags & F_TO_BE_DELETED))
				{
					MarkEntityDeleted(&Memory->EntityArena, GameState->ExitButton);
				}
				GameState->GamePaused = false;
			}
			else if (GameState->GameMenuState == GMS_In_Game && !GameState->GamePaused)
			{
				GameState->ScreenText = AddEntity(Memory, GameState, (F_IS_STRING));
				GameState->ScreenText->Position = { 725.0f, 200.0f,0.0f };
				GameState->ScreenText->Scale = ScaleMat4(2.0f, 2.0f, 0.0f);
				GameState->ScreenText->Color = uint32ColorfromRGB(15, 15, 205);
				StringCopy(GameState->ScreenText->StringMessage, 7, "PAUSED");

				GameState->ExitButton = AddEntity(Memory, GameState, (F_IS_BUTTON | F_IS_EXIT_BUTTON));

				GameState->GamePaused = true;
			}
		}

	}

	//loop through all sorted entites and update them here
	for (int32 i = 0; i < GameState->EntityCount; i++)
	{
		game_entity* Entity = (game_entity*)GameState->SortedEntities[i];

		//Done whether game is paused or not
		if (Entity->Flags & F_IS_RETICLE)
		{

			Entity->Position.x = (real32)Input->MouseXPosition;
			Entity->Position.y = (real32)Input->MouseYPosition;

		}

		if (Entity->Flags & F_IS_BUTTON)
		{
			int32 LeftButtonEdge = (int32)(Entity->Position.x - (BUTTON_WIDTH / 2.0f));
			int32 RightButtonEdge = (int32)(Entity->Position.x + (BUTTON_WIDTH / 2.0f));
			int32 TopButtonEdge = (int32)(Entity->Position.y + (BUTTON_HEIGHT / 2.0f));
			int32 BottomButtonEdge = (int32)(Entity->Position.y - (BUTTON_HEIGHT / 2.0f));

			//check if mouse is hovering over button

			if ((GameState->LastMouseX >= LeftButtonEdge) && (GameState->LastMouseX <= RightButtonEdge)
				&& (GameState->LastMouseY <= TopButtonEdge) && (GameState->LastMouseY >= BottomButtonEdge))
			{
				//mouse inside button box
				Entity->Color = uint32ColorfromRGB(235, 235, 235);
				Entity->ButtonHoveredOver = true;
			}
			else
			{
				Entity->Color = uint32ColorfromRGB(1, 1, 1);
				Entity->ButtonHoveredOver = false;
			}

			if (Entity->ButtonHoveredOver && Input->MouseButtons[0].EndedDown && Input->MouseButtons[0].HalfTransitions == 1)
			{
				//button clicked
				if (GameState->GameMenuState == GMS_Start_Screen)
				{

					if (Entity->Flags & F_IS_START_BUTTON)
					{
						BeginWaves(Memory, GameState, BitmapBuffer);
					}
					else if (Entity->Flags & F_IS_LOAD_BUTTON)
					{

						LoadGame(Memory, GameState, "SaveFile");

						break;
					}
					else if (Entity->Flags & F_IS_EXIT_BUTTON)
					{
						//from start screen, exit button doesn't save, just closes game
						Memory->GameRunning = false;
					}
				}
				else if (GameState->GameMenuState == GMS_In_Game)
				{
					if (Entity->Flags & F_IS_EXIT_BUTTON)
					{
						GameState->GamePaused = false;
						MarkEntityDeleted(&Memory->EntityArena, GameState->ScreenText);
						MarkEntityDeleted(&Memory->EntityArena, GameState->ExitButton);
						
						//only save if the game hasn't ended
						if (GameState->GameEndType == GES_Game_Ongoing)
						{
							SaveGame(Memory, "SaveFile");
						}

						SetupStartScreen(Memory, GameState);
					}
				}



			}

		}



		if (Entity == GameState->DebugInfo)
		{
			for (int j = 0; j < MAX_ENTITY_STRING_CHAR; j++)
			{
				Entity->StringMessage[j] = '\0';
			}
			sprintf_s(GameState->CharacterBuffer, "%d FPS | Entities:%d | Free:%d | Controller:%d",
				(int32)(1.0f / Memory->MSForLastFrame), GameState->EntityCount, GameState->FreeListCount, Input->ControllerDetected);
			StringCopy(Entity->StringMessage, MAX_ENTITY_STRING_CHAR, GameState->CharacterBuffer);
		}

		
		//This code only runs when game isn't paused (game entity stuffs)
		if (!GameState->GamePaused && GameState->GameEndType == GES_Game_Ongoing)
		{



			if (Entity->Flags & F_IS_PLAYER)
			{
				//player update code per frame


				//Player Skill code

				if (Entity->DashingTimer > 0.0f)
				{
					Entity->DashingTimer -= DeltaTime;
				}
				else
				{
					Entity->Dashing = false;
				}

				if (Entity->DashCharges != Entity->MaxDashCharges && Entity->DashCooldown > 0.0f)
				{
					Entity->DashCooldown -= DeltaTime;
				}
				else
				{
					if (Entity->DashCharges < Entity->MaxDashCharges)
					{
						Entity->DashCharges += 1;
						Entity->DashCooldown = Entity->DashCooldownOnUse;
					}
				}

				if (Input->LeftShoulder.EndedDown && Input->LeftShoulder.HalfTransitions == 1)
				{

					if (!Entity->Dashing && Entity->DashCharges > 0)
					{
						Entity->DashCharges -= 1;

						//dash now
						Entity->Dashing = true;
						Entity->DashingTimer = BASE_PLAYER_DASH_TIME;
						Entity->FixedDashDirection = Entity->Direction;
					}


				}


				if (Entity->BurstingTimer > 0.0f)
				{
					Entity->BurstingTimer -= DeltaTime;
				}
				else
				{
					Entity->CanBurst = true;
				}


				if (Entity->BurstCharges != Entity->MaxBurstCharges && Entity->BurstCooldown > 0.0f)
				{
					Entity->BurstCooldown -= DeltaTime;
				}
				else
				{
					if (Entity->BurstCharges < Entity->MaxBurstCharges)
					{
						Entity->BurstCharges += 1;
						Entity->BurstCooldown = Entity->BurstCooldownOnUse;
					}
				}



				//update weapon stuff, timers
				if (Entity->WeaponCooldown > 0.0f)
				{
					Entity->WeaponCooldown -= DeltaTime;

					if (Entity->WeaponCooldown < 0.0f)
					{
						Entity->WeaponCooldown = 0.0f;
					}
				}



				//Player Movement

				if (Entity->Dashing)
				{
					Entity->Position = Entity->Position + ((PLAYER_DASH_SPEED_BOOST * Entity->Speed * DeltaTime) * Entity->FixedDashDirection);
				}


				if (Input->LeftStickX != 0.0f || Input->LeftStickY != 0.0f)
				{

					if (!Entity->Dashing)
					{
						Entity->Direction = Normalize(vec3D{ Input->LeftStickX, -Input->LeftStickY, 0.0f });
						Entity->Position = Entity->Position + ((Entity->Speed * DeltaTime) * Entity->Direction);
					}
				}


				real32 AimThreshold = 0.0f;

				if (Input->ControllerDetected)
				{
					AimThreshold = 0.4f;
				}

				if (Input->RightStickX <= -AimThreshold || Input->RightStickX >= AimThreshold || Input->RightStickY <= -AimThreshold || Input->RightStickY >= AimThreshold)
				{
					//Have player point towards the direction of the right stick, have bullet spawn point point in that direction as well.

					vec3D Direction = { Input->RightStickX, -Input->RightStickY, 0.0f };
					Direction = Normalize(Direction);

					real32 RadiansBetween = AngleBetween(Direction, vec3D{ 0.0f,-1.0f,0.0f });

					//float RotateRadians = F_PI / 32.0f;

					if (Input->RightStickX < 0.0f)
					{
						mat4x4 Rotate = RotateZMat4(-RadiansBetween);
						Entity->Rotation = Rotate;
					}
					else
					{
						mat4x4 Rotate = RotateZMat4(RadiansBetween);
						Entity->Rotation = Rotate;
					}


					Entity->BulletSpawnPoint = Direction;

					//Entity->Direction = Direction;

				}



				//burst attack logic
				if (Input->RightShoulder.EndedDown && Entity->BurstCharges > 0)
				{

					if (Entity->CanBurst)
					{

						Entity->BurstCharges -= 1;

						//launch out split bullets in aoe now
						vec3D FiringDirection = Entity->BulletSpawnPoint;
						real32 RandomDegreeVariance = 75.0f;
						FiringDirection = RotateZMat4(TO_RADIANS(GetRandomReal(-RandomDegreeVariance, RandomDegreeVariance, &GameState->PRNG))) * FiringDirection;
						real32 Rotation = 0.0f;
						real32 RotationAmount = (2 * F_PI) / (real32)Entity->BurstShotCount;
						for (int32 j = 0; j < Entity->BurstShotCount; j++)
						{
							game_entity* SpawnedBullet = AddEntity(Memory, GameState, (F_IS_BULLET | F_PLAYER_OWNED));

							SpawnedBullet->Color = uint32ColorfromRGB(21, 150, 225);
							SpawnedBullet->Position = Entity->Position;
							SpawnedBullet->Speed = Entity->Speed;
							SpawnedBullet->LifeTime = BASE_BULLET_LIFETIME / 3.65f;
							SpawnedBullet->Radius = PLAYER_BASE_BULLET_RADIUS;
							SpawnedBullet->Damage = Entity->Damage;
							SpawnedBullet->Direction = RotateZMat4(Rotation) * FiringDirection;
							SpawnedBullet->IsSplitBullet = true;
							Rotation += RotationAmount;
						}

						Entity->CanBurst = false;
						Entity->BurstingTimer = PLAYER_TIME_BETWEEN_BURSTS;

					}


				}





				//PLAYER WEAPON FIRING
				if (Input->RightShoulder.EndedDown && Entity->WeaponCooldown <= 0.0f)
				{
					game_entity* SpawnedBullet = AddEntity(Memory, GameState, (F_IS_BULLET | F_PLAYER_OWNED));

					SpawnedBullet->Color = uint32ColorfromRGB(21, 225, 50);
					SpawnedBullet->Position = Entity->Position + Entity->BulletSpawnPoint * PLAYER_BASE_SCALE;
					SpawnedBullet->Speed = Entity->Speed * BULLET_MOVEMENT_SCALE;
					SpawnedBullet->LifeTime = BASE_BULLET_LIFETIME;
					SpawnedBullet->Radius = PLAYER_BASE_BULLET_RADIUS;
					SpawnedBullet->Damage = Entity->Damage;

					if (Entity->BulletSpread)
					{
						real32 Variance = TO_RADIANS(Entity->BulletSpreadVariance);

						Variance = GetRandomReal(-Variance, Variance, &GameState->PRNG);
						mat4x4 RandomRotation = RotateZMat4(Variance);
						SpawnedBullet->Direction = RandomRotation * Entity->BulletSpawnPoint;
					}
					else
					{
						SpawnedBullet->Direction = Entity->BulletSpawnPoint;
					}


					Entity->WeaponCooldown = Entity->FireRate;



				}

				if (Entity->Health <= 0)
				{
					//Player has died, show YOU DIED, then return to start screen after a delay?
					//SetupStartScreen(Memory, GameState);
					GameState->GameEndType = GES_Player_Died;
					Entity->Health = 0;

					GameState->ScreenText = AddEntity(Memory, GameState, (F_IS_STRING));
					GameState->ScreenText->Position = { 650.0f, 200.0f,0.0f };
					GameState->ScreenText->Scale = ScaleMat4(2.0f, 2.0f, 0.0f);
					GameState->ScreenText->Color = uint32ColorfromRGB(205, 15, 15);
					StringCopy(GameState->ScreenText->StringMessage, 10, "YOU DIED!");


					GameState->ExitButton = AddEntity(Memory, GameState, (F_IS_BUTTON | F_IS_EXIT_BUTTON));
				}


			}


			if (Entity->Flags & F_IS_BULLET)
			{
				Entity->Position = Entity->Position + ((Entity->Speed * DeltaTime) * Entity->Direction);

				Entity->LifeTime -= DeltaTime;
				if (Entity->LifeTime < 0.0f)
				{
					if (Entity->IsSplitBullet)
					{
						real32 SplitDegree = 12.0f;
						Entity->Direction = RotateZMat4(TO_RADIANS(-SplitDegree)) * Entity->Direction;

						game_entity* SpawnedBullet = AddEntity(Memory, GameState, (F_IS_BULLET | F_PLAYER_OWNED));

						SpawnedBullet->Color = uint32ColorfromRGB(21, 225, 50);
						SpawnedBullet->Position = Entity->Position;
						SpawnedBullet->Speed = Entity->Speed * (BULLET_MOVEMENT_SCALE + 0.33f);
						SpawnedBullet->LifeTime = BASE_BULLET_LIFETIME;
						SpawnedBullet->Radius = PLAYER_BASE_BULLET_RADIUS;
						SpawnedBullet->Damage = Entity->Damage;
						SpawnedBullet->Direction = Entity->Direction;

						Entity->Direction = RotateZMat4(TO_RADIANS(2.0f * SplitDegree)) * Entity->Direction;

						SpawnedBullet = AddEntity(Memory, GameState, (F_IS_BULLET | F_PLAYER_OWNED));

						SpawnedBullet->Color = uint32ColorfromRGB(21, 225, 50);
						SpawnedBullet->Position = Entity->Position;
						SpawnedBullet->Speed = Entity->Speed * (BULLET_MOVEMENT_SCALE + 0.33f);
						SpawnedBullet->LifeTime = BASE_BULLET_LIFETIME;
						SpawnedBullet->Radius = PLAYER_BASE_BULLET_RADIUS;
						SpawnedBullet->Damage = Entity->Damage;
						SpawnedBullet->Direction = Entity->Direction;
					}

					MarkEntityDeleted(&Memory->EntityArena, Entity);
				}
			}


			if (Entity->Flags & F_IS_ENEMY)
			{
				/*
					I want to have each enemy to have different potential behaviors, such as flanking, orbitint, fleeing-to-edge, and sporatic, get creative with it
				*/



				//ENEMY MOVEMENT

				switch (Entity->EnemyBehavior)
				{
				case EB_Stationary:
				{

					break;
				}

				case EB_Random:
				{
					Entity->EnemyMovementCooldown -= DeltaTime;
					if (Entity->EnemyMovementCooldown <= 0.0f)
					{
						real32 RandomRotation = GetRandomReal(0.0f, 2 * F_PI, &GameState->PRNG);
						Entity->Direction = (RotateZMat4(RandomRotation) * vec3D { 0.0f, 1.0f, 0.0 });

						Entity->EnemyMovementCooldown = ENEMY_MOVE_TIMER;
					}

					Entity->Position = Entity->Position + ((Entity->Speed * DeltaTime) * Entity->Direction);

					break;
				}
				case EB_Circling:
				{
					//uses Bullet spawn point as Circling point.
					vec3D ToOrigin = { -Entity->BulletSpawnPoint.x, -Entity->BulletSpawnPoint.y, 0.0f };

					vec3D RotationVector = Entity->Position + ToOrigin;

					if (Entity->FlippedCircling)
					{
						RotationVector = RotateZMat4(-ENEMY_CIRCLING_SPEED * Entity->Speed * DeltaTime) * RotationVector;

					}
					else
					{
						RotationVector = RotateZMat4(ENEMY_CIRCLING_SPEED * Entity->Speed * DeltaTime) * RotationVector;

					}

					RotationVector = RotationVector - ToOrigin;
					Entity->Position = RotationVector;
					break;
				}

				case EB_Pursuit:
				{
					Entity->Direction = Normalize(GameState->Player->Position - Entity->Position);

					Entity->Position = Entity->Position + ((Entity->Speed * DeltaTime) * Entity->Direction);

					break;
				}
				case EB_Afraid:
				{
					real32 DistanceSquaredFromPlayer = DistanceSquared(GameState->Player->Position, Entity->Position);
					if (DistanceSquaredFromPlayer < ENEMY_AFRAID_DISTANCE)
					{
						Entity->Direction = Normalize(Entity->Position - GameState->Player->Position);
						Entity->Position = Entity->Position + ((Entity->Speed * DeltaTime) * Entity->Direction);
					}
					break;
				}

				default:
				{
					break;
				}
				}







				//Enemy WEAPON FIRING
							//update weapon stuff, timers
				if (Entity->WeaponCooldown > 0.0f)
				{
					Entity->WeaponCooldown -= DeltaTime;

					if (Entity->WeaponCooldown < 0.0f)
					{
						Entity->WeaponCooldown = 0.0f;
					}
				}

				if (Entity->WeaponCooldown <= 0.0f)
				{


					switch (Entity->EnemyWeapon)
					{
					case EW_Single:
					{
						game_entity* SpawnedBullet = AddEntity(Memory, GameState, (F_IS_BULLET | F_ENEMY_OWNED));

						SpawnedBullet->Color = uint32ColorfromRGB(225, 21, 50);
						SpawnedBullet->Position = Entity->Position;
						SpawnedBullet->Speed = Entity->Speed * ENEMY_BASE_BULLET_SPEED_MODIFIER;
						SpawnedBullet->LifeTime = ENEMY_BASE_BULLET_LIFETIME;
						SpawnedBullet->Radius = ENEMY_BASE_BULLET_RADIUS;
						SpawnedBullet->Damage = Entity->Damage;
						SpawnedBullet->Direction = Normalize(GameState->Player->Position - Entity->Position);
						break;
					}

					case EW_Double:
					{
						for (int32 j = 0; j < 2; j++)
						{
							game_entity* SpawnedBullet = AddEntity(Memory, GameState, (F_IS_BULLET | F_ENEMY_OWNED));

							SpawnedBullet->Color = uint32ColorfromRGB(225, 21, 50);
							SpawnedBullet->Position = Entity->Position;
							SpawnedBullet->Speed = Entity->Speed * ENEMY_BASE_BULLET_SPEED_MODIFIER;
							SpawnedBullet->LifeTime = ENEMY_BASE_BULLET_LIFETIME * 0.6f;
							SpawnedBullet->Radius = ENEMY_BASE_BULLET_RADIUS + 2;
							SpawnedBullet->Damage = Entity->Damage;
							SpawnedBullet->Direction = Normalize(GameState->Player->Position - Entity->Position);

							real32 Variance = TO_RADIANS(Entity->BulletSpreadVariance);

							Variance = GetRandomReal(-Variance, Variance, &GameState->PRNG);
							mat4x4 RandomRotation = RotateZMat4(Variance);
							SpawnedBullet->Direction = RandomRotation * SpawnedBullet->Direction;
						}

						break;
					}
					case EW_Burst:
					{
						Entity->BurstShotCount--;

						if (Entity->BurstShotCount < 0)
						{
							Entity->BurstShotCount = ENEMY_BURST_SHOT_AMOUNT;
							Entity->FireRate = ENEMY_BURST_COOLDOWN;
						}
						else
						{
							game_entity* SpawnedBullet = AddEntity(Memory, GameState, (F_IS_BULLET | F_ENEMY_OWNED));

							SpawnedBullet->Color = uint32ColorfromRGB(225, 21, 50);
							SpawnedBullet->Position = Entity->Position;
							SpawnedBullet->Speed = Entity->Speed * ENEMY_BURST_SHOT_SPEED;
							SpawnedBullet->LifeTime = ENEMY_BASE_BULLET_LIFETIME * 0.5f;
							SpawnedBullet->Radius = ENEMY_BASE_BULLET_RADIUS - 1;
							SpawnedBullet->Damage = Entity->Damage;
							SpawnedBullet->Direction = Normalize(GameState->Player->Position - Entity->Position);


							real32 Variance = TO_RADIANS(Entity->BulletSpreadVariance);

							Variance = GetRandomReal(-Variance, Variance, &GameState->PRNG);
							mat4x4 RandomRotation = RotateZMat4(Variance);
							SpawnedBullet->Direction = RandomRotation * SpawnedBullet->Direction;

							Entity->FireRate = ENEMY_BURST_FIRERATE;
						}

						break;
					}

					case EW_Triple:
					{
						for (int32 j = 0; j < 3; j++)
						{
							game_entity* SpawnedBullet = AddEntity(Memory, GameState, (F_IS_BULLET | F_ENEMY_OWNED));

							SpawnedBullet->Color = uint32ColorfromRGB(225, 21, 50);
							SpawnedBullet->Position = Entity->Position;
							SpawnedBullet->Speed = Entity->Speed * ENEMY_BASE_BULLET_SPEED_MODIFIER * 0.75f;
							SpawnedBullet->LifeTime = ENEMY_BASE_BULLET_LIFETIME * 0.45f;
							SpawnedBullet->Radius = ENEMY_BASE_BULLET_RADIUS + 4;
							SpawnedBullet->Damage = Entity->Damage;
							SpawnedBullet->Direction = Normalize(GameState->Player->Position - Entity->Position);

							real32 Variance = TO_RADIANS(Entity->BulletSpreadVariance);

							Variance = GetRandomReal(-Variance, Variance, &GameState->PRNG);
							mat4x4 RandomRotation = RotateZMat4(Variance);
							SpawnedBullet->Direction = RandomRotation * SpawnedBullet->Direction;
						}
						break;
					}
					case EW_Mega:
					{
						game_entity* SpawnedBullet = AddEntity(Memory, GameState, (F_IS_BULLET | F_ENEMY_OWNED));

						SpawnedBullet->Color = uint32ColorfromRGB(225, 21, 50);
						SpawnedBullet->Position = Entity->Position;
						SpawnedBullet->Speed = Entity->Speed * ENEMY_BASE_BULLET_SPEED_MODIFIER * 0.5f;
						SpawnedBullet->LifeTime = ENEMY_BASE_BULLET_LIFETIME * 2.0f;
						SpawnedBullet->Radius = ENEMY_BASE_BULLET_RADIUS * 8;
						SpawnedBullet->Damage = Entity->Damage;
						SpawnedBullet->Direction = Normalize(GameState->Player->Position - Entity->Position);

						real32 Variance = TO_RADIANS(Entity->BulletSpreadVariance);

						Variance = GetRandomReal(-Variance, Variance, &GameState->PRNG);
						mat4x4 RandomRotation = RotateZMat4(Variance);
						SpawnedBullet->Direction = RandomRotation * SpawnedBullet->Direction;
						break;
					}
					case EW_AOE:
					{
						vec3D FiringDirection = { 0.0f, 1.0f, 0.0f };
						real32 Rotation = 0.0f;
						real32 RotationAmount = (2 * F_PI) / (real32)ENEMY_AOE_SHOT_AMOUNT;
						for (int32 j = 0; j < ENEMY_AOE_SHOT_AMOUNT; j++)
						{
							game_entity* SpawnedBullet = AddEntity(Memory, GameState, (F_IS_BULLET | F_ENEMY_OWNED));

							SpawnedBullet->Color = uint32ColorfromRGB(225, 21, 50);
							SpawnedBullet->Position = Entity->Position;
							SpawnedBullet->Speed = Entity->Speed * ENEMY_BASE_BULLET_SPEED_MODIFIER * 0.65f;
							SpawnedBullet->LifeTime = ENEMY_BASE_BULLET_LIFETIME * 0.38f;
							SpawnedBullet->Radius = ENEMY_BASE_BULLET_RADIUS;
							SpawnedBullet->Damage = Entity->Damage;
							SpawnedBullet->Direction = RotateZMat4(Rotation) * FiringDirection;
							Rotation += RotationAmount;
						}

						break;
					}

					default:
					{
						break;
					}
					}

					Entity->WeaponCooldown = Entity->FireRate;

					/*
									if (Entity->BulletSpread)
					{
						real32 Variance = RADIANS(Entity->BulletSpreadVariance);

						Variance = GetRandomReal(-Variance, Variance, &GameState->PRNG);
						mat4x4 RandomRotation = RotateZMat4(Variance);
						SpawnedBullet->Direction = RandomRotation * Entity->BulletSpawnPoint;
					}
					else
					{
						SpawnedBullet->Direction = Entity->BulletSpawnPoint;
					}

					*/









				}



				if (Entity->Health <= 0)
				{
					//Enemy below health threshold, it is dead
					MarkEntityDeleted(&Memory->EntityArena, Entity);
					game_entity* DeathRing = AddEntity(Memory, GameState, (F_IS_DEATH_RING | F_ENEMY_OWNED));
					DeathRing->Position = Entity->Position;

					Memory->PlatformSubmitAudioToPlay(&Memory->SoundEffects[GSE_Entity_Death]);
				}



			}

			if (Entity->Flags & F_IS_BOSS)
			{
				real32 Phase2Health = ((real32)BOSS_HEALTH * 0.75f);
				real32 Phase3Health = ((real32)BOSS_HEALTH * 0.35f);

				if ((real32)Entity->Health <= Phase2Health)
				{
					Entity->EnemyBehavior = BP_Enemy_Spawn;
				}

				if ((real32)Entity->Health <= Phase3Health)
				{
					Entity->EnemyBehavior = BP_Danmaku;
				}



				//BOSS MOVEMENT LOGIC
				real32 MovementScreenWidth = BitmapBuffer->Width / 2.25f;
				real32 MovementScreenHeight = BitmapBuffer->Height / 2.25f;
				real32 HalfScreenX = (BitmapBuffer->Width / 2.0f);
				real32 HalfScreenY = (BitmapBuffer->Height / 2.0f);

				Entity->EnemyMovementCooldown -= DeltaTime;
				if (Entity->EnemyMovementCooldown <= 0.0f)
				{
					//generate new location to move to, within the bounds
					real32 MoveXMin = HalfScreenX - MovementScreenWidth;
					real32 MoveXMax = HalfScreenX + MovementScreenWidth;
					real32 MoveYMin = HalfScreenY - MovementScreenHeight;
					real32 MoveYMax = HalfScreenY - 50.0f;

					real32 MoveX = GetRandomReal(MoveXMin, MoveXMax, &GameState->PRNG);
					real32 MoveY = GetRandomReal(MoveYMin, MoveYMax, &GameState->PRNG);

					vec3D MoveLocation = { MoveX, MoveY, 0.0f };
					real32 Speedup = 1.05f;

					if (Entity->Health > 0)
					{
						real32 SpeedupRatio = (real32)BOSS_HEALTH / (real32)Entity->Health;
						real32 MinSpeedup = 1.05f;
						Speedup = log2f(SpeedupRatio + MinSpeedup);

					}

					Entity->EnemyMovementCooldown = BOSS_MOVEMENT_COOLDOWN / Speedup;


					Entity->Direction = Normalize(MoveLocation - Entity->Position);
					Entity->Speed = Distance(MoveLocation, Entity->Position) / Entity->EnemyMovementCooldown;

				}

				Entity->Position = Entity->Position + ((Entity->Speed * DeltaTime) * Entity->Direction);


				//Have the boss rotated to look at the player
				vec3D TowardsPlayer = Normalize(GameState->Player->Position - Entity->Position);
				real32 RadiansBetween = AngleBetween(TowardsPlayer, vec3D{ 0.0f,-1.0f,0.0f });

				if (TowardsPlayer.x < 0.0f)
				{
					Entity->Rotation = RotateZMat4(-RadiansBetween);
				}
				else
				{
					Entity->Rotation = RotateZMat4(RadiansBetween);
				}

				//bullet direction
				Entity->BulletSpawnPoint = TowardsPlayer;



				//BOSS ATTACK LOGIC

				if (Entity->WeaponCooldown > 0.0f)
				{
					Entity->WeaponCooldown -= DeltaTime;

					if (Entity->WeaponCooldown <= 0.0f)
					{
						Entity->BossGunTimer = BOSS_BURST_FIRE_COOLDOWN / 2.75f;
					}
				}

				if (Entity->BossGunTimer > 0.0f)
				{
					Entity->BossGunTimer -= DeltaTime;

					//boss firing
					game_entity* SpawnedBullet = AddEntity(Memory, GameState, (F_IS_BULLET | F_ENEMY_OWNED));

					SpawnedBullet->Color = uint32ColorfromRGB(21, 50, 225);


					real32 MinimumBulletSpeed = 195.0f;
					real32 MaximumBulletSpeed = 700.0f;
					if (Entity->Speed < MinimumBulletSpeed)
					{
						SpawnedBullet->Speed = MinimumBulletSpeed;
					}
					else if (Entity->Speed > MaximumBulletSpeed)
					{
						SpawnedBullet->Speed = MaximumBulletSpeed;
					}
					else
					{
						SpawnedBullet->Speed = Entity->Speed;
					}

					SpawnedBullet->LifeTime = 4.0f;
					SpawnedBullet->Radius = ENEMY_BASE_BULLET_RADIUS + 1;
					SpawnedBullet->Damage = Entity->Damage;
					SpawnedBullet->Direction = Normalize(GameState->Player->Position - Entity->Position);

					real32 Variance = TO_RADIANS(Entity->BulletSpreadVariance);

					Variance = GetRandomReal(-Variance, Variance, &GameState->PRNG);
					mat4x4 RandomRotation = RotateZMat4(Variance);
					SpawnedBullet->Direction = RandomRotation * SpawnedBullet->Direction;

					SpawnedBullet->Position = Entity->Position + (SpawnedBullet->Direction * (BOSS_SCALE + 9.0f));

					if (Entity->BossGunTimer <= 0.0f)
					{
						Entity->WeaponCooldown = BOSS_BURST_FIRE_COOLDOWN;
					}
				}



				if (Entity->EnemyBehavior >= BP_Enemy_Spawn)
				{
					Entity->BossEnemySpawnTimer -= DeltaTime;
					if (Entity->BossEnemySpawnTimer <= 0.0f)
					{
						int32 RandomWeapon = GetRandomInt(0, 5, &GameState->PRNG);
						int32 RandomBehavior = GetRandomInt(0, 4, &GameState->PRNG);

						game_entity* NewEnemy = CreateEnemy(Memory, GameState, Entity->Position.x - (2.0f * Entity->Scale.a0), Entity->Position.y, RandomBehavior, RandomWeapon);
						NewEnemy->Health = 10;

						NewEnemy = CreateEnemy(Memory, GameState, Entity->Position.x + (2.0f * Entity->Scale.a0), Entity->Position.y, RandomBehavior, RandomWeapon);
						NewEnemy->Health = ENEMY_BASE_HEALTH * 7;

						Entity->BossEnemySpawnTimer = BOSS_ENEMY_SPAWN_RATE;
					}
				}

				if (Entity->EnemyBehavior >= BP_Danmaku)
				{
					if (Entity->DanmakuRotation < BOSS_MAX_DANMAKU_SPEED)
					{
						Entity->DanmakuRotation += DeltaTime;
					}

					Entity->BossDanmakuDirection = RotateZMat4(Entity->DanmakuRotation * BOSS_DANMAKU_SPEED_SCALING * DeltaTime) * Entity->BossDanmakuDirection;

					real32 RotationPerStream = (2.0f * F_PI) / BOSS_DANMAKU_ATTACK_COUNT;

					for (int32 k = 0; k < BOSS_DANMAKU_ATTACK_COUNT; k++)
					{
						game_entity* SpawnedBullet = AddEntity(Memory, GameState, (F_IS_BULLET | F_ENEMY_OWNED));

						SpawnedBullet->Color = uint32ColorfromRGB(215, 160, 250);

						SpawnedBullet->LifeTime = 0.5f;
						SpawnedBullet->Speed = 500.0f;
						SpawnedBullet->Radius = ENEMY_BASE_BULLET_RADIUS + 2;
						SpawnedBullet->Damage = Entity->Damage;
						SpawnedBullet->Direction = Entity->BossDanmakuDirection;
						SpawnedBullet->Position = Entity->Position + (Entity->BossDanmakuDirection * (BOSS_SCALE + 7.0f));

						Entity->BossDanmakuDirection = RotateZMat4(RotationPerStream) * Entity->BossDanmakuDirection;
					}

				}


				if (Entity->Health <= 0)
				{
					//boss defeated, show you win? reset game to start menu after

					GameState->FightingBoss = false;
					GameState->GameEndType = GES_Boss_Defeated;

					GameState->ScreenText = AddEntity(Memory, GameState, (F_IS_STRING));
					GameState->ScreenText->Position = { 550.0f, 250.0f,0.0f };
					GameState->ScreenText->Scale = ScaleMat4(2.5f, 2.5f, 0.0f);
					GameState->ScreenText->Color = uint32ColorfromRGB(55, 254, 55);
					StringCopy(GameState->ScreenText->StringMessage, 11, "YOU WIN!!!");




					GameState->ExitButton = AddEntity(Memory, GameState, (F_IS_BUTTON | F_IS_EXIT_BUTTON));
					//SetupStartScreen(Memory, GameState);

				}

			}




			//screen border logic
			if (Entity->Flags & F_IS_BULLET)
			{
				int32 XPos = (int32)Entity->Position.x;
				int32 YPos = (int32)Entity->Position.y;

				if (XPos > BitmapBuffer->Width || XPos < 0 || YPos > BitmapBuffer->Height || YPos < 0)
				{
					MarkEntityDeleted(&Memory->EntityArena, Entity);
				}

			}

			if (Entity->Flags & (F_IS_UI_BAR | F_BOSS_OWNED))
			{

				real32 WidthRatio = (real32)GameState->Boss->Health / (real32)BOSS_HEALTH;
				real32 NewWidth = (real32)BOSS_HP_BAR_WIDTH * WidthRatio;
				Entity->Position.x = 480.0f + (NewWidth / 2.0f);
				Entity->Scale = ScaleMat4(NewWidth, BOSS_HP_BAR_HEIGHT, 0.0f);

			}

			if (Entity->Flags & F_IS_PLAYER_HEALTH)
			{
				Entity->Health = GameState->Player->Health;
			}

			if (Entity->Flags & F_IS_PLAYER_GUN_BAR)
			{
				Entity->WeaponCooldown = GameState->Player->WeaponCooldown;
			}

			if (Entity->Flags & F_IS_PLAYER_DASH_COUNTER)
			{
				AddNumberToEntityString(Entity, GameState->Player->DashCharges, 6);
			}

			if (Entity->Flags & F_IS_PLAYER_BURST_COUNTER)
			{
				AddNumberToEntityString(Entity, GameState->Player->BurstCharges, 7);
			}

			if (Entity->Flags & F_IS_WAVE_COUNTER)
			{
				AddNumberToEntityString(Entity, GameState->CurrentWave, 6);

			}

			if (Entity->Flags & F_IS_DEATH_RING)
			{


				if (Entity->Flags & F_BULLET_OWNED)
				{
					Entity->DeathRingExpansion += (DeltaTime * DEATH_RING_EXPANSION_SPEED * 0.3f);
					Entity->LifeTime -= (DeltaTime * 2.0f);
				}
				else if (Entity->Flags & F_BOSS_OWNED)
				{
					Entity->DeathRingExpansion += (DeltaTime * DEATH_RING_EXPANSION_SPEED * 3.0f);
					Entity->LifeTime -= (DeltaTime * 0.65f);

				}
				else
				{
					Entity->DeathRingExpansion += (DeltaTime * DEATH_RING_EXPANSION_SPEED);
					Entity->LifeTime -= DeltaTime;
				}

				Entity->Radius = (int32)Entity->DeathRingExpansion;

				if (Entity->LifeTime <= 0.0f)
				{
					MarkEntityDeleted(&Memory->EntityArena, Entity);
				}

			}


			if (Entity->Flags & F_IS_PLAYER || Entity->Flags & F_IS_ENEMY || Entity->Flags & F_IS_BOSS)
			{
				real32 HalfWidth = (Entity->Scale.a0 / 2.0f);
				real32 HalfHeight = (Entity->Scale.b1 / 2.0f);
				real32 LeftEdge = Entity->Position.x - HalfWidth;
				real32 RightEdge = Entity->Position.x + HalfWidth;
				real32 TopEdge = Entity->Position.y - HalfHeight;
				real32 BottomEdge = Entity->Position.y + HalfHeight;
				real32 BitmapWidth = (real32)BitmapBuffer->Width;
				real32 BitmapHeight = (real32)BitmapBuffer->Height;

				if (RightEdge > BitmapWidth)
				{
					Entity->Position.x = BitmapWidth - HalfWidth;
				}
				if (LeftEdge < 0.0f)
				{
					Entity->Position.x = HalfWidth;
				}

				if (TopEdge < 0.0f)
				{
					Entity->Position.y = HalfHeight;
				}

				if (BottomEdge > BitmapHeight)
				{
					Entity->Position.y = BitmapHeight - HalfHeight;
				}



			}

		}
		



	}



	//process collisions

	if (!GameState->GamePaused)
	{
		ProcessCollisions(Memory, GameState, &Memory->EntityArena, &Memory->ScratchArena);
	}





	//create sorted list of entities AGAIN
	GameState->SortedEntities = PopulateSortedEntitiesArray(GameState->SortedEntities, &Memory->EntityArena, &Memory->ScratchArena, &GameState->EntityCount, &GameState->FreeListCount);



	//draw entities
	GameState->EnemiesDrawn = false;
	for (int32 i = 0; i < GameState->EntityCount; i++)
	{
		game_entity* Entity = (game_entity*)GameState->SortedEntities[i];

		if ((GameState->EnemiesDrawn == false) && (Entity->Flags & F_IS_ENEMY))
		{
			int32 NumberOfEnemies = 1;
			int32 CurrentIndex = i;
			int32 FirstIndex = i;
			while (1)
			{
				CurrentIndex++;
				game_entity* CheckEntity = (game_entity*)GameState->SortedEntities[CurrentIndex];

				if (!(CheckEntity->Flags & F_IS_ENEMY & CheckEntity->Flags & F_TO_BE_DELETED))
				{
					break;
				}
				else
				{
					NumberOfEnemies++;
				}
			}

			game_enemy_draw_info* EnemyInfo = (game_enemy_draw_info*)MemoryAllocate(&Memory->ScratchArena, sizeof(game_enemy_draw_info) * NumberOfEnemies);

			int32 EnemyInfoIndex = 0;
			for (int32 EnemyIndex = FirstIndex; EnemyIndex < CurrentIndex; EnemyIndex++)
			{
				game_entity* SecondCheckEntity = (game_entity*)GameState->SortedEntities[EnemyIndex];

				EnemyInfo[EnemyInfoIndex++] = { ((int32)SecondCheckEntity->Position.x) % BitmapBuffer->Width, (int32)SecondCheckEntity->Position.y, SecondCheckEntity->Transparency };
			}

			i = (CurrentIndex - 1); //move i index to past enemies
			
			
			//FIRST ENEMY, BATCH DRAW THEM NOW

			DrawAllEnemies(BitmapBuffer, EnemyInfo, NumberOfEnemies, (int32)Entity->Scale.a0, (int32)Entity->Scale.b1, Entity->TextureBitMap);

			GameState->EnemiesDrawn = true;
		}
		else
		{
			//if not an enemy, draw everything else

			if (Entity->Flags & F_RENDERABLE)
			{

				if (Entity->Flags & F_DRAW_WITH_TRIANGLE_QUAD)
				{
					DrawQuad(BitmapBuffer, Entity);
				}

				if (Entity->Flags & F_DRAW_WITH_RECT)
				{
					//DrawRectangle(BitmapBuffer, (uint32)Entity->Position.x, (uint32)Entity->Position.y, Entity->Radius, Entity->Radius, Entity->Color);
					DrawRectangle(BitmapBuffer, (uint32)Entity->Position.x, (uint32)Entity->Position.y, (int32)Entity->Scale.a0, (int32)Entity->Scale.b1, Entity->TextureBitMap, Entity->Transparency);
				}

				if (Entity->Flags & F_DRAW_WITH_STRING)
				{
					DrawString(BitmapBuffer, &GameState->TextureHandles[Font_Sprite], Entity->Position.x, Entity->Position.y,
						Entity->Scale.a0, Entity->Scale.b1, Entity->StringMessage, Entity->Color, Entity->Transparency);
				}

				if (Entity->Flags & F_DRAW_WITH_FILLED_CIRCLE)
				{
					DrawCircle(BitmapBuffer, &Memory->ScratchArena, (int32)Entity->Position.x, (int32)Entity->Position.y, Entity->Radius, Entity->Color, true, 255);
				}

				if (Entity->Flags & F_DRAW_WITH_CIRCLE)
				{
					DrawCircle(BitmapBuffer, &Memory->ScratchArena, (int32)Entity->Position.x, (int32)Entity->Position.y, Entity->Radius, Entity->Color, false, Entity->Transparency);
				}

				if (Entity->Flags & F_DRAW_WITH_FILLED_RECT)
				{
					DrawFilledRectangle(BitmapBuffer, (int32)Entity->Position.x, (int32)Entity->Position.y, (int32)Entity->Scale.a0, (int32)Entity->Scale.b1, Entity->Color);
				}

				if (Entity->Flags & F_IS_PLAYER_HEALTH)
				{
					real32 XHealthOffset{};
					real32 XOffsetPerBar = (real32)PLAYER_HEALTH_DIMENSIONS + 5.0f;

					for (int32 k = 0; k < Entity->Health; k++)
					{
						DrawFilledRectangle(BitmapBuffer, (int32)(Entity->Position.x + XHealthOffset), (int32)Entity->Position.y, PLAYER_HEALTH_DIMENSIONS, PLAYER_HEALTH_DIMENSIONS, Entity->Color);
						XHealthOffset += XOffsetPerBar;
					}
				}

				if (Entity->Flags & F_IS_PLAYER_GUN_BAR)
				{
					real32 WidthRatio = GameState->Player->WeaponCooldown / GameState->Player->FireRate;
					real32 NewWidth = (real32)PLAYER_WEAPON_BAR_WIDTH * WidthRatio;
					Entity->Position.x = (UI_OFFSET_PIXELS / 2.0f) + (NewWidth / 2.0f);
					Entity->Scale = ScaleMat4(NewWidth, (real32)PLAYER_HEALTH_DIMENSIONS / 1.5f, 0.0f);
					DrawFilledRectangle(BitmapBuffer, (int32)Entity->Position.x, (int32)Entity->Position.y, (int32)Entity->Scale.a0, (int32)Entity->Scale.b1, Entity->Color);
				}


				if (Entity->Flags & F_IS_BUTTON)
				{
					int32 BackgroundColor = uint32ColorfromRGB(25,185,215);
					//draw background color first
					DrawFilledRectangle(BitmapBuffer, (int32)Entity->Position.x, (int32)Entity->Position.y, (int32)Entity->Scale.a0, (int32)Entity->Scale.b1, BackgroundColor);

					if (Entity->Flags & F_IS_START_BUTTON)
					{
						DrawString(BitmapBuffer, &GameState->TextureHandles[Font_Sprite], Entity->Position.x - START_BUTTON_TEXT_OFFSET, Entity->Position.y, BUTTON_TEXT_WIDTH, BUTTON_TEXT_HEIGHT,
							Entity->StringMessage, Entity->Color, FULL_OPACITY);
					}
					else if (Entity->Flags & F_IS_LOAD_BUTTON)
					{
						DrawString(BitmapBuffer, &GameState->TextureHandles[Font_Sprite], Entity->Position.x - LOAD_BUTTON_TEXT_OFFSET, Entity->Position.y, BUTTON_TEXT_WIDTH, BUTTON_TEXT_HEIGHT,
							Entity->StringMessage, Entity->Color, FULL_OPACITY);
					}
					else if (Entity->Flags & F_IS_EXIT_BUTTON)
					{
						DrawString(BitmapBuffer, &GameState->TextureHandles[Font_Sprite], Entity->Position.x - EXIT_BUTTON_TEXT_OFFSET, Entity->Position.y,
							BUTTON_TEXT_WIDTH, BUTTON_TEXT_HEIGHT, Entity->StringMessage, Entity->Color, FULL_OPACITY);
					}

				}


			}

		}


	}


	//check wave to advance

	if (!GameState->GamePaused && GameState->GameEndType == GES_Game_Ongoing)
	{
		if (GameState->GameMenuState == GMS_In_Game)
		{
			if (CheckIfWaveDefeated(&Memory->EntityArena) && !GameState->FightingBoss)
			{
				GameState->WaveSpawnTimer -= DeltaTime;
				if (GameState->WaveSpawnTimer <= 0.0f)
				{
					AdvanceWave(Memory, GameState, BitmapBuffer);
					GameState->WaveSpawnTimer = TIME_BETWEEN_WAVES;
				}
			}
		}
	}








	//DrawString(BitmapBuffer, &GameState->Font->TextureBitMap, 100.0f, 600.0f, 0.44f, 0.44f, FONT_TEST_STRING, 0x03FF11, 255);
	//DrawCircle(BitmapBuffer, &Memory->ScratchArena, 600, 600, TESTCIRCLERADIUS, uint32ColorfromRGB(200, 0, 0), false, 55);
	//DrawString(BitmapBuffer, &GameState->Font->TextureBitMap, 600.0f, 600.0f, 0.75, 0.75f, "-_- you got me     !!!...!!!", 0x11FFA0, 215);


	//uint32 PixelToDraw = uint32ColorfromRGB(Red, Green, Blue);


	//last thing done is updating mouse position
	GameState->LastMouseX = Input->MouseXPosition;
	GameState->LastMouseY = Input->MouseYPosition;

}

