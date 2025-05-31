#include "window_checks.h"

const std::vector<std::wstring> TARGET_WINDOW_TITLES_LOWER = {
    L"cluely",
    L"electron"
};

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
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&callback_args));
    if (!callback_args.cluely_indicator_found) {
        std::wcout << L"no suspicious window titles found this cycle." << std::endl;
    }
    return callback_args.cluely_indicator_found;
} 