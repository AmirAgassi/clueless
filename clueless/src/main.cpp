#include <iostream>
#include <string>
#include <algorithm>
#include <cwchar>
#include <vector>
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>

// const std::vector<std::wstring> TARGET_WINDOW_TITLES_LOWER = {
//     L"cluely",
//     L"electron"
// };
const std::vector<std::wstring> TARGET_WINDOW_TITLES_LOWER = {
    L"cluely",
    L"electron",
    L"visual studio code",
    L"calculator"
};

// struct EnumWindowsCallbackArgs {
//     const std::vector<std::wstring>* target_process_names_lower; // Will be used later for more specific checks
//     const std::vector<std::wstring>* target_window_titles_lower;
//     bool cluely_indicator_found;
// };
struct EnumWindowsCallbackArgs {
    const std::vector<std::wstring>* target_window_titles_lower;
    bool cluely_indicator_found;
    std::vector<std::wstring>* found_process_names_for_titles;
};

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

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    EnumWindowsCallbackArgs* args = reinterpret_cast<EnumWindowsCallbackArgs*>(lParam);
    wchar_t window_title_raw[256];
    if (IsWindowVisible(hwnd) && GetWindowTextW(hwnd, window_title_raw, sizeof(window_title_raw) / sizeof(wchar_t))) {
        if (wcslen(window_title_raw) == 0) return TRUE; 

        std::wstring title_lower = to_lower_wstring(std::wstring(window_title_raw));
        for (const auto& target_title : *args->target_window_titles_lower) {
            if (title_lower.find(target_title) != std::wstring::npos) {
                DWORD process_id = 0;
                GetWindowThreadProcessId(hwnd, &process_id);
                
                std::wstring process_name_from_pid = L"<unknown>";
                HANDLE h_process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                if (h_process_snapshot != INVALID_HANDLE_VALUE) {
                    PROCESSENTRY32W pe32;
                    pe32.dwSize = sizeof(PROCESSENTRY32W);
                    if (Process32FirstW(h_process_snapshot, &pe32)) {
                        do {
                            if (pe32.th32ProcessID == process_id) {
                                process_name_from_pid = pe32.szExeFile;
                                break;
                            }
                        } while (Process32NextW(h_process_snapshot, &pe32));
                    }
                    CloseHandle(h_process_snapshot);
                }

                std::wcout << L"[window check] suspicious window title found: '" << to_lower_wstring(std::wstring(window_title_raw))
                           << L"' (pid: " << process_id
                           << L", process: " << to_lower_wstring(process_name_from_pid)
                           << L") matching target: '" << target_title << L"'" << std::endl;
                args->cluely_indicator_found = true;
            }
        }
    }
    return TRUE; 
}

bool check_window_titles() {
    std::wcout << L"performing window title check..." << std::endl;
    EnumWindowsCallbackArgs callback_args;
    callback_args.target_window_titles_lower = &TARGET_WINDOW_TITLES_LOWER;
    callback_args.cluely_indicator_found = false;
    // callback_args.found_process_names_for_titles = new std::vector<std::wstring>(); // If collecting

    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&callback_args));

    if (!callback_args.cluely_indicator_found) {
        std::wcout << L"no suspicious window titles found this cycle." << std::endl;
    }
    // delete callback_args.found_process_names_for_titles; // if collecting
    return callback_args.cluely_indicator_found;
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
    std::wcout << L"hello, clueless! (now with window title check)" << std::endl;
    
    std::wcout << L"performing specific process name check for 'notepad.exe' (example)..." << std::endl;
    ProcessInfo notepad_info = check_specific_process(L"notepad.exe");
    if (notepad_info.found) {
        std::wcout << L"'notepad.exe' found by specific check." << std::endl;
    } else {
        std::wcout << L"'notepad.exe' not found by specific check." << std::endl;
    }

    std::wcout << L"performing specific process name check for 'explorer.exe' (example)..." << std::endl;
    ProcessInfo explorer_info = check_specific_process(L"explorer.exe");
    if (explorer_info.found) {
        std::wcout << L"'explorer.exe' found by specific check." << std::endl;
    } else {
        std::wcout << L"'explorer.exe' not found by specific check." << std::endl;
    }
    std::wcout << L"specific process check complete." << std::endl << std::endl;

    bool suspicious_titles_found = check_window_titles();
    if (suspicious_titles_found) {
        std::wcout << L"suspicious window title(s) detected." << std::endl;
    } else {
        std::wcout << L"no suspicious window titles detected by dedicated check." << std::endl;
    }

    std::wcout << L"all checks complete." << std::endl;
    return 0;
}
