#pragma once
#pragma once
#include <Windows.h>
#define SPEEDGEAR_MEMORY_MAPPING_NAME "SPEEDGEAR_SHARED_MEMORY"
#define SPEEDGEAR_MAX_HOOK_PROCESS 8

typedef struct
{
	DWORD hookCount;
	DWORD hookPid[SPEEDGEAR_MAX_HOOK_PROCESS];
	float hookSpeed;
	BOOL hookIsOn;
	int hookMode;
	int statusPosition;
	BOOL fontItalic;
	LONG fontHeight;
	DWORD fontWeight;
	DWORD fontColor;
	char fontName[32];
	char statusFormat[256];
	char cd[MAX_PATH];
}SPEEDGEAR_SHARED_MEMORY;

#define FONTHEIGHT_TO_POUND(fs) -MulDiv(fs,72,GetDeviceCaps(GetDC(NULL),LOGPIXELSY))
#define POUND_TO_FONTHEIGHT(p) -MulDiv(p,GetDeviceCaps(GetDC(NULL),LOGPIXELSY),72)

BOOL SpeedGear_InitializeSharedMemory(BOOL bCreate);
BOOL SpeedGear_ReleaseSharedMemory();
BOOL SpeedGear_IsSharedMemoryInitialized();
SPEEDGEAR_SHARED_MEMORY* SpeedGear_GetSharedMemory();
