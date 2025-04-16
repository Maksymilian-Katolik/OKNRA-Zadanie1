#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <vector>
#include <thread>
#include <mutex>
#include <sys/time.h>

static double gtod_ref_time_sec = 0.0;

double dclock()
{
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
constexpr int num_threads{8};

bool isPunctuation(char c) {
    return std::ispunct(static_cast<unsigned char>(c)) && c != ',';
}

std::string cleanChars(const std::string &input) {
    std::string cleaned;
    cleaned.reserve(input.size());
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

std::string removeWS(const std::string &input) {
    std::string spaced;
    spaced.reserve(input.size());
    bool inWhitespace = false;
    for (char c : input) {
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

void processChunk(const std::string& input, std::string& output) {
    std::string cleaned = cleanChars(input);
    output = removeWS(cleaned);
}

std::string normalizeTextParallel(const std::string& input) {
    size_t total_len = input.size();
    size_t chunk_size = total_len / num_threads;

    std::vector<std::string> partials(num_threads);
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == num_threads - 1) ? total_len : (i + 1) * chunk_size;

        // nie przerwanie słowa na końcu fragmentu
        if (i != num_threads - 1) {
            while (end < total_len && !std::isspace(static_cast<unsigned char>(input[end]))) {
                ++end;
            }
        }

        threads.emplace_back([&, start, end, i]() {
            std::string part = input.substr(start, end - start);
            processChunk(part, partials[i]);
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    std::ostringstream combined;
    for (const auto& part : partials) {
        if (!part.empty()) {
            if (!combined.str().empty() && combined.str().back() != ' ') {
                combined << ' ';
            }
            combined << part;
        }
    }

    return removeDuplicates(combined.str());
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
        result = normalizeTextParallel(inputText);
    }
    
    dtime = dclock() - dtime;

    std::cout << "Time: " << dtime << "\n";

    std::ofstream outFile("output.txt");
    outFile << result;
    outFile.close();

    fflush( stdout );

    return 0;
}




