#include <Windows.h>
#include <AccCtrl.h>
#include <AclAPI.h>
#include <Sddl.h>
#include <TlHelp32.h>
#include <filesystem>
#include <iostream>
#include <string>

namespace {
    constexpr wchar_t kProcessName[] = L"Minecraft.Windows.exe"; 
    constexpr wchar_t kDllName[] = L"NebulaClient.dll";   
    constexpr wchar_t kConsoleTitle[] = L"Nebula Client Injector v1.0"; 

    void reportError(const std::wstring& what) {
        std::wcerr << L"[-] " << what << L" failed (error " << GetLastError() << L")\n";
    }

    DWORD findProcess(const wchar_t* name) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return 0;
        PROCESSENTRY32W entry{};
        entry.dwSize = sizeof(entry);
        DWORD pid = 0;
        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (_wcsicmp(entry.szExeFile, name) == 0) {
                    pid = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }
        CloseHandle(snapshot);
        return pid;
    }

    bool alreadyLoaded(DWORD pid, const std::wstring& dllName) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snapshot == INVALID_HANDLE_VALUE) return false;
        MODULEENTRY32W entry{};
        entry.dwSize = sizeof(entry);
        bool found = false;
        if (Module32FirstW(snapshot, &entry)) {
            do {
                if (_wcsicmp(entry.szModule, dllName.c_str()) == 0) {
                    found = true;
                    break;
                }
            } while (Module32NextW(snapshot, &entry));
        }
        CloseHandle(snapshot);
        return found;
    }

    bool grantAppContainerAccess(const std::wstring& path) {
        PSID sid = nullptr;
        if (!ConvertStringSidToSidW(L"S-1-15-2-1", &sid)) return false;
        PACL oldAcl = nullptr;
        PSECURITY_DESCRIPTOR descriptor = nullptr;
        DWORD status = GetNamedSecurityInfoW(path.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, &oldAcl, nullptr, &descriptor);
        if (status != ERROR_SUCCESS) { LocalFree(sid); return false; }
        EXPLICIT_ACCESS_W access{};
        access.grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
        access.grfAccessMode = GRANT_ACCESS;
        access.grfInheritance = NO_INHERITANCE;
        access.Trustee.TrusteeForm = TRUSTEE_IS_SID;
        access.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        access.Trustee.ptstrName = static_cast<LPWSTR>(sid);
        PACL newAcl = nullptr;
        status = SetEntriesInAclW(1, &access, oldAcl, &newAcl);
        if (status == ERROR_SUCCESS) {
            status = SetNamedSecurityInfoW(const_cast<LPWSTR>(path.c_str()), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, newAcl, nullptr);
        }
        if (newAcl) LocalFree(newAcl);
        if (descriptor) LocalFree(descriptor);
        LocalFree(sid);
        return status == ERROR_SUCCESS;
    }

    bool inject(DWORD pid, const std::wstring& dllPath) {
        HANDLE process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
        if (!process) return false;
        size_t bytes = (dllPath.size() + 1) * sizeof(wchar_t);
        void* remote = VirtualAllocEx(process, nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!remote) { CloseHandle(process); return false; }
        bool ok = WriteProcessMemory(process, remote, dllPath.c_str(), bytes, nullptr) != 0;
        if (ok) {
            auto loadLibrary = reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"));
            HANDLE thread = CreateRemoteThread(process, nullptr, 0, loadLibrary, remote, 0, nullptr);
            if (!thread) ok = false;
            else { WaitForSingleObject(thread, 10000); CloseHandle(thread); }
        }
        VirtualFreeEx(process, remote, 0, MEM_RELEASE);
        CloseHandle(process);
        return ok;
    }
}

int wmain() {
    SetConsoleTitleW(kConsoleTitle);
    std::wcout << kConsoleTitle << L"\n\n";
    DWORD pid = findProcess(kProcessName);
    if (!pid) { std::wcerr << L"[-] " << kProcessName << L" is not running.\n"; system("pause"); return 1; }
    if (alreadyLoaded(pid, kDllName)) { std::wcerr << L"[-] DLL already loaded.\n"; system("pause"); return 1; }
    wchar_t exePath[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::filesystem::path dllPath = std::filesystem::path(exePath).parent_path() / kDllName;
    if (!std::filesystem::exists(dllPath)) { std::wcerr << L"[-] DLL not found next to injector.\n"; system("pause"); return 1; }
    if (!grantAppContainerAccess(dllPath.wstring())) { std::wcerr << L"[-] AppContainer access failed.\n"; system("pause"); return 1; }
    if (!inject(pid, dllPath.wstring())) { std::wcerr << L"[-] Injection failed.\n"; system("pause"); return 1; }
    std::wcout << L"[+] Success! Nebula Client injected.\n";
    Sleep(3000);
    return 0;
}
