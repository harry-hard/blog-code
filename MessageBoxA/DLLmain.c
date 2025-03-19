#include <windows.h>

// DLL ��ں���
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        // �� DLL ������ʱ���� MessageBoxA
        MessageBoxA(NULL, "DLL ע��ɹ���", "Hello from DLL", MB_OK | MB_ICONINFORMATION);
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}