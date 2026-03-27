#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>
#include <sstream>
#include <iomanip>

// ── Caesar ────────────────────────────────────────────────────
std::string caesar(const std::string& text, int shift, bool decrypt) {
    if (decrypt) shift = (26 - (shift % 26)) % 26;
    std::string result;
    for (char c : text) {
        if (std::isalpha(c)) {
            char base = std::isupper(c) ? 'A' : 'a';
            result += (char)((c - base + shift) % 26 + base);
        } else {
            result += c;
        }
    }
    return result;
}

// ── Vigenere ──────────────────────────────────────────────────
std::string vigenere(const std::string& text, const std::string& key, bool decrypt) {
    if (key.empty()) throw std::runtime_error("Key cannot be empty");
    std::string result;
    int ki = 0;
    for (char c : text) {
        if (std::isalpha(c)) {
            char base = std::isupper(c) ? 'A' : 'a';
            char kc = std::toupper(key[ki % key.size()]) - 'A';
            if (decrypt) {
                result += (char)((c - base - kc + 26) % 26 + base);
            } else {
                result += (char)((c - base + kc) % 26 + base);
            }
            ki++;
        } else {
            result += c;
        }
    }
    return result;
}

// ── XOR ───────────────────────────────────────────────────────
std::string xorCipher(const std::string& text, const std::string& key) {
    if (key.empty()) throw std::runtime_error("Key cannot be empty");
    std::string result;
    for (size_t i = 0; i < text.size(); i++) {
        result += (char)(text[i] ^ key[i % key.size()]);
    }
    return result;
}

std::string toHex(const std::string& s) {
    std::ostringstream oss;
    for (unsigned char c : s)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    return oss.str();
}

std::string fromHex(const std::string& hex) {
    if (hex.size() % 2 != 0) throw std::runtime_error("Invalid hex length");
    std::string result;
    for (size_t i = 0; i < hex.size(); i += 2) {
        result += (char)std::stoi(hex.substr(i, 2), nullptr, 16);
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: cipher <caesar|vigenere|xor> <encode|decode> <key> <text...>" << std::endl;
        return 1;
    }
    std::string cipher_type = argv[1];
    std::string mode        = argv[2];
    std::string key         = argv[3];
    std::string text;
    for (int i = 4; i < argc; i++) {
        if (i > 4) text += " ";
        text += argv[i];
    }
    bool decrypt = (mode == "decode" || mode == "decrypt");

    try {
        std::string result;
        if (cipher_type == "caesar") {
            int shift = std::stoi(key);
            result = caesar(text, shift, decrypt);
        } else if (cipher_type == "vigenere") {
            result = vigenere(text, key, decrypt);
        } else if (cipher_type == "xor") {
            if (decrypt) {
                result = xorCipher(fromHex(text), key);
            } else {
                result = toHex(xorCipher(text, key));
            }
        } else {
            std::cerr << "Unknown cipher: " << cipher_type << std::endl;
            return 1;
        }
        std::cout << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
