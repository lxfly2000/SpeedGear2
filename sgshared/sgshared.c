#include "sgshared.h"

HANDLE hSgMemMap = NULL;
SPEEDGEAR_SHARED_MEMORY* pSgMem = NULL;

BOOL SpeedGear_InitializeSharedMemory(BOOL bCreate)
{
    hSgMemMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SPEEDGEAR_MEMORY_MAPPING_NAME);
    if (!hSgMemMap)
    {
        if (bCreate)
            hSgMemMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPEEDGEAR_SHARED_MEMORY), SPEEDGEAR_MEMORY_MAPPING_NAME);
        if (!hSgMemMap)
            return FALSE;
    }
    pSgMem = (SPEEDGEAR_SHARED_MEMORY*)MapViewOfFile(hSgMemMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (pSgMem == NULL)
        return FALSE;
    ZeroMemory(pSgMem, sizeof(SPEEDGEAR_SHARED_MEMORY));
    return TRUE;
}

BOOL SpeedGear_ReleaseSharedMemory()
{
    if (!UnmapViewOfFile(pSgMem))
        return FALSE;
    pSgMem = NULL;
    if (!CloseHandle(hSgMemMap))
        return FALSE;
    hSgMemMap = NULL;
    return TRUE;
}

BOOL SpeedGear_IsSharedMemoryInitialized()
{
    return hSgMemMap && pSgMem;
}

SPEEDGEAR_SHARED_MEMORY* SpeedGear_GetSharedMemory()
{
    return pSgMem;
}
