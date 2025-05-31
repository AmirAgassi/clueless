#ifndef PROCESS_CHECKS_H
#define PROCESS_CHECKS_H

#include "common_includes.h"
#include "utils.h"

struct ProcessInfo {
    bool found = false;
    DWORD pid = 0;
    std::wstring path;
    std::wstring name;
};

extern const std::vector<std::wstring> CLUELY_TARGET_PROCESS_NAMES;

std::wstring get_process_path(DWORD process_id);
ProcessInfo check_specific_process(const std::wstring& process_name_to_find_lower);
void list_processes_basic();

#endif // PROCESS_CHECKS_H 