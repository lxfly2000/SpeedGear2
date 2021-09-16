#pragma once
#pragma once
#include <Windows.h>
#define SPEEDGEAR_MEMORY_MAPPING_NAME "SPEEDGEAR_SHARED_MEMORY"
#define SPEEDGEAR_MAX_HOOK_PROCESS 8

typedef struct
{
	float hookSpeed;
	BOOL hookIsOn;
	int hookMode;
	DWORD hookCount;
	DWORD hookPid[SPEEDGEAR_MAX_HOOK_PROCESS];
	int statusPosition;
	DWORD fontSize;
	DWORD fontColor;
	char fontName[32];
	char fontSubName[16];
	char statusFormat[256];
	char cd[MAX_PATH];
}SPEEDGEAR_SHARED_MEMORY;

BOOL SpeedGear_InitializeSharedMemory(BOOL bCreate);
BOOL SpeedGear_ReleaseSharedMemory();
BOOL SpeedGear_IsSharedMemoryInitialized();
SPEEDGEAR_SHARED_MEMORY* SpeedGear_GetSharedMemory();
