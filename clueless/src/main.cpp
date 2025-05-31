#include <iostream>
#include <string>
#include <algorithm>
#include <cwchar>
#include <vector>
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <sstream>
#include <shlobj.h>

const int POINTS_PROCESS_DIRECT_MATCH = 3;
const int POINTS_WINDOW_TITLE_CLUELY = 2;
const int POINTS_CMD_LINE_CLUELY = 2;
const int POINTS_ONBOARDING_FILE = 1;
const int POINTS_REGISTRY_PROTOCOL = 1;
const int POINTS_RELATIVE_FINGERPRINT = 2;
const int DETECTION_THRESHOLD = 4;

const std::vector<std::wstring> CLUELY_TARGET_PROCESS_NAMES = {
    L"cluely.exe",
    L"electron.exe" 
};

const std::vector<std::wstring> CLUELY_REL_FILE_FINGERPRINTS = {
    L"package.json",
    L"out\\main\\index.js"
};

const std::vector<std::wstring> TARGET_WINDOW_TITLES_LOWER = {
    L"cluely",
    L"electron" 
};

const std::vector<std::wstring> ELECTRON_CMD_KEYWORDS_LOWER = {
    L"cluely",
    L"out\\main\\index.js"
};

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
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&callback_args));
    if (!callback_args.cluely_indicator_found) {
        std::wcout << L"no suspicious window titles found this cycle." << std::endl;
    }
    return callback_args.cluely_indicator_found;
}

std::wstring run_shell_command(const std::wstring& command) {
    wchar_t buffer[2048];
    std::wstring result = L"";
    FILE* pipe = _wpopen(command.c_str(), L"r");
    if (!pipe) {
        std::wcerr << L"error: _wpopen() failed for command: " << to_lower_wstring(command) << std::endl;
        return L"";
    }
    try {
        while (fgetws(buffer, sizeof(buffer)/sizeof(wchar_t), pipe)) {
            result += buffer;
        }
    } catch (...) {
        _pclose(pipe);
        throw; 
    }
    _pclose(pipe);
    return result;
}

bool check_electron_command_lines_wmic() {
    std::wcout << L"performing electron command line check (wmic)..." << std::endl;
    bool cluely_indicator_found = false;
    std::wstring wmic_command = L"wmic process where \"name='electron.exe' or name='cluely.exe'\" get processid,commandline /format:csv";
    std::wstring output = run_shell_command(wmic_command);
    if (output.empty()) {
        std::wcout << L"wmic command for electron/cluely processes yielded no output or failed." << std::endl;
        return false;
    }
    std::wstringstream ss(output);
    std::wstring line;
    bool header_skipped = false;
    while (std::getline(ss, line)) {
        if (line.empty() || line.find_first_not_of(L" \t\r\n\f\v") == std::wstring::npos) continue;
        std::wstring line_lower_for_header = to_lower_wstring(line);
        if (!header_skipped) {
            if (line_lower_for_header.find(L"commandline") != std::wstring::npos &&
                line_lower_for_header.find(L"processid") != std::wstring::npos &&
                line_lower_for_header.find(L"node") != std::wstring::npos) {
                header_skipped = true;
                continue;
            }
        }
        if (!header_skipped) continue; 
        size_t first_comma = line.find(L',');
        size_t second_comma = std::wstring::npos;
        if (first_comma != std::wstring::npos) {
             second_comma = line.find(L',', first_comma + 1);
        }
        if (first_comma != std::wstring::npos && second_comma != std::wstring::npos) {
            std::wstring cmd_line_part = line.substr(first_comma + 1, second_comma - (first_comma + 1));
            std::wstring pid_part = line.substr(second_comma + 1);
            if (!cmd_line_part.empty() && cmd_line_part.front() == L'\"' && cmd_line_part.back() == L'\"' && cmd_line_part.length() > 1) {
                cmd_line_part = cmd_line_part.substr(1, cmd_line_part.length() - 2);
            }
            if (!pid_part.empty() && pid_part.front() == L'\"' && pid_part.back() == L'\"' && pid_part.length() > 1) {
                pid_part = pid_part.substr(1, pid_part.length() - 2);
            }
            pid_part.erase(std::remove_if(pid_part.begin(), pid_part.end(), ::isspace), pid_part.end());
            cmd_line_part.erase(std::remove_if(cmd_line_part.begin(), cmd_line_part.end(), [](wchar_t c){ return c == L'\r' || c == L'\n'; }), cmd_line_part.end());
            std::wstring cmd_line_lower = to_lower_wstring(cmd_line_part);
            for (const auto& keyword : ELECTRON_CMD_KEYWORDS_LOWER) {
                if (cmd_line_lower.find(keyword) != std::wstring::npos) {
                    std::wcout << L"[cmdline check] suspicious command line (pid: " << pid_part
                               << L"). keyword: '" << keyword << L"'. full cmd: " << cmd_line_lower << std::endl;
                    cluely_indicator_found = true;
                    break; 
                }
            }
        }
        if (cluely_indicator_found && !ELECTRON_CMD_KEYWORDS_LOWER.empty()) break; 
    }
    if (!cluely_indicator_found) {
        std::wcout << L"no suspicious electron/cluely command lines found via wmic." << std::endl;
    }
    return cluely_indicator_found;
}

bool check_onboarding_file() {
    std::wcout << L"performing onboarding file check..." << std::endl;
    wchar_t path_raw[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path_raw))) {
        std::wstring full_path = std::wstring(path_raw) + L"\\cluely\\onboarding.done";
        DWORD file_attributes = GetFileAttributesW(full_path.c_str());
        if (file_attributes != INVALID_FILE_ATTRIBUTES && !(file_attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wcout << L"[file check] detected onboarding.done file at: " << to_lower_wstring(full_path) << std::endl;
            return true;
        }
    }
    std::wcout << L"onboarding.done file not found." << std::endl;
    return false;
}

bool check_cluely_protocol_registry() {
    std::wcout << L"performing cluely protocol registry check..." << std::endl;
    HKEY hkey;
    LONG result = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"cluely", 0, KEY_READ, &hkey);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hkey);
        std::wcout << L"[registry check] detected 'cluely' protocol handler registration." << std::endl;
        return true;
    }
    std::wcout << L"'cluely' protocol handler not registered." << std::endl;
    return false;
}

bool check_cluely_rel_file_fingerprints(const std::wstring& base_process_path) {
    std::wcout << L"performing relative file fingerprint check for: " << to_lower_wstring(base_process_path) << std::endl;
    if (base_process_path.empty()) {
        std::wcout << L"base process path is empty, skipping relative file fingerprint check." << std::endl;
        return false;
    }
    size_t last_slash = base_process_path.find_last_of(L"\\");
    if (last_slash == std::wstring::npos) {
        std::wcout << L"could not determine base directory from path: " << to_lower_wstring(base_process_path) << std::endl;
        return false;
    }
    std::wstring base_dir = base_process_path.substr(0, last_slash);
    bool found_fingerprint = false;
    for (const auto& rel_path : CLUELY_REL_FILE_FINGERPRINTS) {
        std::wstring full_check_path = base_dir + L"\\" + rel_path;
        DWORD file_attributes = GetFileAttributesW(full_check_path.c_str());
        if (file_attributes != INVALID_FILE_ATTRIBUTES && !(file_attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wcout << L"[rel file check] detected fingerprint file: " << to_lower_wstring(full_check_path)
                       << L" (relative to " << to_lower_wstring(base_process_path) << L")" << std::endl;
            found_fingerprint = true;
        }
    }
    if (!found_fingerprint) {
        std::wcout << L"no specific relative file fingerprints found for " << to_lower_wstring(base_process_path) << std::endl;
    }
    return found_fingerprint;
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
    std::wcout << L"--- clueless detector initializing ---" << std::endl;
    int total_detection_score = 0;
    ProcessInfo identified_cluely_process_info;

    std::wcout << L"\nchecking for target process names..." << std::endl;
    for (const auto& target_name : CLUELY_TARGET_PROCESS_NAMES) {
        ProcessInfo p_info = check_specific_process(target_name);
        if (p_info.found) {
            total_detection_score += POINTS_PROCESS_DIRECT_MATCH;
            if (!identified_cluely_process_info.found) { 
                identified_cluely_process_info = p_info;
            }
            std::wcout << L"contribution to score: +" << POINTS_PROCESS_DIRECT_MATCH << std::endl;
            break; 
        }
    }
    if (!identified_cluely_process_info.found) {
        std::wcout << L"no primary cluely process name detected." << std::endl;
    }

    std::wcout << L"\nchecking window titles..." << std::endl;
    if (check_window_titles()) {
        total_detection_score += POINTS_WINDOW_TITLE_CLUELY;
        std::wcout << L"contribution to score: +" << POINTS_WINDOW_TITLE_CLUELY << std::endl;
    }

    std::wcout << L"\nchecking command lines via wmic..." << std::endl;
    if (check_electron_command_lines_wmic()) {
        total_detection_score += POINTS_CMD_LINE_CLUELY;
        std::wcout << L"contribution to score: +" << POINTS_CMD_LINE_CLUELY << std::endl;
    }

    std::wcout << L"\nchecking for onboarding file..." << std::endl;
    if (check_onboarding_file()) {
        total_detection_score += POINTS_ONBOARDING_FILE;
        std::wcout << L"contribution to score: +" << POINTS_ONBOARDING_FILE << std::endl;
    }

    std::wcout << L"\nchecking for cluely protocol in registry..." << std::endl;
    if (check_cluely_protocol_registry()) {
        total_detection_score += POINTS_REGISTRY_PROTOCOL;
        std::wcout << L"contribution to score: +" << POINTS_REGISTRY_PROTOCOL << std::endl;
    }

    std::wcout << L"\nchecking relative file fingerprints..." << std::endl;
    if (identified_cluely_process_info.found && !identified_cluely_process_info.path.empty()){
        if (check_cluely_rel_file_fingerprints(identified_cluely_process_info.path)) {
            total_detection_score += POINTS_RELATIVE_FINGERPRINT;
            std::wcout << L"contribution to score: +" << POINTS_RELATIVE_FINGERPRINT << std::endl;
        }
    } else {
        std::wcout << L"skipping relative file fingerprint check: no confirmed cluely process path." << std::endl;
    }

    std::wcout << L"\n--- detection summary ---" << std::endl;
    std::wcout << L"final detection score: " << total_detection_score << std::endl;
    if (total_detection_score >= DETECTION_THRESHOLD) {
        std::wcout << L"conclusion: potential cluely activity detected!" << std::endl;
    } else {
        std::wcout << L"conclusion: cluely activity not conclusively detected based on score." << std::endl;
    }
    std::wcout << L"--- clueless detector finished ---" << std::endl;
    return 0;
}
