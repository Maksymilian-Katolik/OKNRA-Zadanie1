#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
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

void cleanCharsInPlace(std::string& text) {
    // będę tu iterował mo kązdym znaku w oryginalnym pliku zamiast alokowac dodatkową pamięć
    size_t writeIndex = 0;
    for (size_t readIndex = 0; readIndex < text.size(); ++readIndex) {
        char c = text[readIndex];

        if (std::isspace(static_cast<unsigned char>(c))) {
            text[writeIndex++] = ' ';
        }
        else if (c >= 32 && c <= 126) {
            if (isPunctuation(c)) {
                text[writeIndex++] = ',';
            } else {
                text[writeIndex++] = std::tolower(static_cast<unsigned char>(c));
            }
        }
    }
    text.resize(writeIndex);
}

void removeWSInPlace(std::string& text) {
    size_t writeIndex = 0;
    bool inWhitespace = false;

    for (size_t readIndex = 0; readIndex < text.size(); ++readIndex) {
        char c = text[readIndex];
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!inWhitespace) {
                text[writeIndex++] = ' ';
                inWhitespace = true;
            }
        } else {
            text[writeIndex++] = c;
            inWhitespace = false;
        }
    }

    // usuwanie końcowego białego znaku
    if (writeIndex > 0 && text[writeIndex - 1] == ' ')
        --writeIndex;

    text.resize(writeIndex);
}

void removeDuplicatesInPlace(std::string& text) {
    size_t writeIndex = 0;
    std::string prevWord;
    size_t readIndex = 0;

    while (readIndex < text.size()) {
        while (readIndex < text.size() && std::isspace(static_cast<unsigned char>(text[readIndex]))) {
            ++readIndex;
        }

        if (readIndex >= text.size()) break;
        
        size_t wordStart = readIndex;
        while (readIndex < text.size() && !std::isspace(static_cast<unsigned char>(text[readIndex]))) {
            ++readIndex;
        }
        size_t wordEnd = readIndex;

        std::string currentWord = text.substr(wordStart, wordEnd - wordStart);

        if (currentWord != prevWord) {
            if (writeIndex != 0) {
                text[writeIndex++] = ' ';
            }

            for (size_t i = 0; i < currentWord.size(); ++i) {
                text[writeIndex++] = currentWord[i];
            }

            prevWord = currentWord;
        }
    }

    text.resize(writeIndex);
}

std::string normalizeText(std::string input) {
    cleanCharsInPlace(input);
    removeWSInPlace(input);
    removeDuplicatesInPlace(input);
    return input;
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

    std::string result;

    dtime = dclock();
    for (unsigned long long i = 0; i < repeats; ++i) {
        result = normalizeText(inputText);
    }
    dtime = dclock() - dtime;

    std::cout << "Time: " << dtime << "\n";

    std::ofstream outFile("output.txt");
    outFile << result;
    outFile.close();

    fflush( stdout );

    return 0;
}