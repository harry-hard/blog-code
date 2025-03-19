//code by harry hard
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h> 

// 根据进程名获取PID
DWORD GetProcessIdByName(const char* processName) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateToolhelp32Snapshot 失败 (错误码: %lu)\n", GetLastError());
        return 0;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &processEntry)) {
        do {
            wprintf(L"%s\n", processEntry.szExeFile);
            if (_wcsicmp(processEntry.szExeFile, processName) == 0) {
                pid = processEntry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &processEntry));
    }
    else {
        fprintf(stderr, "Process32First 失败 (错误码: %lu)\n", GetLastError());
    }

    CloseHandle(snapshot);
    return pid;
}

// DLL注入函数
BOOL InjectDLL(DWORD pid, const char* dllPath) {
    HANDLE hProcess = NULL;
    LPVOID remoteMemory = NULL;
    HANDLE hRemoteThread = NULL;
    BOOL success = FALSE;

    // 1. 打开目标进程
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        fprintf(stderr, "OpenProcess 失败 (错误码: %lu)\n", GetLastError());
        goto cleanup;
    }

    // 2. 计算路径长度（包含空终止符）
    SIZE_T pathSize = strlen(dllPath) + 1;

    // 3. 分配远程内存
    remoteMemory = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT, PAGE_READWRITE);
    if (remoteMemory == NULL) {
        fprintf(stderr, "VirtualAllocEx 失败 (错误码: %lu)\n", GetLastError());
        goto cleanup;
    }

    // 4. 写入DLL路径
    if (!WriteProcessMemory(hProcess, remoteMemory, dllPath, pathSize, NULL)) {
        fprintf(stderr, "WriteProcessMemory 失败 (错误码: %lu)\n", GetLastError());
        goto cleanup;
    }

    // 5. 获取LoadLibraryA地址
    LPTHREAD_START_ROUTINE loadLibraryAddr = (LPTHREAD_START_ROUTINE)GetProcAddress(
        GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (loadLibraryAddr == NULL) {
        fprintf(stderr, "GetProcAddress 失败 (错误码: %lu)\n", GetLastError());
        goto cleanup;
    }

    // 6. 创建远程线程
    hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, loadLibraryAddr, remoteMemory, 0, NULL);
    if (hRemoteThread == NULL) {
        fprintf(stderr, "CreateRemoteThread 失败 (错误码: %lu)\n", GetLastError());
        goto cleanup;
    }

    // 7. 等待线程完成
    WaitForSingleObject(hRemoteThread, INFINITE);
    success = TRUE;

cleanup:
    // 清理资源
    if (remoteMemory != NULL) {
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
    }
    if (hRemoteThread != NULL) {
        CloseHandle(hRemoteThread);
    }
    if (hProcess != NULL) {
        CloseHandle(hProcess);
    }
    return success;
}

int main() {
    const wchar_t* targetProcess = L"Notepad.exe";
    const char* dllPath = "C:\\Users\\otis\\Documents\\mal_exclude\\maldev_program\\DLL injection\\MessageBoxA\\x64\\Release\\myMessageBox.dll";  // 替换为实际路径

    DWORD pid = GetProcessIdByName(targetProcess);
    if (pid == 0) {
        fprintf(stderr, "未找到进程: %s\n", targetProcess);
        getchar();
        return 1;
    }

    printf("正在注入PID: %lu...\n", pid);
    getchar();
    if (InjectDLL(pid, dllPath)) {
        printf("注入成功!\n");
        getchar();
        return 0;
    }
    else {
        fprintf(stderr, "注入失败\n");
        getchar();
        return 1;
    }
}