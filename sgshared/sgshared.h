#pragma once
#pragma once
#include <Windows.h>
#define SPEEDGEAR_MEMORY_MAPPING_NAME "SPEEDGEAR_SHARED_MEMORY"
#define SPEEDGEAR_MAX_HOOK_PROCESS 8
#define SPEEDGEAR_PROC SGProc
#ifdef _WIN64
#define SPEEDGEAR_PROC_STR _CRT_STRINGIZE(SPEEDGEAR_PROC)
#else
#define SPEEDGEAR_PROC_STR "_" _CRT_STRINGIZE(SPEEDGEAR_PROC) "@12"
#endif

typedef struct
{
	DWORD hookCount;
	DWORD hookPid[SPEEDGEAR_MAX_HOOK_PROCESS];
	float hookSpeed;
	int hookMode;
	int statusPosition;
	BOOL useSystemDPI;
	BOOL fontItalic;
	DWORD fontSize;//Point, LOGFONT::lfHeight(Pixel)
	DWORD fontWeight;
	DWORD fontColor;
	char fontName[32];
	char statusFormat[256];
	char fontPath[MAX_PATH];
}SPEEDGEAR_SHARED_MEMORY;

#define PIXEL_TO_LOGICAL_UNIT(hwnd,fs) MulDiv(fs,72,GetDeviceCaps(GetDC(hwnd),LOGPIXELSY))
#define LOGICAL_UNIT_TO_PIXEL(hwnd,p) MulDiv(p,GetDeviceCaps(GetDC(hwnd),LOGPIXELSY),72)
#define PIXEL_TO_LOGICAL_UNIT_96DPI(fs) MulDiv(fs,72,USER_DEFAULT_SCREEN_DPI)
#define LOGICAL_UNIT_TO_PIXEL_96DPI(p) MulDiv(p,USER_DEFAULT_SCREEN_DPI,72)
#define DPI_SCALED_VALUE(hwnd,x) MulDiv(x,GetDeviceCaps(GetDC(hwnd),LOGPIXELSY),USER_DEFAULT_SCREEN_DPI)

#ifdef __cplusplus
extern "C"{
#endif
BOOL SpeedGear_InitializeSharedMemory(BOOL bCreate);
BOOL SpeedGear_ReleaseSharedMemory();
BOOL SpeedGear_IsSharedMemoryInitialized();
SPEEDGEAR_SHARED_MEMORY* SpeedGear_GetSharedMemory();

char* SpeedGear_FormatText(char* buf, int len, const char* fmt, float speed, int fps, int width, int height, int hour, int minute, int second,const char*api);
#ifdef __cplusplus
}
#endif
