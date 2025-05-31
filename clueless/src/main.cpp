#include <iostream>
#include <string>
#include <algorithm>
#include <cwchar>
#include <vector>
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>

struct ProcessInfo {
    bool found = false;
    DWORD pid = 0;
    std::wstring path;
    std::wstring name;
};

std::wstring to_lower_wstring(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](wchar_t c){ return static_cast<wchar_t>(std::tolower(c)); });
    return s;
}

std::wstring get_process_path(DWORD process_id) {
    wchar_t process_path[MAX_PATH] = {0};
    HANDLE h_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
    if (h_process) {
        if (GetModuleFileNameExW(h_process, NULL, process_path, MAX_PATH)) {
            CloseHandle(h_process);
            return std::wstring(process_path);
        }
        CloseHandle(h_process);
    }
    return L"";
}

ProcessInfo check_specific_process(const std::wstring& process_name_to_find_lower) {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);
    ProcessInfo p_info;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        return p_info;
    }

    if (Process32FirstW(snapshot, &entry)) {
        do {
            std::wstring current_process_name_lower = to_lower_wstring(std::wstring(entry.szExeFile));
            if (current_process_name_lower.find(process_name_to_find_lower) != std::wstring::npos) {
                p_info.found = true;
                p_info.pid = entry.th32ProcessID;
                p_info.name = std::wstring(entry.szExeFile);
                p_info.path = get_process_path(entry.th32ProcessID);
                std::wcout << L"[process check] detected process: " << to_lower_wstring(p_info.name)
                           << L" (pid: " << p_info.pid << L")"
                           << L" matching target: " << process_name_to_find_lower << std::endl;
                if (!p_info.path.empty()) {
                    std::wcout << L"                path: " << to_lower_wstring(p_info.path) << std::endl;
                }
                CloseHandle(snapshot);
                return p_info;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return p_info;
}

void list_processes_basic() {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        std::wcerr << L"failed to create snapshot of processes. error: " << GetLastError() << std::endl;
        return;
    }

    if (Process32FirstW(snapshot, &entry)) {
        std::wcout << L"--- running processes ---" << std::endl;
        int count = 0;
        do {
            std::wstring process_name = entry.szExeFile;
            std::wcout << L"process: " << to_lower_wstring(process_name) << L" (pid: " << entry.th32ProcessID << L")" << std::endl;
            count++;
            if (count >= 20) { 
                std::wcout << L"... (and more processes)" << std::endl;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
        std::wcout << L"-----------------------" << std::endl;
    } else {
        std::wcerr << L"failed to retrieve first process. error: " << GetLastError() << std::endl;
    }

    CloseHandle(snapshot);
}

int main() {
    std::wcout << L"hello, clueless! (now with specific process check)" << std::endl;
    
    std::wcout << L"performing specific process name check for 'notepad.exe' (example)..." << std::endl;
    ProcessInfo notepad_info = check_specific_process(L"notepad.exe");
    if (notepad_info.found) {
        std::wcout << L"'notepad.exe' found." << std::endl;
    } else {
        std::wcout << L"'notepad.exe' not found." << std::endl;
    }

    std::wcout << L"performing specific process name check for 'explorer.exe' (example)..." << std::endl;
    ProcessInfo explorer_info = check_specific_process(L"explorer.exe");
    if (explorer_info.found) {
        std::wcout << L"'explorer.exe' found." << std::endl;
    } else {
        std::wcout << L"'explorer.exe' not found." << std::endl;
    }
    
    std::wcout << L"specific process check complete." << std::endl;
    return 0;
}
