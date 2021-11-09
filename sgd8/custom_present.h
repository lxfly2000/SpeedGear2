#pragma once
#include"d3d8.h"
#ifdef __cplusplus
extern "C" {
#endif
//自定义Present的附加操作
void CustomPresent(LPDIRECT3DDEVICE8);
void CustomReset(LPDIRECT3DDEVICE8, D3DPRESENT_PARAMETERS*);
void CustomSCPresent(LPDIRECT3DSWAPCHAIN8);
#ifdef __cplusplus
}
#endif
