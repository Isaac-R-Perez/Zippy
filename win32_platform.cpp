#include <windows.h>
#include <Xinput.h>
#include <xaudio2.h>
#include <stdio.h>

#include "IGame.h"
#include "win32_platform.h"



//GLOBALS
global_variable bool32 RUNNING{true};
global_variable int64 GlobalPerformanceCountFrequency;
global_variable IXAudio2* XAudioInterface;
global_variable IXAudio2MasteringVoice* XAudioMasteringVoice;


internal void Win32ProcessButton(game_button* OldButton, game_button* NewButton, bool32 IsDown)
{
	NewButton->EndedDown = IsDown;
	NewButton->HalfTransitions = ((OldButton->EndedDown != NewButton->EndedDown) ? 1 : 0);

}

internal void Win32ProcessControllerInput(game_button* OldButton, game_button* NewButton, int32 XInputButtonsState, int32 ButtonBit)
{
	NewButton->EndedDown = ((XInputButtonsState & ButtonBit) == ButtonBit);
	NewButton->HalfTransitions = ((OldButton->EndedDown != NewButton->EndedDown) ? 1 : 0);
}

internal real32 Win32ProcessXInputStick(int16 Value, int16 DeadZoneThreshold)
{
	real32 Result{};

	//Result will be a value from 0 to 1, where 0 is first valid input after removing deadzone noise
	//Change deadzone logic here to account for circular deadzone? (code above) IDK if the deadzone is actually circular
	if (Value < -DeadZoneThreshold)
	{
		Result = (real32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
	}
	else if (Value > DeadZoneThreshold)
	{
		Result = (real32)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
	}

	return Result;
}

inline void Win32GetWindowDimension(HWND Window, win32_window_dimensions* ClientWindow)
{ 

	RECT ClientRect{};

	GetClientRect(Window, &ClientRect);

	ClientWindow->Width = ClientRect.right - ClientRect.left;
	ClientWindow->Height = ClientRect.bottom - ClientRect.top;

}

internal void Win32LoadXInput()
{


	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");

	if (!XInputLibrary)
	{
		XInputLibrary = LoadLibraryA("xinput1_3.dll");
	}

	if (!XInputLibrary)
	{
		XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
	}

	if (!XInputLibrary)
	{
		//error XInput not found on machine, controller not supported
	}

}


internal void UnloadGameCode(win32_game_code* GameCode)
{
	if (GameCode->GameCodeDLL)
	{
		FreeLibrary(GameCode->GameCodeDLL);
		GameCode->GameCodeDLL = 0;
	}

	GameCode->IsValid = false;
	GameCode->GameUpdateAndRender = GameUpdateAndRenderSTUB;
}


inline FILETIME Win32GetLastWriteTime(char* FileName)
{
	FILETIME LastWriteTime{};

	WIN32_FILE_ATTRIBUTE_DATA Data;

	if (GetFileAttributesEx(FileName, GetFileExInfoStandard, &Data))
	{
		LastWriteTime = Data.ftLastWriteTime;
	}
	return LastWriteTime;
}



internal win32_game_code LoadGameCode(char* SourceDLLName, char* TempDLLName)
{
	win32_game_code Result{};

	Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
	CopyFile(SourceDLLName, TempDLLName, FALSE);
	Result.GameCodeDLL = LoadLibraryA(TempDLLName);

	if (Result.GameCodeDLL)
	{
		Result.GameUpdateAndRender = (void (*)(game_memory*, game_input*, game_bitmap_buffer*, real32))GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");

		//AND function pointers together to check if you got all of them
		Result.IsValid = (Result.GameUpdateAndRender != 0);
	}

	if (!Result.IsValid)
	{
		Result.GameUpdateAndRender = GameUpdateAndRenderSTUB;
	}

	return Result;
}


bool32 Win32WriteEntireFile(char* FileName, uint64 MemorySize, void* Memory)
{
	bool32 Result = false;

	HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD BytesWritten;

		if (WriteFile(FileHandle, Memory, (DWORD)MemorySize, &BytesWritten, 0))
		{
			Result = (BytesWritten == MemorySize);
		}
		else
		{
			//error
		}
		CloseHandle(FileHandle);
	}

	return Result;
}


void Win32FreeFileMemory(void* Memory)
{
	if (Memory)
	{
		//CHANGE THIS TO ARENA FREE LATER!!!
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
}

read_file_result Win32ReadEntireFile(char* FileName)
{
	read_file_result Result{};

	HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;

		if (GetFileSizeEx(FileHandle, &FileSize)) 
		{
			uint32 FileSize32 = (uint32)FileSize.QuadPart;

			//CHANGE THIS TO AN ARENA ALLOCATION LATER!!!
			Result.Contents = VirtualAlloc(0, FileSize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			if (Result.Contents)
			{
				DWORD BytesRead;
				if (ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && (FileSize32 == BytesRead))
				{
					//successful file read
					Result.ContentsSize = FileSize32;
				}
				else
				{
					Win32FreeFileMemory(Result.Contents);
					Result.Contents = 0;
				}
			}
		}

		CloseHandle(FileHandle);
	}

	return Result;
}

//-1 is LeftFile YOUNGER | 0 is error, either left or right doesn't exist | 1 is RightFile is YOUNGER
int32 Win32CompareFileTimes(char* FileLeft, char* FileRight)
{
	FILETIME FileLeftTime = Win32GetLastWriteTime(FileLeft);
	FILETIME FileRightTime = Win32GetLastWriteTime(FileRight);

	if (FileLeftTime.dwLowDateTime == 0 && FileLeftTime.dwHighDateTime == 0)
	{
		return 0;
	}

	if (FileRightTime.dwLowDateTime == 0 && FileRightTime.dwHighDateTime == 0)
	{
		return 0;
	}

	SYSTEMTIME FileLeftSystemTime{};
	FileTimeToSystemTime(&FileLeftTime, &FileLeftSystemTime);

	SYSTEMTIME FileRightSystemTime{};
	FileTimeToSystemTime(&FileRightTime, &FileRightSystemTime);



	return 0;
}

bool32 Win32WriteToFile(char* FileName, int32 FileOffsetBytes, uint64 MemorySize, void* Memory)
{
	bool32 Result = false;

	HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(FileHandle, FileOffsetBytes, 0, FILE_BEGIN); //move file pointer to desired location in file
		
		DWORD BytesWritten;

		if (WriteFile(FileHandle, Memory, (DWORD)MemorySize, &BytesWritten, 0))
		{
			Result = (BytesWritten == MemorySize);
		}
		else
		{
			//error
		}
		CloseHandle(FileHandle);
	}

	return Result;
}


//REMOVE THIS WITH AN IGAME FUNCTION (loadlibrary from IGame.dll)
internal void TEST_FillBackBuffer(game_bitmap_buffer* BackBuffer)
{
	//BB GG RR 00
	uint32* Pixel = (uint32*)BackBuffer->Memory;
	uint32* EndOfBitmap = Pixel + (BackBuffer->Height * BackBuffer->Width);

	while (Pixel != EndOfBitmap)
	{
		*Pixel++ = 0x0000BBAA;
	}

}


internal void Win32DisplayStretchedBitmap(HDC DeviceContext, win32_window_dimensions WindowDim, game_bitmap_buffer* BitmapBuffer)
{
	BITMAPINFO BMI{};
	BMI.bmiHeader.biSize = sizeof(BITMAPINFO);
	BMI.bmiHeader.biWidth = BitmapBuffer->Width;
	BMI.bmiHeader.biHeight = -BitmapBuffer->Height; //top-down DIB
	BMI.bmiHeader.biPlanes = 1;
	BMI.bmiHeader.biBitCount = (WORD)(8 * BitmapBuffer->BytesPerPixel);
	BMI.bmiHeader.biCompression = BI_RGB;



	StretchDIBits(DeviceContext,
		0, 0, WindowDim.Width, WindowDim.Height,
		0, 0, BitmapBuffer->Width, BitmapBuffer->Height,
		BitmapBuffer->Memory, &BMI, DIB_RGB_COLORS, SRCCOPY);
}

internal void Win32DisplayBitmap(HDC DeviceContext, game_bitmap_buffer* BitmapBuffer)
{
	BITMAPINFO BMI{};
	BMI.bmiHeader.biSize = sizeof(BITMAPINFO);
	BMI.bmiHeader.biWidth = BitmapBuffer->Width;
	BMI.bmiHeader.biHeight = -BitmapBuffer->Height; //top-down DIB
	BMI.bmiHeader.biPlanes = 1;
	BMI.bmiHeader.biBitCount = (WORD)(8 * BitmapBuffer->BytesPerPixel);
	BMI.bmiHeader.biCompression = BI_RGB;



	StretchDIBits(DeviceContext,
		0, 0, BitmapBuffer->Width, BitmapBuffer->Height,
		0, 0, BitmapBuffer->Width, BitmapBuffer->Height,
		BitmapBuffer->Memory, &BMI, DIB_RGB_COLORS, SRCCOPY);
}

inline LARGE_INTEGER GetWallClock()
{
	LARGE_INTEGER Counter;
	QueryPerformanceCounter(&Counter);
	return Counter;
}


inline real32 GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
	return (((real32)(End.QuadPart - Start.QuadPart)) / GlobalPerformanceCountFrequency);
	
}


internal bool32 FindChunk(HANDLE File, DWORD fourcc, DWORD* ChunkSize, DWORD* ChunkDataPosition) 
{
	bool32 Result = 0;

	if (SetFilePointer(File, 0, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		//error
		return 1;
	}

	DWORD ChunkType;
	DWORD ChunkDataSize;
	DWORD RIFFDataSize = 0;
	DWORD FileType;
	DWORD BytesRead = 0;
	DWORD Offset = 0;

	while (Result == 0)
	{
		DWORD Read;
		if (ReadFile(File, &ChunkType, sizeof(DWORD), &Read, 0) == 0)
		{
			return 1;
		}

		if (ReadFile(File, &ChunkDataSize, sizeof(DWORD), &Read, 0) == 0)
		{
			return 1;
		}

		switch (ChunkType)
		{
		case fourccRIFF:
		{
			RIFFDataSize = ChunkDataSize;
			ChunkDataSize = 4;

			if (ReadFile(File, &FileType, sizeof(DWORD), &Read, 0) == 0)
			{
				return 1;
			}
			break;

		}

		default:
		{
			if (SetFilePointer(File, ChunkDataSize, 0, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			{
				return 1;
			}
		}

		}

		Offset += ChunkDataSize;

		if (ChunkType == fourcc)
		{
			*ChunkSize = ChunkDataSize;
			*ChunkDataPosition = Offset;
			return 0;
		}

		Offset += ChunkDataSize;

		if (BytesRead >= RIFFDataSize)
		{
			return 1;
		}


	}

	return Result;

}


internal bool32 ReadChunkData(HANDLE File, void* Buffer, DWORD BufferSize, DWORD BufferOffset)
{
	bool32 Result = 0;

	if (SetFilePointer(File, BufferOffset, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		return 1;
	}

	DWORD Read;

	if (ReadFile(File, Buffer, BufferSize, &Read, 0) == 0)
	{
		return 1;
	}

	return Result;
}


internal void Win32TransferOldInputToNew(game_input* NewInput, game_input* OldInput)
{
	/*
	IF ANY BUTTONS ARE ADDED, ADD THEM INTO THIS FUNCTION TOO!
	*/

	NewInput->MouseXPosition = OldInput->MouseXPosition;
	NewInput->MouseYPosition = OldInput->MouseYPosition;

	NewInput->MouseButtons[0].EndedDown = OldInput->MouseButtons[0].EndedDown;
	NewInput->MouseButtons[1].EndedDown = OldInput->MouseButtons[1].EndedDown;


	
	//by reseting HalfTransition at the start of input processing, there is one frame where HalfTransition == 1, which is the first frame to process the press
	NewInput->MouseButtons[0].HalfTransitions = 0;
	NewInput->MouseButtons[1].HalfTransitions = 0;

	NewInput->Back.EndedDown = OldInput->Back.EndedDown;
	NewInput->Back.HalfTransitions = 0;

	NewInput->Start.EndedDown = OldInput->Start.EndedDown;
	NewInput->Start.HalfTransitions = 0;

	NewInput->Up.EndedDown = OldInput->Up.EndedDown;
	NewInput->Up.HalfTransitions = 0;
	
	NewInput->Down.EndedDown = OldInput->Down.EndedDown;
	NewInput->Down.HalfTransitions = 0;
	
	NewInput->Left.EndedDown = OldInput->Left.EndedDown;
	NewInput->Left.HalfTransitions = 0;

	NewInput->Right.EndedDown = OldInput->Right.EndedDown;
	NewInput->Right.HalfTransitions = 0;

	NewInput->AButton.EndedDown = OldInput->AButton.EndedDown;
	NewInput->AButton.HalfTransitions = 0;

	//NewInput->Up = OldInput->Up;
	//NewInput->Down = OldInput->Down;
	//NewInput->Left = OldInput->Left;
	//NewInput->Right = OldInput->Right;
	//NewInput->Start = OldInput->Start;
	//NewInput->Back = OldInput->Back;

	NewInput->LeftThumb = OldInput->LeftThumb;
	NewInput->RightThumb = OldInput->RightThumb;
	NewInput->LeftShoulder = OldInput->LeftShoulder;
	NewInput->RightShoulder = OldInput->RightShoulder;

	//NewInput->AButton = OldInput->AButton;
	NewInput->BButton = OldInput->BButton;
	NewInput->XButton = OldInput->XButton;
	NewInput->YButton = OldInput->YButton;

	NewInput->LeftTrigger = OldInput->LeftTrigger;
	NewInput->RightTrigger = OldInput->RightTrigger;

	NewInput->LeftStickX = OldInput->LeftStickX;
	NewInput->LeftStickY = OldInput->LeftStickY;
	NewInput->RightStickX = OldInput->RightStickX;
	NewInput->RightStickY = OldInput->RightStickY;




}


LRESULT Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{

    LRESULT Result = 0;

    switch (Message) {
    case WM_SIZE:
    {


        break;
    }

    case WM_DESTROY:
    {
       //TODO - error? window was asked to be destroyed, create new window?
        RUNNING = false;
        break;
    }

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        

        break;
    }



    case WM_ACTIVATEAPP:
    {
		

        break;
    }
	/*
	
	    case WM_PAINT:
    {
        
				PAINTSTRUCT paint;

        HDC deviceContext = BeginPaint(Window, &paint);

        win32_window_dimensions dimensions{ GetWindowDimensions(Window)};

        Win32DisplayBufferInWindow(&BACK_BUFFER, deviceContext, dimensions.width, dimensions.height);


        EndPaint(Window, &paint);
		


        break;
    }

	
	case WM_CLOSE:
	{
		//TODO ask the user if they really want to exit the game/program
		//RUNNING = false;
		break;
	}
	
	*/

    default:
    {
        //OutputDebugStringA("WM_SIZE\n");
        Result = DefWindowProcA(Window, Message, WParam, LParam);
        break;
    }

    }

    return Result;

}




internal void Win32InitializeXAudio2()
{
	//AUDIO STUFF
	::CoInitializeEx(nullptr, COINIT_MULTITHREADED);


	XAudio2Create(&XAudioInterface, 0, XAUDIO2_DEFAULT_PROCESSOR);

	XAudioInterface->CreateMasteringVoice(&XAudioMasteringVoice);
}

internal void Win32FormatAudioBeforePlayback(game_audio_info* GameAudioInfo, void* (*ArenaAllocate)(game_arena*, uint64), game_arena* Arena)
{

	HANDLE AudioFile = CreateFile(GameAudioInfo->FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	SetFilePointer(AudioFile, 0, 0, FILE_BEGIN);

	DWORD ChunkSize;
	DWORD ChunkPosition;

	FindChunk(AudioFile, fourccRIFF, &ChunkSize, &ChunkPosition);

	DWORD FileType;
	ReadChunkData(AudioFile, &FileType, sizeof(DWORD), ChunkPosition);

	FindChunk(AudioFile, fourccDATA, &ChunkSize, &ChunkPosition);
	
	GameAudioInfo->AudioData = ArenaAllocate(Arena, ChunkSize);

	ReadChunkData(AudioFile, GameAudioInfo->AudioData, ChunkSize, 44);

	GameAudioInfo->ChunkSize = ChunkSize;

	CloseHandle(AudioFile);
}

internal void Win32SubmitAudioToMasterVoice(game_audio_info* GameAudioInfo)
{
	WAVEFORMATEX  WFX = { 0 };

	WFX.wFormatTag = WAVE_FORMAT_PCM;
	WFX.nChannels = (WORD)GameAudioInfo->Channels;
	WFX.nSamplesPerSec = GameAudioInfo->SamplesPerSecond;
	WFX.wBitsPerSample = (WORD)GameAudioInfo->BitsPerSample;
	WFX.nBlockAlign = (WFX.nChannels * WFX.wBitsPerSample) / 8;
	WFX.nAvgBytesPerSec = WFX.nSamplesPerSec * WFX.nBlockAlign;
	WFX.cbSize = 0;

	XAUDIO2_BUFFER XA2Buffer = { 0 };

	XA2Buffer.AudioBytes = GameAudioInfo->ChunkSize;
	XA2Buffer.pAudioData = (const BYTE*)GameAudioInfo->AudioData;
	XA2Buffer.Flags = XAUDIO2_END_OF_STREAM;

	IXAudio2SourceVoice* XA2SourceVoice{};
	XAudioInterface->CreateSourceVoice(&XA2SourceVoice, &WFX);

	XA2SourceVoice->SetVolume(GameAudioInfo->Volume);
	//XA2SourceVoice->SetFrequencyRatio(GameAudioInfo->Pitch);

	XA2SourceVoice->SubmitSourceBuffer(&XA2Buffer);

	if (SUCCEEDED(XA2SourceVoice->Start(0)))
	{
		//playing audio!

	}

	//Win32FreeFileMemory(DataBuffer);
}




int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CommandLine, int ShowCode)
{
	//initialization (DLL stuff)
	
	//set up timing code
	LARGE_INTEGER PerformanceFrequency;
	QueryPerformanceFrequency(&PerformanceFrequency);
	GlobalPerformanceCountFrequency = PerformanceFrequency.QuadPart;
	
	UINT DesiredSchedulerMs = 1;
	bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMs) == TIMERR_NOERROR);
	
	
	

	//WNDCLASSA
	
	WNDCLASSA WindowClass{};
	
	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "ZippyWindowClass";
	//windowClass.hIcon = ;     //set for a custom window icon
	
	if(RegisterClass(&WindowClass))
	{
		HWND Window{
			CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			"Zippy",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			DEFAULT_WINDOW_WIDTH,
			DEFAULT_WINDOW_HEIGHT,
			0,
			0,
			Instance,
			0)
		};
		



		int GameRefreshRate = 60;
		real32 TargetSecondsPerFrame = 1.0f / (real32)GameRefreshRate;

		
		
		
		
		if(Window)
		{
			//hides cursor (show one using game code instead)
			bool32 CursorVisibility = false;
			ShowCursor(CursorVisibility);

			//Window created, initialize game stuff
			game_bitmap_buffer BackBuffer{};

			BackBuffer.Width = DEFAULT_WINDOW_WIDTH;
			BackBuffer.Height = DEFAULT_WINDOW_HEIGHT;
			BackBuffer.Memory = VirtualAlloc(0, sizeof(uint32) * BackBuffer.Width * BackBuffer.Height, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			BackBuffer.BytesPerPixel = 4;
			BackBuffer.Pitch = BackBuffer.BytesPerPixel * BackBuffer.Width;


			HDC DeviceContext = GetDC(Window);
			
			

			//game memory
			game_memory GameMemory{};

			GameMemory.PermanentStorageSize = MEGABYTES(1);
			GameMemory.PermanentStorage = VirtualAlloc(0, GameMemory.PermanentStorageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			GameMemory.BitMapArenaStorageSize = MEGABYTES(1);
			GameMemory.BitMapArenaStorage = VirtualAlloc(0, GameMemory.BitMapArenaStorageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			GameMemory.EntityArenaStorageSize = sizeof(game_entity) * MAX_ENTITY_COUNT;
			GameMemory.EntityArenaStorage = VirtualAlloc(0, GameMemory.EntityArenaStorageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			GameMemory.ScratchArenaStorageSize = MEGABYTES(5);
			GameMemory.ScratchArenaStorage = VirtualAlloc(0, GameMemory.ScratchArenaStorageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			GameMemory.AudioDataArenaStorageSize = MEGABYTES(350);
			GameMemory.AudioDataArenaStorage = VirtualAlloc(0, GameMemory.AudioDataArenaStorageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			GameMemory.PlatformFreeFileMemory = Win32FreeFileMemory;
			GameMemory.PlatformReadEntireFile = Win32ReadEntireFile;
			GameMemory.PlatformWriteEntireFile = Win32WriteEntireFile;
			GameMemory.PlatformCompareFileTimes = Win32CompareFileTimes;
			GameMemory.PlatformWriteToFile = Win32WriteToFile;
			GameMemory.PlatformSubmitAudioToPlay = Win32SubmitAudioToMasterVoice;
			GameMemory.PlatformFormatAudioBeforePlayback = Win32FormatAudioBeforePlayback;

			win32_game_code GameCode{};
			char SrcGameDLL[] = "IGame.dll";
			char TempGameDLL[] = "IGame_copy.dll";

			GameCode = LoadGameCode(SrcGameDLL, TempGameDLL);


			Win32InitializeXAudio2();

			XAudioMasteringVoice->SetVolume(0.45f);







			//input for this frame
			game_input Input[2]{};

			game_input* NewGameInput = &Input[0];
			game_input* OldGameInput = &Input[1];


			//get counter before game starts for real
			LARGE_INTEGER LastCounter = GetWallClock();
			
			while(RUNNING)
			{
				//check if DLLs were updated and load new code if needed
				FILETIME SourceDLLFileTime = Win32GetLastWriteTime(SrcGameDLL);
				if ( CompareFileTime(&GameCode.DLLLastWriteTime, &SourceDLLFileTime) != 0)
				{
					UnloadGameCode(&GameCode);
					GameCode = LoadGameCode(SrcGameDLL, TempGameDLL);
				}




				//set old mouse state into the current mouse state to carry over state information
				// 
				//---------------------------------		IMPORTANT	 -------------------------------------
				// Input state MUST be carried over between frames. ALL old input state must be set into new input before
				// any processing of buttons or mouse positions can be done for this frame.
				Win32TransferOldInputToNew(NewGameInput, OldGameInput);



				//NewGameInput.MouseState[1] = OldGameInput.MouseState[1];

				
				//windows message handling
				MSG message;


				//process windows messages
				while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
				{


					switch (message.message)
					{
					case WM_QUIT:
					{
						RUNNING = false;

						break;
					}

					case WM_SYSKEYDOWN:
					case WM_SYSKEYUP:
					case WM_KEYDOWN:
					case WM_KEYUP:
					{

						uint32 VKCode = (uint32)message.wParam;

						bool32 WasDown = (message.lParam & (1 << 30)) != 0;
						bool32 IsDown = (message.lParam & (1 << 31)) == 0;

						if (WasDown != IsDown)
						{
							if (VKCode == 'A')
							{
								Win32ProcessButton(&OldGameInput->Left, &NewGameInput->Left, IsDown);
							}
							if (VKCode == 'D')
							{
								Win32ProcessButton(&OldGameInput->Right, &NewGameInput->Right, IsDown);
							}
							if (VKCode == 'W')
							{
								Win32ProcessButton(&OldGameInput->Up, &NewGameInput->Up, IsDown);
							}
							if (VKCode == 'S')
							{
								Win32ProcessButton(&OldGameInput->Down, &NewGameInput->Down, IsDown);
							}

							if (VKCode == VK_BACK)
							{
								Win32ProcessButton(&OldGameInput->Back, &NewGameInput->Back, IsDown);
							}

							if (VKCode == VK_SPACE)
							{
								Win32ProcessButton(&OldGameInput->LeftShoulder, &NewGameInput->LeftShoulder, IsDown);
							}

							if (VKCode == 'F')
							{
								Win32ProcessButton(&OldGameInput->AButton, &NewGameInput->AButton, IsDown);
							}
							
							if (VKCode == VK_ESCAPE)
							{
								Win32ProcessButton(&OldGameInput->Start, &NewGameInput->Start, IsDown);
							}
						}



						bool32 AltKeyWasDown = (message.lParam & (1 << 29));
						if ((VKCode == VK_F4) && AltKeyWasDown)
						{
							RUNNING = false;
						}

						

						break;
					}


					case WM_MOUSEMOVE:
					{
						//message is received when the mouse is moved inside the window
						int16 MouseXPos = (int16)(message.lParam & 0x0000FFFF);
						int16 MouseYPos = (int16)(message.lParam >> 16);


						win32_window_dimensions ClientWindow{};
						Win32GetWindowDimension(Window, &ClientWindow);


						real64 WidthRatio = (real64)BackBuffer.Width / (real64)ClientWindow.Width;
						real64 HeightRatio = (real64)BackBuffer.Height / (real64)ClientWindow.Height;

						int32 GameXPos = (int32)((real64)MouseXPos * WidthRatio);
						int32 GameYPos = (int32)((real64)MouseYPos * HeightRatio);

						NewGameInput->MouseXPosition = GameXPos;
						NewGameInput->MouseYPosition = GameYPos;
						/*
												char MouseTestOutput[256];
						_snprintf_s(MouseTestOutput, sizeof(MouseTestOutput), "(%d,%d) W:%d H:%d   WR:%.02f HR:%.02f G(%d,%d)\n",
							MouseXPos, MouseYPos, ClientWindow.Width, ClientWindow.Height, WidthRatio, HeightRatio, GameXPos, GameYPos);
						OutputDebugStringA(MouseTestOutput);
						*/


						break;
					}

					case WM_LBUTTONDOWN:
					{
						Win32ProcessButton(&OldGameInput->MouseButtons[0], &NewGameInput->MouseButtons[0], true);

						break;
					}
					case WM_LBUTTONUP:
					{

						Win32ProcessButton(&OldGameInput->MouseButtons[0], &NewGameInput->MouseButtons[0], false);

						break;
					}
					case WM_RBUTTONDOWN:
					{
						Win32ProcessButton(&OldGameInput->MouseButtons[1], &NewGameInput->MouseButtons[1], true);

						break;
					}
					case WM_RBUTTONUP:
					{

						Win32ProcessButton(&OldGameInput->MouseButtons[1], &NewGameInput->MouseButtons[1], false);

						break;
					}


					default:
					{
						TranslateMessage(&message);
						DispatchMessageA(&message);

						break;
					}

					}


				}
				


				//			GET CONTROLLER INPUT
				//	My game will only support 1 controller, it is a single player game...
				
				// set new controller to the state of the old controller

				XINPUT_STATE ControllerState{};

				if (XInputGetState(0, &ControllerState) == ERROR_SUCCESS)
				{
					NewGameInput->ControllerDetected = true;

					XINPUT_GAMEPAD Controller = ControllerState.Gamepad;
					int32 GamePadButtons = Controller.wButtons;

					Win32ProcessControllerInput(&OldGameInput->Up, &NewGameInput->Up, GamePadButtons,  XINPUT_GAMEPAD_DPAD_UP);
					Win32ProcessControllerInput(&OldGameInput->Down, &NewGameInput->Down, GamePadButtons, XINPUT_GAMEPAD_DPAD_DOWN);
					Win32ProcessControllerInput(&OldGameInput->Left, &NewGameInput->Left, GamePadButtons, XINPUT_GAMEPAD_DPAD_LEFT);
					Win32ProcessControllerInput(&OldGameInput->Right, &NewGameInput->Right, GamePadButtons, XINPUT_GAMEPAD_DPAD_RIGHT);

					Win32ProcessControllerInput(&OldGameInput->Start, &NewGameInput->Start, GamePadButtons, XINPUT_GAMEPAD_START);
					Win32ProcessControllerInput(&OldGameInput->Back, &NewGameInput->Back, GamePadButtons, XINPUT_GAMEPAD_BACK);
					
					Win32ProcessControllerInput(&OldGameInput->LeftThumb, &NewGameInput->LeftThumb, GamePadButtons, XINPUT_GAMEPAD_LEFT_THUMB);
					Win32ProcessControllerInput(&OldGameInput->RightThumb, &NewGameInput->RightThumb, GamePadButtons, XINPUT_GAMEPAD_RIGHT_THUMB);

					Win32ProcessControllerInput(&OldGameInput->LeftShoulder, &NewGameInput->LeftShoulder, GamePadButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
					Win32ProcessControllerInput(&OldGameInput->RightShoulder, &NewGameInput->RightShoulder, GamePadButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);

					Win32ProcessControllerInput(&OldGameInput->AButton, &NewGameInput->AButton, GamePadButtons, XINPUT_GAMEPAD_A);
					Win32ProcessControllerInput(&OldGameInput->BButton, &NewGameInput->BButton, GamePadButtons, XINPUT_GAMEPAD_B);
					Win32ProcessControllerInput(&OldGameInput->XButton, &NewGameInput->XButton, GamePadButtons, XINPUT_GAMEPAD_X);
					Win32ProcessControllerInput(&OldGameInput->YButton, &NewGameInput->YButton, GamePadButtons, XINPUT_GAMEPAD_Y);

					if (Controller.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
					{
						NewGameInput->LeftTrigger = Controller.bLeftTrigger;
					}
					else
					{
						NewGameInput->LeftTrigger = 0;
					}


					if (Controller.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
					{
						NewGameInput->RightTrigger = Controller.bRightTrigger;
					}
					else
					{
						NewGameInput->RightTrigger = 0;
					}

					NewGameInput->LeftStickX = Win32ProcessXInputStick(Controller.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
					NewGameInput->LeftStickY = Win32ProcessXInputStick(Controller.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
					NewGameInput->RightStickX = Win32ProcessXInputStick(Controller.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
					NewGameInput->RightStickY = Win32ProcessXInputStick(Controller.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);



					
				}
				else
				{
					//Controller read failed
					NewGameInput->ControllerDetected = false;
				}
				
				



				

				//TEST_FillBackBuffer(&BackBuffer);
				GameCode.GameUpdateAndRender(&GameMemory, NewGameInput, &BackBuffer, TargetSecondsPerFrame);
				
				//game signaled game should shut down now (only check the game is running)
				if (RUNNING)
				{
					RUNNING = GameMemory.GameRunning;
				}
				
				//ShowCursor(GameMemory.ShowCursor);








				
				
				
				LARGE_INTEGER WorkCounter = GetWallClock();
				real32 WorkSecondsElapsed = GetSecondsElapsed(LastCounter, WorkCounter);
				real32 SecondsElapsedForFrame = WorkSecondsElapsed;
				GameMemory.MSForLastFrame = SecondsElapsedForFrame;

				if(SecondsElapsedForFrame < TargetSecondsPerFrame)
				{
					if(SleepIsGranular)
					{
						DWORD SleepMs = (DWORD)((TargetSecondsPerFrame - SecondsElapsedForFrame) * 925.0f);
						
						if(SleepMs > 0)
						{
							Sleep(SleepMs);
						}
						
					}
					
					while(SecondsElapsedForFrame < TargetSecondsPerFrame)
					{
						SecondsElapsedForFrame = GetSecondsElapsed(LastCounter, GetWallClock());
					}
					
				}
				else
				{
					//FRAMERATE MISSED!!!
				}
				
				
				
				LARGE_INTEGER EndCounter = GetWallClock();
				real32 MsPerFrame = GetSecondsElapsed(LastCounter, EndCounter) * 1000.0f;
				real64 FPS = ((real64)GlobalPerformanceCountFrequency / (((real64)EndCounter.QuadPart) - ((real64)LastCounter.QuadPart)));
				
				
				LastCounter = EndCounter;
				
				




				//				FLIP BACKBUFFER HERE!!!
				
				win32_window_dimensions WindowDimensions{};
				Win32GetWindowDimension(Window, &WindowDimensions);

				Win32DisplayStretchedBitmap(DeviceContext, WindowDimensions, &BackBuffer);
				//Win32DisplayBitmap(DeviceContext, &BackBuffer);


				//swap inputs
				game_input* INPUT_TEMP = OldGameInput;
				OldGameInput = NewGameInput;
				NewGameInput = INPUT_TEMP;


#ifdef DEBUG
				char FPSBuffer[256];
				_snprintf_s(FPSBuffer, sizeof(FPSBuffer), "%.02fms/f, %.02f FPS\n", MsPerFrame, FPS);
				OutputDebugStringA(FPSBuffer);
#endif // DEBUG


				
			}
			
			
		}
		else
		{
			
		}
		
		
		
	}
	else
	{
		
	}
	
	//set up window
	
	//set up frame rate limit
	
	//game initialization (memory allocation, sound stuff)
	
	//main game loop
	
	//input processing
	
	//windows message processing
	
	//game update and render
	
	//sound code XAudio2
	
	//spin-lock frame time enforcement
	
	//display bitmap (game rendering buffer flip)
	
	

	return 0;
}