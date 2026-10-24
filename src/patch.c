#include "patch.h"
#include <stdint.h>
#include <string.h>

_Static_assert(sizeof(void *) == 4, "Cricket 07 requires a 32-bit plugin");

static void put32(BYTE *where, uintptr_t value) {
    DWORD word = (DWORD)value;
    memcpy(where, &word, sizeof(word));
}

static void jump32(BYTE *where, const void *target) {
    where[0] = 0xE9;
    put32(where + 1, (uintptr_t)target - (uintptr_t)(where + 5));
}

BOOL CpuVsCpuInstall(BYTE *image, void **stub_out) {
    /* Original controller counts, disabled-button path, and enabled-button path.
       Absolute operands are adjusted for the actual executable load address. */
    BYTE expected[] = {
        0x80,0x3D,0x0C,0xE5,0x90,0x00,0x00,0x77,0x1A,
        0x80,0x3D,0x0D,0xE5,0x90,0x00,0x00,0x77,0x11,
        0x80,0x66,0x35,0xFE,0x6A,0x40,0x8D,0x4E,0x58,
        0x6A,0x08,0x8D,0x41,0x0C,0x50,0xEB,0x12,
        0x80,0x4E,0x35,0x01
    };
    BYTE getter[] = {0xB8,0,0,0,0,0xC3};
    /* Preserve EAX/EFLAGS. Exhibition 0x0B and tournament 0x14 skip the count test.
       Other screens replay the displaced CMP and use the original branches.
       This executable stub calls no C code and needs no stack-alignment fix. */
    BYTE code[] = {
        0x9C,                         /* pushfd */
        0x50,                         /* push eax */
        0xA1,0,0,0,0,                /* mov eax,[frontend.screen] */
        0x85,0xC0,                   /* test eax,eax */
        0x74,0x15,                   /* jz normal */
        0x66,0x83,0x78,0x04,0x0B,    /* cmp word [eax+4],0x0B */
        0x74,0x07,                   /* je enable */
        0x66,0x83,0x78,0x04,0x14,    /* cmp word [eax+4],0x14 */
        0x75,0x07,                   /* jne normal */
        0x58,0x9D,                   /* pop eax; popfd */
        0xE9,0,0,0,0,                /* jmp original enable path */
        0x58,0x9D,                   /* normal: pop eax; popfd */
        0x80,0x3D,0,0,0,0,0x00,     /* cmp byte [home count],0 */
        0xE9,0,0,0,0                 /* jmp original JA */
    };
    BYTE detour[C07_HOOK_LENGTH];
    BYTE *site = image + C07_GATE_RVA;
    BYTE *stub;
    DWORD previous, ignored;
    *stub_out = NULL;
    put32(expected + 2, (uintptr_t)(image + C07_COUNT_RVA));
    put32(expected + 11, (uintptr_t)(image + C07_COUNT_RVA + 1));
    put32(getter + 1, (uintptr_t)(image + C07_FRONTEND_RVA));
    if (memcmp(site, expected, sizeof(expected)) != 0 ||
        memcmp(image + C07_GET_FRONTEND_RVA, getter, sizeof(getter)) != 0)
        return FALSE;

    stub = VirtualAlloc(NULL, sizeof(code), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!stub) return FALSE;
    memcpy(stub, code, sizeof(code));
    put32(stub + 3, (uintptr_t)(image + C07_FRONTEND_RVA));
    jump32(stub + 27, image + C07_ENABLE_RVA);
    put32(stub + 36, (uintptr_t)(image + C07_COUNT_RVA));
    jump32(stub + 41, site + C07_HOOK_LENGTH);
    if (!VirtualProtect(stub, sizeof(code), PAGE_EXECUTE_READ, &previous) ||
        !FlushInstructionCache(GetCurrentProcess(), stub, sizeof(code))) {
        VirtualFree(stub, 0, MEM_RELEASE);
        return FALSE;
    }
    /* This jump is assembled on the stack but executes at site. */
    detour[0] = 0xE9;
    put32(detour + 1, (uintptr_t)stub - (uintptr_t)(site + 5));
    detour[5] = detour[6] = 0x90;
    if (!VirtualProtect(site, sizeof(detour), PAGE_EXECUTE_READWRITE, &previous)) {
        VirtualFree(stub, 0, MEM_RELEASE);
        return FALSE;
    }
    memcpy(site, detour, sizeof(detour));
    FlushInstructionCache(GetCurrentProcess(), site, sizeof(detour));
    VirtualProtect(site, sizeof(detour), previous, &ignored);
    *stub_out = stub;
    return TRUE;
}
