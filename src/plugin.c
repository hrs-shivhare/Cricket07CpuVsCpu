#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#include <string.h>
#include "patch.h"

static HMODULE self;
static volatile LONG initialized;
static volatile LONG status;
static void *stub;
static const BYTE supported_sha256[32] = {
    0xD7,0x76,0xDB,0xA0,0xCC,0x50,0x6B,0x96,
    0x8F,0x7D,0x22,0x8E,0x03,0x6C,0xF4,0x2A,
    0xEB,0x4A,0xBE,0x45,0xA8,0xD1,0xAD,0x0D,
    0x72,0x26,0xCD,0xEB,0xF9,0x15,0x53,0xA2
};

static void log_line(const char *line) {
    WCHAR path[MAX_PATH];
    DWORD length = GetModuleFileNameW(self, path, MAX_PATH), written;
    HANDLE file;
    if (!length || length >= MAX_PATH) return;
    while (length && path[length - 1] != L'\\') --length;
    if (length + 23 >= MAX_PATH) return;
    memcpy(path + length, L"Cricket07CpuVsCpu.log", sizeof(L"Cricket07CpuVsCpu.log"));
    file = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return;
    WriteFile(file, line, (DWORD)strlen(line), &written, NULL);
    WriteFile(file, "\r\n", 2, &written, NULL);
    CloseHandle(file);
}

static BOOL supported_executable(void) {
    WCHAR path[MAX_PATH];
    BYTE buffer[16384], digest[32];
    DWORD count, size = sizeof(digest);
    HCRYPTPROV provider = 0;
    HCRYPTHASH hash = 0;
    HANDLE file = INVALID_HANDLE_VALUE;
    BOOL ok = FALSE;
    DWORD length = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (!length || length >= MAX_PATH) return FALSE;
    file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE |
                      FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return FALSE;
    if (!CryptAcquireContextW(&provider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) ||
        !CryptCreateHash(provider, CALG_SHA_256, 0, 0, &hash)) goto done;
    for (;;) {
        if (!ReadFile(file, buffer, sizeof(buffer), &count, NULL)) goto done;
        if (!count) break;
        if (!CryptHashData(hash, buffer, count, 0)) goto done;
    }
    if (!CryptGetHashParam(hash, HP_HASHVAL, digest, &size, 0)) goto done;
    ok = size == sizeof(digest) && memcmp(digest, supported_sha256, size) == 0;
done:
    if (hash) CryptDestroyHash(hash);
    if (provider) CryptReleaseContext(provider, 0);
    CloseHandle(file);
    return ok;
}

/* Ultimate ASI Loader invokes this AFTER LoadLibrary returns. Keep hashing,
   provider loading and patch allocation outside the Windows loader lock. */
void InitializeASI(void) {
    BYTE *image;
    IMAGE_DOS_HEADER *dos;
    IMAGE_NT_HEADERS32 *pe;
    if (InterlockedCompareExchange(&initialized, 1, 0)) return;
    log_line("Cricket07CpuVsCpu 1.1: initializing (exhibition and tournament screens).");
    if (!supported_executable()) {
        status = -1;
        log_line("NOT APPLIED: executable SHA-256 does not match the supported Cricket07.exe.");
        return;
    }
    image = (BYTE *)GetModuleHandleW(NULL);
    dos = (IMAGE_DOS_HEADER *)image;
    pe = (IMAGE_NT_HEADERS32 *)(image + dos->e_lfanew);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || pe->Signature != IMAGE_NT_SIGNATURE ||
        pe->FileHeader.Machine != IMAGE_FILE_MACHINE_I386 ||
        pe->OptionalHeader.SizeOfImage != 6606446u) {
        status = -1;
        log_line("NOT APPLIED: unsupported loaded PE image.");
        return;
    }
    if (!CpuVsCpuInstall(image, &stub)) {
        status = -2;
        log_line("NOT APPLIED: code signature conflict or memory protection/allocation failure.");
        return;
    }
    status = 1;
    log_line("APPLIED: controller Continue gate hooked at RVA 0x7AC38.");
    log_line("Play Now / tournament next match: leave every controller in the middle for CPU vs CPU.");
}

LONG GetCpuVsCpuStatus(void) { return InterlockedCompareExchange(&status, 0, 0); }

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID reserved) {
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        self = module;
        DisableThreadLibraryCalls(module);
    }
    return TRUE;
}
