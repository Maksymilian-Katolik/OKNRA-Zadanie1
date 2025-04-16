#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <cstring>
#include <sys/time.h>
// wersja bez memcpy

static double gtod_ref_time_sec = 0.0;

/* Adapted from the bl2_clock() routine in the BLIS library */
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

char* cleanChars(const char* input, size_t len, size_t& outLen) {
    char* buffer = new char[len];
    size_t j = 0;

    for (size_t i = 0; i < len; ++i) {
        char c = input[i];
        if (std::isspace(static_cast<unsigned char>(c))) {
            buffer[j++] = ' ';
        } else if (c >= 32 && c <= 126) {
            if (isPunctuation(c)) {
                buffer[j++] = ',';
            } else {
                buffer[j++] = std::tolower(static_cast<unsigned char>(c));
            }
        }
    }
    outLen = j;
    return buffer;
}

char* removeWS(const char* input, size_t len, size_t& outLen) {
    char* buffer = new char[len];
    size_t j = 0;
    bool inWhitespace = false;

    for (size_t i = 0; i < len; ++i) {
        char c = input[i];
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!inWhitespace) {
                buffer[j++] = ' ';
                inWhitespace = true;
            }
        } else {
            buffer[j++] = c;
            inWhitespace = false;
        }
    }
    outLen = j;
    return buffer;
}

std::string removeDuplicates(const char* input, size_t len) {
    std::string result;

    const char* start = input;
    const char* end = input + len;
    const char* prev_start = nullptr;
    std::string prev;

    while (start < end) {
        while (start < end && std::isspace(static_cast<unsigned char>(*start))) {
            ++start;
        }

        if (start == end) break;

        const char* word_end = start;
        while (word_end < end && !std::isspace(static_cast<unsigned char>(*word_end))) {
            ++word_end;
        }

        std::string word(start, word_end);

        if (!word.empty() && word != prev) {
            if (!result.empty()) result += ' ';
            result += word;
            prev = word;
        }

        start = word_end;
    }

    return result;
}

std::string normalizeText(const std::string& input) {
    size_t len1;
    char* cleaned = cleanChars(input.c_str(), input.size(), len1);

    size_t len2;
    char* spaced = removeWS(cleaned, len1, len2);

    std::string result = removeDuplicates(spaced, len2);

    delete[] cleaned;
    delete[] spaced;

    return result;
}

int main() {
    double dtime;

    std::ifstream inFile("text.txt");
    if (!inFile) {
        std::cerr << "Nie możcna otworzyć pliku!" << std::endl;
        return 1;
    }

    std::ostringstream buffer;
    buffer << inFile.rdbuf();
    std::string inputText = buffer.str();
    inFile.close();

    dtime = dclock();

    std::string result;
    for (unsigned long long i = 0; i < repeats; ++i) {
        result = normalizeText(inputText);
    }

    dtime = dclock() - dtime;

    std::cout << "Time: " << dtime << "\n";

    std::ofstream outFile("output.txt");
    outFile << result;
    outFile.close();

    fflush(stdout);
    return 0;
}
