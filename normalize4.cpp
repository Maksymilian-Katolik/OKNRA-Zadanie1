#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <algorithm>
#include <iterator>
#include <vector>
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

std::string cleanCharsSTL(const std::string& input) {
    std::string output;

    std::transform(input.begin(), input.end(), std::back_inserter(output), [](char c) -> char {
        if (std::isspace(static_cast<unsigned char>(c))) {
            return ' ';
        } else if (c >= 32 && c <= 126) {
            if (isPunctuation(c)) return ',';
            return std::tolower(static_cast<unsigned char>(c));
        }
        return '\0'; // jeśli nie spełnia warunku - to znak pusty który usuwam potem
    });

    // usunięcie '\0' 
    output.erase(std::remove(output.begin(), output.end(), '\0'), output.end());
    return output;
}


std::string removeExtraWhitespaceSTL(const std::string& input) {
    std::ostringstream oss;
    bool inSpace = false;

    std::for_each(input.begin(), input.end(), [&](char c) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!inSpace) {
                oss << ' ';
                inSpace = true;
            }
        } else {
            oss << c;
            inSpace = false;
        }
    });

    return oss.str();
}

std::string removeAdjacentDuplicatesSTL(const std::string& input) {
    std::string result;
    std::string prev;
    bool first = true;

    // iteratory po całym łańcuchu wejściowym
    auto it = input.begin();
    while (it != input.end()) {
        while (it != input.end() && std::isspace(static_cast<unsigned char>(*it))) {
            ++it;
        }

        std::string word;
        while (it != input.end() && !std::isspace(static_cast<unsigned char>(*it))) {
            word += *it;
            ++it;
        }

        if (!word.empty() && word != prev) {
            if (!first) result += ' ';
            result += word;
            prev = word;
            first = false;
        }
    }

    return result;
}

std::string normalizeText(const std::string& input) {
    std::string cleaned = cleanCharsSTL(input);
    std::string spaced = removeExtraWhitespaceSTL(cleaned);
    return removeAdjacentDuplicatesSTL(spaced);
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