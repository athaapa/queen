#include <algorithm>
#include <cstdint>
#include <immintrin.h>
#include <iostream>
#include <vector>

constexpr int SAMPLES = 1'000'000;

static uint64_t read_rdtsc() { return __rdtsc(); }

static uint64_t read_lfence_rdtsc() {
    _mm_lfence();
    return __rdtsc();
}

static uint64_t read_rdtscp() {
    unsigned aux = 0;
    return __rdtscp(&aux);
}

template <typename ReadFn>
static void run_benchmark(const char* label, ReadFn read_fn) {
    std::vector<uint64_t> samples(static_cast<std::size_t>(SAMPLES));
    for (int i = 0; i < SAMPLES; ++i) {
        const uint64_t start = read_fn();
        const uint64_t end = read_fn();
        samples[static_cast<std::size_t>(i)] = end - start;
    }

    std::sort(samples.begin(), samples.end());

    std::cout << label << '\n';
    std::cout << "samples: " << SAMPLES << '\n';
    std::cout << "p50: " << samples[SAMPLES / 2] << '\n';
    std::cout << "p90: " << samples[SAMPLES * 9 / 10] << '\n';
    std::cout << "p99: " << samples[SAMPLES * 99 / 100] << '\n';
    std::cout << "p99.9: " << samples[SAMPLES * 999 / 1000] << '\n';
    std::cout << "max: " << samples.back() << '\n';
    std::cout << "min: " << samples.front() << '\n';
    std::cout << '\n';
}

int main() {
    run_benchmark("rdtsc", read_rdtsc);
    run_benchmark("lfence; rdtsc", read_lfence_rdtsc);
    run_benchmark("rdtscp", read_rdtscp);
    return 0;
}
