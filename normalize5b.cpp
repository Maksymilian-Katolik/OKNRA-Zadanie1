#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <sys/time.h>

static double gtod_ref_time_sec = 0.0;

double dclock() {
    double the_time, norm_sec;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if (gtod_ref_time_sec == 0.0)
        gtod_ref_time_sec = (double)tv.tv_sec;
    norm_sec = (double)tv.tv_sec - gtod_ref_time_sec;
    the_time = norm_sec + tv.tv_usec * 1.0e-6;
    return the_time;
}

constexpr unsigned long long repeats{5000};

bool isPunctuation(char c) {
    return std::ispunct(static_cast<unsigned char>(c)) && c != ',';
}

void cleanChars(const char* input, char* output, size_t& out_len) {
    const char* src = input;
    char* dst = output;
    while (*src) {
        unsigned char uc = static_cast<unsigned char>(*src);
        if (std::isspace(uc)) {
            *dst++ = ' ';
        } else if (uc >= 32 && uc <= 126) {
            if (isPunctuation(*src)) {
                *dst++ = ',';
            } else {
                *dst++ = std::tolower(uc);
            }
        }
        ++src;
    }
    *dst = '\0';
    out_len = dst - output;
}

void removeWS(const char* input, char* output, size_t& out_len) {
    const char* src = input;
    char* dst = output;
    bool inWhitespace = false;
    while (*src) {
        if (std::isspace(static_cast<unsigned char>(*src))) {
            if (!inWhitespace) {
                *dst++ = ' ';
                inWhitespace = true;
            }
        } else {
            *dst++ = *src;
            inWhitespace = false;
        }
        ++src;
    }
    *dst = '\0';
    out_len = dst - output;
}

void removeDuplicates(const char* input, char* output, size_t& out_len) {
    // wersja z memcpy
    const char* src = input;
    char* dst = output;
    const size_t max_word_len = 256;
    char prev[max_word_len]{};
    char word[max_word_len]{};
    size_t word_len = 0;

    while (*src) {
        if (*src == ' ') {
            ++src;
            continue;
        }

        word_len = 0;
        while (*src && *src != ' ' && word_len < max_word_len - 1) {
            word[word_len++] = *src++;
        }
        word[word_len] = '\0';

        if (std::strcmp(word, prev) != 0) {
            if (dst != output) *dst++ = ' ';
            std::memcpy(dst, word, word_len);
            dst += word_len;
            std::strcpy(prev, word);
        }
    }
    *dst = '\0';
    out_len = dst - output;
}

std::string normalizeTextBareMetal(const std::string& input) {
    size_t len = input.size();
    char* buffer1 = new char[len + 1];
    char* buffer2 = new char[len + 1];
    char* buffer3 = new char[len + 1];
    size_t len1, len2, len3;

    cleanChars(input.c_str(), buffer1, len1);
    removeWS(buffer1, buffer2, len2);
    removeDuplicates(buffer2, buffer3, len3);

    std::string result(buffer3, len3);

    delete[] buffer1;
    delete[] buffer2;
    delete[] buffer3;

    return result;
}

int main() {
    double dtime;

    std::ifstream inFile("text.txt"); 
    if (!inFile) {
        std::cerr << "Nie można otworzyć pliku!" << std::endl;
        return 1;
    }

    std::ostringstream buffer;
    buffer << inFile.rdbuf();
    std::string inputText = buffer.str();
    inFile.close();

    dtime = dclock();

    std::string result;

    for (unsigned long long i{}; i < repeats; i++) {
        result = normalizeTextBareMetal(inputText);
    }

    dtime = dclock() - dtime;

    std::cout << "Time: " << dtime << "\n";

    std::ofstream outFile("output.txt");
    outFile << result;
    outFile.close();

    fflush( stdout );

    return 0;
}




