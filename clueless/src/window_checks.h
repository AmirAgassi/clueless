#ifndef WINDOW_CHECKS_H
#define WINDOW_CHECKS_H

#include "common_includes.h"
#include "utils.h"
#include "process_checks.h" // For ProcessInfo, GetProcessPath, CreateToolhelp32Snapshot

struct EnumWindowsCallbackArgs {
    const std::vector<std::wstring>* target_window_titles_lower;
    bool cluely_indicator_found;
    std::vector<std::wstring>* found_process_names_for_titles; // Currently unused but kept from previous version
};

extern const std::vector<std::wstring> TARGET_WINDOW_TITLES_LOWER;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
bool check_window_titles();

#endif // WINDOW_CHECKS_H 