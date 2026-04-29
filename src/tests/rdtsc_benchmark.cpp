#include <algorithm>
#include <cstdint>
#include <immintrin.h>
#include <iostream>
#include <vector>

constexpr int SAMPLES = 1'000'000;

int main() {
    std::vector<uint64_t> samples(SAMPLES);

    for (int i = 0; i < SAMPLES; i++) {
        uint64_t start = __rdtsc();
        uint64_t end = __rdtsc();
        samples[i] = end - start;
    }

    std::sort(samples.begin(), samples.end());
    std::cout << "samples: " << samples.size() << '\n';
    std::cout << "p50: " << samples[SAMPLES / 2] << '\n';
    std::cout << "p90: " << samples[SAMPLES * 9 / 10] << '\n';
    std::cout << "p99: " << samples[SAMPLES * 99 / 100] << '\n';
    std::cout << "p99.9: " << samples[SAMPLES * 999 / 1000] << '\n';

    std::cout << "max: " << samples.back() << '\n';
    std::cout << "min: " << samples.front() << '\n';

    return 0;
}