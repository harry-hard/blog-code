#include <windows.h>

// DLL 入口函数
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        // 当 DLL 被加载时弹出 MessageBoxA
        MessageBoxA(NULL, "DLL 注入成功！", "Hello from DLL", MB_OK | MB_ICONINFORMATION);
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}