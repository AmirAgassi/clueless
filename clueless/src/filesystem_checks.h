#ifndef FILESYSTEM_CHECKS_H
#define FILESYSTEM_CHECKS_H

#include "common_includes.h"
#include "utils.h"

extern const std::vector<std::wstring> CLUELY_REL_FILE_FINGERPRINTS;

bool check_onboarding_file();
bool check_cluely_rel_file_fingerprints(const std::wstring& base_process_path);

#endif // FILESYSTEM_CHECKS_H 