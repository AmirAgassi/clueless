#ifndef COMMANDLINE_CHECKS_H
#define COMMANDLINE_CHECKS_H

#include "common_includes.h"
#include "utils.h"

extern const std::vector<std::wstring> ELECTRON_CMD_KEYWORDS_LOWER;

std::wstring run_shell_command(const std::wstring& command);
bool check_electron_command_lines_wmic();

#endif // COMMANDLINE_CHECKS_H 