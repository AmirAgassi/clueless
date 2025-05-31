#include "registry_checks.h"

bool check_cluely_protocol_registry() {
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