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
