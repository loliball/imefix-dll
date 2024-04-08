#include <windows.h>
#include <string>
#include <imm.h>
#include "include/detours.h"

#pragma comment(lib, "detours.lib")
#pragma comment(lib, "imm32.lib")

void OutputDebugPrintf(const char *strOutputString, ...) {
    char strBuffer[4096] = {0};
    va_list vlArgs;

            va_start(vlArgs, strOutputString);
    _vsnprintf(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
            va_end(vlArgs);

    OutputDebugString(strBuffer);
//    printf("%s", strBuffer);
}

typedef BOOL (WINAPI *PeekMessageFP)(
        _Out_ LPMSG lpMsg,
        _In_opt_ HWND hWnd,
        _In_ UINT wMsgFilterMin,
        _In_ UINT wMsgFilterMax,
        _In_ UINT wRemoveMsg
);

PeekMessageFP peekMessageOld = (PeekMessageFP)
        DetourFindFunction("user32.dll", "PeekMessageW");

BOOL WINAPI PeekMessageNew(
        _Out_ LPMSG lpMsg,
        _In_opt_ HWND hWnd,
        _In_ UINT wMsgFilterMin,
        _In_ UINT wMsgFilterMax,
        _In_ UINT wRemoveMsg) {
    BOOL result = peekMessageOld(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    if (result != 0) {
        if (lpMsg->wParam == VK_PROCESSKEY) {
            lpMsg->wParam = ImmGetVirtualKey(lpMsg->hwnd);
        }
//        uint32_t msg = lpMsg->message;
//        if (msg == WM_KEYDOWN) {
//            OutputDebugPrintf("WM_KEYDOWN");
//        }
    }
    return result;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved) {
    UNREFERENCED_PARAMETER(lpReserved);
    HANDLE thread = GetCurrentThread();
    switch (Reason) {
        case DLL_PROCESS_ATTACH:
            DetourTransactionBegin();
            DetourUpdateThread(thread);
            DetourAttach(&(LPVOID &) peekMessageOld, (PVOID) PeekMessageNew);
            DetourTransactionCommit();
            break;
        case DLL_PROCESS_DETACH:
            DetourTransactionBegin();
            DetourUpdateThread(thread);
            DetourDetach(&(LPVOID &) peekMessageOld, (PVOID) PeekMessageNew);
            DetourTransactionCommit();
            break;
        default:
            break;
    }
    if (Reason == DLL_PROCESS_ATTACH || Reason == DLL_PROCESS_DETACH) {
        const char *injected = "dll inject success thread: %d, reason: %d\n";
        DWORD tid = GetCurrentThreadId();
        printf(injected, tid, Reason);
        OutputDebugPrintf(injected, tid, Reason);
    }
    return TRUE;
}
