# Inline Hooks: Exploration of API Hooking Techniques

This repository contains examples of Inline  hooking in C on Windows.

## Techniques Demonstrated

### 1. Return (RET) Hook (`src/ret_hook.c`)

This technique involves patching the very beginning of a target function to make it return immediately. This is often used to disable or bypass certain functionalities.

**How it works in `ret_hook.c`:**

- The `PatchEtw` function targets `EtwEventWrite` from `ntdll.dll`.
- `GetProcAddress` is used to find the memory address of `EtwEventWrite`.
- `VirtualProtect` is called to change the memory protection of the first byte of `EtwEventWrite` to `PAGE_EXECUTE_READWRITE`, allowing it to be modified.
- The first byte of `EtwEventWrite` is overwritten with the opcode `0xC3`, which is the x86 instruction for `RET` (return from procedure).
- `VirtualProtect` is called again to restore the original memory protection.
- After this patch, any call to `EtwEventWrite` will immediately return, effectively disabling its normal operation.

The `main` function in `ret_hook.c` demonstrates this by:
1. Patching `EtwEventWrite`.
2. Performing some network operations (connecting to google.com and sending a GET request) that would normally generate ETW events.
3. With `EtwEventWrite` patched, these events are not logged (or at least, the logging function returns before doing any significant work).

### 2. Relative Jump (JMP) Hook (`src/rel_jmp_hook.c`)

This is a common inline hooking method where the first few bytes of a target function are replaced with a `JMP` instruction that redirects execution to a custom (hooking) function.

**How it works in `src/rel_jmp_hook.c`:**

- The target function is `MessageBoxA` from `user32.dll`.
- A function pointer `OriginalMessageBoxA` is defined to store the address of the original `MessageBoxA`.
- A buffer `originalBytes[5]` is used to save the first 5 bytes of the original `MessageBoxA` function. These bytes will be overwritten by the JMP instruction.
- The `HookMessageBoxA` function performs the hooking:
    1. It gets the address of `MessageBoxA` using `GetProcAddress`.
    2. It uses `VirtualProtect` to make the first 5 bytes of `MessageBoxA` writable.
    3. It copies the original first 5 bytes of `MessageBoxA` into the `originalBytes` buffer.
    4. It overwrites the first 5 bytes of `MessageBoxA` with a relative `JMP` instruction (`0xE9` followed by a 4-byte relative offset) that points to `HookedMessageBoxA`. The relative offset is calculated as `(address of HookedMessageBoxA) - (address of MessageBoxA + 5)`.
    5. It restores the original memory protection.
- The `HookedMessageBoxA` function is our custom function that will be executed instead of the original `MessageBoxA`:
    1. **Unhook (Temporarily):** To call the original `MessageBoxA` without causing infinite recursion, it first restores the original 5 bytes to `MessageBoxA` using `memcpy` from `originalBytes`. This is done after changing memory protection with `VirtualProtect`.
    2. **Call Original Function:** It then calls `OriginalMessageBoxA` (which now points to the actual, temporarily unpatched, `MessageBoxA`), but with modified arguments (e.g., changing the displayed text).
    3. **Re-hook:** After the original function returns, it re-applies the `JMP` hook to `MessageBoxA` so that future calls are still intercepted. This also involves changing memory protection.
- The `main` function demonstrates this by:
    1. Calling `MessageBoxA` once before the hook is applied.
    2. Calling `HookMessageBoxA()` to install the hook.
    3. Calling `MessageBoxA` again. This time, `HookedMessageBoxA` will execute, displaying a modified message.


## Disclaimer
These examples are for educational purposes to understand API hooking mechanisms. API hooking can be used for legitimate purposes (e.g., debugging, extending software functionality) but also for malicious purposes (e.g., malware, cheating in games). Use this knowledge responsibly.
