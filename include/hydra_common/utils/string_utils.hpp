#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <map>
#include <cstdint>

namespace hydra { namespace common {

// --- Basic String Utilities ---

inline std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

inline std::string join(const std::vector<std::string>& tokens, const std::string& delimiter) {
    std::ostringstream joined;
    for (size_t i = 0; i < tokens.size(); ++i) {
        joined << tokens[i];
        if (i + 1 < tokens.size()) joined << delimiter;
    }
    return joined.str();
}

inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
        [](unsigned char ch) { return !std::isspace(ch); }));
}

inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
        [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

inline std::string trimmed(const std::string& s) {
    std::string copy = s;
    trim(copy);
    return copy;
}

inline bool starts_with(const std::string& str, const std::string& prefix) {
    return str.rfind(prefix, 0) == 0;
}

inline bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// --- String to Integer and Integer to String ---

template<typename T>
inline std::string to_string(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template<typename T>
inline T from_string(const std::string& str) {
    std::istringstream iss(str);
    T value;
    iss >> value;
    if (iss.fail()) {
        throw std::invalid_argument("Invalid conversion from string to number");
    }
    return value;
}

// --- Base64 Encoding/Decoding ---

inline std::string base64_encode(const std::string& input, bool padded = true) {
    static const char* base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::string output;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            output.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) output.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    if (padded) {
        while (output.size() % 4) output.push_back('=');
    }
    return output;
}

inline std::string base64_decode(const std::string& input) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;
    std::string output;
    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            output.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return output;
}

// --- Base32 Encoding/Decoding ---

inline std::string base32_encode(const std::string& input, bool padded = true) {
    static const char* base32_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    std::string output;
    int buffer = 0, bitsLeft = 0;
    for (unsigned char c : input) {
        buffer <<= 8;
        buffer |= c & 0xFF;
        bitsLeft += 8;
        while (bitsLeft >= 5) {
            output.push_back(base32_chars[(buffer >> (bitsLeft - 5)) & 0x1F]);
            bitsLeft -= 5;
        }
    }
    if (bitsLeft > 0) {
        buffer <<= (5 - bitsLeft);
        output.push_back(base32_chars[buffer & 0x1F]);
    }
    if (padded) {
        while (output.size() % 8) output.push_back('=');
    }
    return output;
}

inline std::string base32_decode(const std::string& input) {
    static const std::map<char, int> base32_map = {
        {'A', 0}, {'B', 1}, {'C', 2}, {'D', 3},
        {'E', 4}, {'F', 5}, {'G', 6}, {'H', 7},
        {'I', 8}, {'J', 9}, {'K',10}, {'L',11},
        {'M',12}, {'N',13}, {'O',14}, {'P',15},
        {'Q',16}, {'R',17}, {'S',18}, {'T',19},
        {'U',20}, {'V',21}, {'W',22}, {'X',23},
        {'Y',24}, {'Z',25}, {'2',26}, {'3',27},
        {'4',28}, {'5',29}, {'6',30}, {'7',31}
    };
    int buffer = 0, bitsLeft = 0;
    std::string output;
    for (char c : input) {
        if (c == '=') break;
        c = toupper(c);
        if (base32_map.find(c) == base32_map.end()) continue;
        buffer <<= 5;
        buffer |= base32_map.at(c);
        bitsLeft += 5;
        if (bitsLeft >= 8) {
            output.push_back((char)((buffer >> (bitsLeft - 8)) & 0xFF));
            bitsLeft -= 8;
        }
    }
    return output;
}

}} // namespace hydra::common
