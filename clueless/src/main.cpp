#include "common_includes.h"
#include "utils.h"
#include "process_checks.h"
#include "window_checks.h"
#include "commandline_checks.h"
#include "filesystem_checks.h"
#include "registry_checks.h"

const int POINTS_PROCESS_DIRECT_MATCH = 3;
const int POINTS_WINDOW_TITLE_CLUELY = 2;
const int POINTS_CMD_LINE_CLUELY = 2;
const int POINTS_ONBOARDING_FILE = 1;
const int POINTS_REGISTRY_PROTOCOL = 1;
const int POINTS_RELATIVE_FINGERPRINT = 2;
const int DETECTION_THRESHOLD = 4;

void print_score(const std::wstring& label, int points, bool found) {
    if (found) {
        std::wcout << L"  [+] " << to_lower_wstring(label) << L": +" << points << std::endl;
    } else {
        std::wcout << L"  [ ] " << to_lower_wstring(label) << std::endl;
    }
}

int main() {
    std::wcout << L"\n========== clueless detector ==========\n" << std::endl;
    int total_detection_score = 0;
    ProcessInfo identified_cluely_process_info;

    bool process_found = false;
    for (const auto& target_name : CLUELY_TARGET_PROCESS_NAMES) {
        ProcessInfo p_info = check_specific_process(target_name);
        if (p_info.found) {
            total_detection_score += POINTS_PROCESS_DIRECT_MATCH;
            if (!identified_cluely_process_info.found) { 
                identified_cluely_process_info = p_info;
            }
            process_found = true;
            break; 
        }
    }
    print_score(L"process name match", POINTS_PROCESS_DIRECT_MATCH, process_found);

    bool window_found = check_window_titles();
    print_score(L"suspicious window title", POINTS_WINDOW_TITLE_CLUELY, window_found);
    if (window_found) {
        total_detection_score += POINTS_WINDOW_TITLE_CLUELY;
    }

    bool cmdline_found = check_electron_command_lines_wmic();
    print_score(L"suspicious command line", POINTS_CMD_LINE_CLUELY, cmdline_found);
    if (cmdline_found) {
        total_detection_score += POINTS_CMD_LINE_CLUELY;
    }

    bool onboarding_found = check_onboarding_file();
    print_score(L"onboarding file", POINTS_ONBOARDING_FILE, onboarding_found);
    if (onboarding_found) {
        total_detection_score += POINTS_ONBOARDING_FILE;
    }

    bool registry_found = check_cluely_protocol_registry();
    print_score(L"registry protocol", POINTS_REGISTRY_PROTOCOL, registry_found);
    if (registry_found) {
        total_detection_score += POINTS_REGISTRY_PROTOCOL;
    }

    bool rel_fingerprint_found = false;
    if (identified_cluely_process_info.found && !identified_cluely_process_info.path.empty()) {
        rel_fingerprint_found = check_cluely_rel_file_fingerprints(identified_cluely_process_info.path);
    }
    print_score(L"relative file fingerprint", POINTS_RELATIVE_FINGERPRINT, rel_fingerprint_found);
    if (rel_fingerprint_found) {
        total_detection_score += POINTS_RELATIVE_FINGERPRINT;
    }

    std::wcout << L"\n============== summary ==============" << std::endl;
    std::wcout << L"  detection score: " << total_detection_score << L" / (threshold: " << DETECTION_THRESHOLD << L")" << std::endl;
    if (total_detection_score >= DETECTION_THRESHOLD) {
        std::wcout << L"  => potential cluely activity detected!" << std::endl;
    } else {
        std::wcout << L"  => no conclusive cluely activity detected." << std::endl;
    }
    std::wcout << L"=====================================\n" << std::endl;
    return 0;
}
