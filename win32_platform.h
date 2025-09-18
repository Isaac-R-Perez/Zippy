#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

#define DEFAULT_WINDOW_WIDTH 1920
#define DEFAULT_WINDOW_HEIGHT 1080

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

struct win32_window_dimensions
{
	int32 Width;
	int32 Height;
};

struct win32_game_code
{
	HMODULE GameCodeDLL;
	bool32 IsValid;
	FILETIME DLLLastWriteTime;

	void (*GameUpdateAndRender)(game_memory* , game_input*, game_bitmap_buffer*, real32);
};
#endif