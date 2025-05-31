#include <iostream>
#include <string>
#include <algorithm>
#include <cwchar>
#include <vector>
#include <windows.h>
#include <tlhelp32.h>

std::wstring to_lower_wstring(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](wchar_t c){ return static_cast<wchar_t>(std::tolower(c)); });
    return s;
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

    std::wcout << L"hello, clueless! (now with process listing)" << std::endl;
    list_processes_basic();
    std::wcout << L"process listing complete." << std::endl;
    return 0;
}
