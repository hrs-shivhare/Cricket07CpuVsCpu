#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../src/patch.h"

static int failures;
static void check(int ok, const char *message) {
    if (!ok) { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}
static void word32(BYTE *where, uintptr_t word) {
    DWORD value = (DWORD)word;
    memcpy(where, &value, 4);
}
static BYTE *fixture(void) {
    BYTE prefix[] = {
        0x80,0x3D,0x0C,0xE5,0x90,0x00,0x00,0x77,0x1A,
        0x80,0x3D,0x0D,0xE5,0x90,0x00,0x00,0x77,0x11,
        0x80,0x66,0x35,0xFE,0x6A,0x40,0x8D,0x4E,0x58,
        0x6A,0x08,0x8D,0x41,0x0C,0x50,0xEB,0x12,
        0x80,0x4E,0x35,0x01
    };
    BYTE *image = VirtualAlloc(NULL, 0x650000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!image) return NULL;
    word32(prefix + 2, (uintptr_t)(image + C07_COUNT_RVA));
    word32(prefix + 11, (uintptr_t)(image + C07_COUNT_RVA + 1));
    memcpy(image + C07_GATE_RVA, prefix, sizeof(prefix));
    image[C07_GET_FRONTEND_RVA] = 0xB8;
    word32(image + C07_GET_FRONTEND_RVA + 1, (uintptr_t)(image + C07_FRONTEND_RVA));
    image[C07_GET_FRONTEND_RVA + 5] = 0xC3;
    return image;
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    BYTE *image = fixture(), *wrapper;
    BYTE screen[16] = {0}, control[128] = {0};
    const WORD screen_ids[] = {0x0B,0x14,0x23,0x1FB,0x3B,0x10B,0x114,0x260,0};
    BYTE wrapper_code[] = {
        0x56,0x8B,0x74,0x24,0x08,       /* push esi; mov esi,[esp+8] */
        0xE8,0,0,0,0,                  /* call relocated original gate */
        0x0F,0xB6,0x46,0x35,0x5E,0xC3 /* movzx eax,[esi+35]; pop esi; ret */
    };
    void *stub = NULL, *second = NULL;
    DWORD old, ignored;
    int cases = 0;
    check(image != NULL, "fixture allocation");
    if (!image) return 1;
    puts("Installing native hook fixture...");
    VirtualProtect(image + C07_GATE_RVA, 40, PAGE_EXECUTE_READ, &old);
    check(CpuVsCpuInstall(image, &stub), "install against supported instruction context");
    check(stub != NULL && image[C07_GATE_RVA] == 0xE9,
          "detour installed and executable stub allocated");
    check(!CpuVsCpuInstall(image, &second), "second hook/conflicting hook rejected");

    /* Stop at the two original button-bit writes. The real game's following
       instructions update button colour; those UI calls aren't fixture code. */
    VirtualProtect(image + C07_GATE_RVA, 40, PAGE_EXECUTE_READWRITE, &old);
    image[C07_GATE_RVA + 22] = 0xC3;
    image[C07_ENABLE_RVA + 4] = 0xC3;
    FlushInstructionCache(GetCurrentProcess(), image + C07_GATE_RVA, 40);
    VirtualProtect(image + C07_GATE_RVA, 40, PAGE_EXECUTE_READ, &ignored);
    wrapper = VirtualAlloc(NULL, sizeof(wrapper_code), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    check(wrapper != NULL, "native wrapper allocation");
    if (!wrapper || !stub) return 1;
    memcpy(wrapper, wrapper_code, sizeof(wrapper_code));
    word32(wrapper + 6, (uintptr_t)(image + C07_GATE_RVA) - (uintptr_t)(wrapper + 10));
    VirtualProtect(wrapper, sizeof(wrapper_code), PAGE_EXECUTE_READ, &old);
    FlushInstructionCache(GetCurrentProcess(), wrapper, sizeof(wrapper_code));

    for (unsigned context = 0; context < sizeof(screen_ids)/sizeof(screen_ids[0]); ++context) {
        *(WORD *)(screen + 4) = screen_ids[context];
        word32(image + C07_FRONTEND_RVA, screen_ids[context] == 0 ? 0 : (uintptr_t)screen);
        for (unsigned home = 0; home < 2; ++home) {
            for (unsigned away = 0; away < 2; ++away) {
                for (unsigned enabled = 0; enabled < 2; ++enabled) {
                    unsigned result, expected = 0xA0 | (context < 2 || home || away);
                    image[C07_COUNT_RVA] = (BYTE)home;
                    image[C07_COUNT_RVA + 1] = (BYTE)away;
                    control[0x35] = (BYTE)(0xA0 | enabled);
                    result = ((unsigned (__cdecl *)(void *))wrapper)(control);
                    check(result == expected, "screen/assignment/old-state native execution");
                    ++cases;
                }
            }
        }
    }
    VirtualFree(wrapper, 0, MEM_RELEASE);
    VirtualFree(stub, 0, MEM_RELEASE);
    VirtualFree(image, 0, MEM_RELEASE);

    image = fixture();
    if (!image) return 1;
    image[C07_GATE_RVA] ^= 1;
    check(!CpuVsCpuInstall(image, &stub) && !stub, "altered game code rejected before patching");
    VirtualFree(image, 0, MEM_RELEASE);

    if (argc > 1) {
        HMODULE module = LoadLibraryA(argv[1]);
        void (__cdecl *init)(void);
        LONG (__cdecl *get_status)(void);
        check(module != NULL, "32-bit ASI loads with installed Windows dependencies");
        if (module) {
            init = (void (__cdecl *)(void))GetProcAddress(module, "InitializeASI");
            get_status = (LONG (__cdecl *)(void))GetProcAddress(module, "GetCpuVsCpuStatus");
            check(init && get_status, "undecorated ASI exports present");
            if (init && get_status) {
                init();
                check(get_status() == -1, "unsupported host executable safely refused");
                init();
                check(get_status() == -1, "loader initialization is idempotent");
            }
            FreeLibrary(module);
        }
    }
    printf("%d native screen/assignment/state cases; failures=%d\n", cases, failures);
    return failures ? 1 : 0;
}
