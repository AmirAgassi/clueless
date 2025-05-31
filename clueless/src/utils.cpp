#include "utils.h"

std::wstring to_lower_wstring(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](wchar_t c){ return static_cast<wchar_t>(std::tolower(c)); });
    return s;
} 