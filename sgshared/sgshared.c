#include "sgshared.h"
#include <stdio.h>

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
    if (bCreate)
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

BOOL IsFmtValid(const char* fmt)
{
    int ps = 0;//百分号状态 0:没有百分号 1:有百分号
    while (*fmt)
    {
        switch (ps)
        {
        case 0:
            if (*fmt == '%')
                ps = 1;
            break;
        case 1:
            if (*fmt == '%' || isalpha(*fmt))
                ps = 0;
            break;
        }
        fmt++;
    }
    return ps == 0;
}

char* SpeedGear_FormatText(char* buf, int len, const char* fmt, float speed, int fps, int width, int height, int hour, int minute, int second,const char*api)
{
    if (buf == fmt)
        return NULL;
    ZeroMemory(buf, len);
    int posFmt = 0, posBuf = 0;
    int braceStatus = 0;//0=正常 1=左括号 2=冒号
    char kwbuf[8] = "";//变量名
    char kwfmt[8] = "";//格式串
    char kwstr[16] = "";//数值串
    int posKwBuf = 0, posKwFmt = 0;
    while (fmt[posFmt] != 0)
    {
        if (braceStatus == 0)
        {
            if (fmt[posFmt] == '{')
            {
                braceStatus = 1;
            }
            else
            {
                buf[posBuf] = fmt[posFmt];
                posBuf++;
            }
        }
        else
        {
            if (fmt[posFmt] == ':')
            {
                braceStatus = 2;
            }
            else if(fmt[posFmt]=='}')
            {
                kwfmt[posKwFmt] = 0;
                posKwFmt = 0;
                kwbuf[posKwBuf] = 0;
                posKwBuf = 0;
                braceStatus = 0;
                if (lstrcmpA(kwbuf, "speed") == 0)
                {
                    if (lstrlenA(kwfmt) == 0)
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), "%g", speed);
                    else if (IsFmtValid(kwfmt))
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), kwfmt, speed);
                }
                else if (lstrcmpA(kwbuf, "fps") == 0)
                {
                    if (lstrlenA(kwfmt) == 0)
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), "%d", fps);
                    else if (IsFmtValid(kwfmt))
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), kwfmt, fps);
                }
                else if (lstrcmpA(kwbuf, "width") == 0)
                {
                    if (lstrlenA(kwfmt) == 0)
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), "%d", width);
                    else if (IsFmtValid(kwfmt))
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), kwfmt, width);
                }
                else if (lstrcmpA(kwbuf, "height") == 0)
                {
                    if (lstrlenA(kwfmt) == 0)
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), "%d", height);
                    else if (IsFmtValid(kwfmt))
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), kwfmt, height);
                }
                else if (lstrcmpA(kwbuf, "hour") == 0)
                {
                    if (lstrlenA(kwfmt) == 0)
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), "%d", hour);
                    else if (IsFmtValid(kwfmt))
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), kwfmt, hour);
                }
                else if (lstrcmpA(kwbuf, "minute") == 0)
                {
                    if (lstrlenA(kwfmt) == 0)
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), "%d", minute);
                    else if (IsFmtValid(kwfmt))
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), kwfmt, minute);
                }
                else if (lstrcmpA(kwbuf, "second") == 0)
                {
                    if (lstrlenA(kwfmt) == 0)
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), "%d", second);
                    else if (IsFmtValid(kwfmt))
                        sprintf_s(kwstr, ARRAYSIZE(kwstr), kwfmt, second);
                }
                else if (lstrcmpA(kwbuf, "api") == 0)
                {
                    lstrcpyA(kwstr, api);
                }
                posBuf += lstrlenA(kwstr);
                lstrcatA(buf, kwstr);
                ZeroMemory(kwstr, sizeof kwstr);
                ZeroMemory(kwbuf, sizeof kwbuf);
                ZeroMemory(kwfmt, sizeof kwfmt);
            }
            else if (braceStatus == 2)
            {
                kwfmt[posKwFmt] = fmt[posFmt];
                posKwFmt++;
            }
            else
            {
                kwbuf[posKwBuf] = fmt[posFmt];
                posKwBuf++;
            }
        }
        posFmt++;
    }
    buf[posBuf] = 0;
    return buf;
}
