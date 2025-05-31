#include "filesystem_checks.h"

const std::vector<std::wstring> CLUELY_REL_FILE_FINGERPRINTS = {
    L"package.json",
    L"out\\main\\index.js"
};

bool check_onboarding_file() {
    std::wcout << L"performing onboarding file check..." << std::endl;
    wchar_t path_raw[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path_raw))) {
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