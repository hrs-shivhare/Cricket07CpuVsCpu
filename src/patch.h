#ifndef CRICKET07_CPU_PATCH_H
#define CRICKET07_CPU_PATCH_H
#include <windows.h>

#define C07_GATE_RVA 0x0007AC38u
#define C07_ENABLE_RVA 0x0007AC5Bu
#define C07_COUNT_RVA 0x0050E50Cu
#define C07_FRONTEND_RVA 0x0055BFE8u
#define C07_GET_FRONTEND_RVA 0x00087930u
#define C07_HOOK_LENGTH 7u

BOOL CpuVsCpuInstall(BYTE *image, void **stub_out);
#endif
