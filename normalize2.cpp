#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <sys/time.h>

static double gtod_ref_time_sec = 0.0;

/* Adapted from the bl2_clock() routine in the BLIS library */
// wzięte z labu 6

double dclock()
{
    double the_time, norm_sec;
    struct timeval tv;
    gettimeofday( &tv, NULL );
    if ( gtod_ref_time_sec == 0.0 )
        gtod_ref_time_sec = ( double ) tv.tv_sec;
    norm_sec = ( double ) tv.tv_sec - gtod_ref_time_sec;
    the_time = norm_sec + tv.tv_usec * 1.0e-6;
    return the_time;
}


constexpr unsigned long long repeats{5000};


bool isPunctuation(char c) {
    return std::ispunct(static_cast<unsigned char>(c)) && c != ',';
}

std::string cleanChars(const std::string &input) {
    std::string cleaned;

    cleaned.reserve(input.size()); // Prealokacja — alokujemy tyle miejsca co w pliku wejściowym 
    // w pliku wyjściowym nigdy nie będzie więcej znaków niż w wejściowym (zawsze będzie albo tyle samo albo mniej)

    for (char c : input) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            cleaned += ' ';
        } else if (c >= 32 && c <= 126) {
            if (isPunctuation(c)) {
                cleaned += ',';
            } else {
                cleaned += std::tolower(static_cast<unsigned char>(c));
            }
        }
    }
    return cleaned;
}

std::string removeWS(const std::string &cleaned) {
    std::string spaced;
    spaced.reserve(cleaned.size()); // nie będzie dłuższy niż cleaned

    bool inWhitespace = false;
    for (char c : cleaned) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!inWhitespace) {
                spaced += ' ';
                inWhitespace = true;
            }
        } else {
            spaced += c;
            inWhitespace = false;
        }
    }
    return spaced;
}


std::string removeDuplicates(const std::string &spaced) {
    std::string result;
    result.reserve(spaced.size());

    std::string prev, word;
    size_t i = 0, len = spaced.size();

    while (i < len) {
        while (i < len && std::isspace(static_cast<unsigned char>(spaced[i]))) {
            ++i;
        }

        word.clear();
        while (i < len && !std::isspace(static_cast<unsigned char>(spaced[i]))) {
            word += spaced[i++];
        }

        if (!word.empty() && word != prev) {
            if (!result.empty()) result += ' ';
            result += word;
            prev = word;
        }
    }

    return result;
}


std::string normalizeText(const std::string& input) {
    std::string cleaned{cleanChars(input)};
    
    std::string spaced{removeWS(cleaned)};

    return removeDuplicates(spaced);
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
;
    for (unsigned long long i{}; i < repeats; i++) {
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




