#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <stdexcept>

// Each byte is encoded as 4 DNA bases (2 bits per base)
// 00=A, 01=T, 10=C, 11=G
const char BASES[4] = {'A', 'T', 'C', 'G'};
const std::map<char, int> BASE_VAL = {{'A',0},{'T',1},{'C',2},{'G',3}};

std::string encode(const std::string& text) {
    std::string result;
    result.reserve(text.size() * 4);
    for (unsigned char c : text) {
        result += BASES[(c >> 6) & 3];
        result += BASES[(c >> 4) & 3];
        result += BASES[(c >> 2) & 3];
        result += BASES[c & 3];
    }
    return result;
}

std::string decode(const std::string& dna) {
    if (dna.size() % 4 != 0) throw std::runtime_error("Invalid DNA length");
    std::string result;
    result.reserve(dna.size() / 4);
    for (size_t i = 0; i < dna.size(); i += 4) {
        unsigned char c = 0;
        for (int j = 0; j < 4; j++) {
            auto it = BASE_VAL.find(std::toupper(dna[i+j]));
            if (it == BASE_VAL.end()) throw std::runtime_error("Invalid base: " + std::string(1, dna[i+j]));
            c = (c << 2) | it->second;
        }
        result += c;
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: dna <encode|decode> <text>" << std::endl;
        return 1;
    }
    std::string mode = argv[1];
    // Join all args after mode as text (handles spaces)
    std::string input;
    for (int i = 2; i < argc; i++) {
        if (i > 2) input += " ";
        input += argv[i];
    }
    try {
        if (mode == "encode") {
            std::cout << encode(input) << std::endl;
        } else if (mode == "decode") {
            std::cout << decode(input) << std::endl;
        } else {
            std::cerr << "Unknown mode: " << mode << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
