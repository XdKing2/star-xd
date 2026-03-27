#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <iterator>

// ── Simple Base64 ─────────────────────────────────────────────
static const std::string B64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string b64encode(const std::string& in) {
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out += B64[(val >> valb) & 0x3F];
            valb -= 6;
        }
    }
    if (valb > -6) out += B64[((val << 8) >> (valb + 8)) & 0x3F];
    while (out.size() % 4) out += '=';
    return out;
}

std::string b64decode(const std::string& in) {
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[(unsigned char)B64[i]] = i;
    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out += (char)((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return out;
}

// ── RLE on raw bytes ──────────────────────────────────────────
// Format: each run = [count(1 byte)][value(1 byte)]
// Max run = 255
std::string rleCompress(const std::string& data) {
    if (data.empty()) return "";
    std::string out;
    size_t i = 0;
    while (i < data.size()) {
        unsigned char cur = data[i];
        int count = 1;
        while (i + count < data.size() && (unsigned char)data[i + count] == cur && count < 255)
            count++;
        out += (char)count;
        out += (char)cur;
        i += count;
    }
    return out;
}

std::string rleDecompress(const std::string& data) {
    if (data.size() % 2 != 0) throw std::runtime_error("Invalid RLE data");
    std::string out;
    for (size_t i = 0; i < data.size(); i += 2) {
        int count = (unsigned char)data[i];
        char val  = data[i + 1];
        out.append(count, val);
    }
    return out;
}

// ── Stats ─────────────────────────────────────────────────────
void printStats(const std::string& original, const std::string& compressed, bool compress_mode) {
    if (compress_mode) {
        double ratio = original.empty() ? 0 : (1.0 - (double)compressed.size() / original.size()) * 100;
        std::cerr << "STATS|original=" << original.size()
                  << "|compressed=" << compressed.size()
                  << "|ratio=" << (int)ratio << "%" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: rle <compress|decompress> <text|file> [data]" << std::endl;
        return 1;
    }

    std::string mode     = argv[1];
    std::string datatype = argv[2]; // "text" or "file"
    bool compress_mode   = (mode == "compress");

    std::string input;
    if (datatype == "text") {
        for (int i = 3; i < argc; i++) {
            if (i > 3) input += " ";
            input += argv[i];
        }
    } else if (datatype == "file") {
        if (argc < 4) { std::cerr << "No file path provided" << std::endl; return 1; }
        std::ifstream f(argv[3], std::ios::binary);
        if (!f) { std::cerr << "Cannot open file: " << argv[3] << std::endl; return 1; }
        input = std::string(std::istreambuf_iterator<char>(f), {});
    } else {
        std::cerr << "Unknown data type: " << datatype << std::endl;
        return 1;
    }

    try {
        if (compress_mode) {
            std::string compressed = rleCompress(input);
            printStats(input, compressed, true);
            std::cout << b64encode(compressed) << std::endl;
        } else {
            std::string decoded = b64decode(input);
            std::string decompressed = rleDecompress(decoded);
            std::cout << decompressed;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
