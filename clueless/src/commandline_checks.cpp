#include "commandline_checks.h"

const std::vector<std::wstring> ELECTRON_CMD_KEYWORDS_LOWER = {
    L"cluely",
    L"out\\main\\index.js"
};

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