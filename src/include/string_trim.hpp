#ifndef STRING_TRIM_HPP
#define STRING_TRIM_HPP

#include <algorithm>
#include <cctype>
#include <locale>
#include <string_view>

// trim from start (in place)
static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
}

static inline std::string_view ltrim_sv(const std::string_view& sv) {
    const auto lst = std::find_if(sv.begin(), sv.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    });
    return sv.substr(0, lst - sv.begin());
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}

static inline std::string_view rtrim_sv(const std::string_view& sv) {
    const auto fst = std::find_if(sv.rbegin(), sv.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    });
    return sv.substr(sv.rend() - fst, fst - sv.rbegin());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
    rtrim(s);
    ltrim(s);
}

static inline std::string_view trim_sv(const std::string_view& sv) {
    return ltrim_sv(rtrim_sv(sv));
}

#endif  // STRING_TRIM_HPP