#include <windows.h>
#include <stdio.h>

// Function pointer to hold original MessageBoxA
typedef int (WINAPI *MESSAGEBOXA)(HWND, LPCSTR, LPCSTR, UINT);
MESSAGEBOXA OriginalMessageBoxA;

// Buffer to save original first 5 bytes
BYTE originalBytes[5];

// Hooked function
int WINAPI HookedMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    // Temporarily unhook to avoid infinite recursion
    DWORD oldProtect;
    VirtualProtect(OriginalMessageBoxA, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(OriginalMessageBoxA, originalBytes, 5);
    VirtualProtect(OriginalMessageBoxA, 5, oldProtect, &oldProtect);

    // Call original with modified text
    int ret = OriginalMessageBoxA(hWnd, ">> HOOKED <<\nThis message has been intercepted.", lpCaption, uType);
    for (int i = 0; i < 5; i++) {
        printf("%02x ", originalBytes[i]); // %02x ensures 2-digit hex with leading zeros
    }
    // Re-hook
    VirtualProtect(OriginalMessageBoxA, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    DWORD relativeAddr = (DWORD)HookedMessageBoxA - (DWORD)OriginalMessageBoxA - 5;
    ((BYTE*)OriginalMessageBoxA)[0] = 0xE9;
    *((DWORD*)((BYTE*)OriginalMessageBoxA + 1)) = relativeAddr;
    VirtualProtect(OriginalMessageBoxA, 5, oldProtect, &oldProtect);

    return ret;
}

// Hooking logic
void HookMessageBoxA() {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (!hUser32) {
        MessageBoxA(NULL, "Failed to load user32.dll", "Error", MB_OK);
        return;
    }

    FARPROC addr = GetProcAddress(hUser32, "MessageBoxA");
    if (!addr) {
        MessageBoxA(NULL, "Failed to get MessageBoxA address", "Error", MB_OK);
        return;
    }

    OriginalMessageBoxA = (MESSAGEBOXA)addr;

    DWORD oldProtect;
    VirtualProtect(addr, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

    memcpy(originalBytes, addr, 5); // Save original instructions

    // Write JMP to HookedMessageBoxA
    DWORD relativeAddr = (DWORD)HookedMessageBoxA - (DWORD)addr - 5;
    ((BYTE*)addr)[0] = 0xE9;
    *((DWORD*)((BYTE*)addr + 1)) = relativeAddr;

    VirtualProtect(addr, 5, oldProtect, &oldProtect);
}

int main() {
    // Call original MessageBoxA first
    MessageBoxA(NULL, "This is the original MessageBoxA", "Before Hook", MB_OK);

    // Set the hook
    HookMessageBoxA();

    // Call again after hook
    MessageBoxA(NULL, "You should not see this text", "After Hook", MB_OK);
    Sleep(60000);
    return 0;
}
