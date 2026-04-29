#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <immintrin.h>
#include <iostream>
#include <pthread.h>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <thread>
#include <vector>

namespace {

constexpr std::size_t DEFAULT_SAMPLES = 1'000'000;
constexpr std::size_t RING_CAPACITY = 1024;
constexpr uint64_t DEFAULT_PACING_CYCLES = 3400; // ~1us on the Ryzen 5 2600 invariant TSC.

struct Options {
    int producer_cpu = 6;
    int consumer_cpu = 8;
    std::size_t samples = DEFAULT_SAMPLES;
    uint64_t pacing_cycles = DEFAULT_PACING_CYCLES;
};

struct alignas(64) PaddedIndex {
    std::atomic<std::size_t> value { 0 };
};

struct alignas(64) Event {
    uint64_t timestamp = 0;
    uint64_t sequence = 0;
    char payload[48] {};
};

struct Ring {
    static_assert((RING_CAPACITY & (RING_CAPACITY - 1)) == 0, "ring capacity must be a power of two");

    Event events[RING_CAPACITY];
    PaddedIndex head;
    PaddedIndex tail;
};

static uint64_t read_lfence_rdtsc() {
    _mm_lfence();
    return __rdtsc();
}

static void pin_current_thread(int cpu) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);

    const int rc = pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
    if (rc != 0) {
        throw std::runtime_error("pthread_setaffinity_np(cpu " + std::to_string(cpu)
            + ") failed: " + std::strerror(rc));
    }
}

static uint64_t parse_u64(const char* value, const char* name) {
    char* end = nullptr;
    errno = 0;
    const unsigned long long parsed = std::strtoull(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0') {
        throw std::runtime_error(std::string("invalid ") + name + ": " + value);
    }
    return static_cast<uint64_t>(parsed);
}

static Options parse_options(int argc, char** argv) {
    Options options;
    if (argc > 1) {
        options.producer_cpu = static_cast<int>(parse_u64(argv[1], "producer_cpu"));
    }
    if (argc > 2) {
        options.consumer_cpu = static_cast<int>(parse_u64(argv[2], "consumer_cpu"));
    }
    if (argc > 3) {
        options.samples = static_cast<std::size_t>(parse_u64(argv[3], "samples"));
    }
    if (argc > 4) {
        options.pacing_cycles = parse_u64(argv[4], "pacing_cycles");
    }
    if (argc > 5) {
        throw std::runtime_error(
            "usage: spsc_baseline [producer_cpu] [consumer_cpu] [samples] [pacing_cycles]");
    }
    return options;
}

static void print_percentiles(std::vector<uint64_t>& samples) {
    std::sort(samples.begin(), samples.end());

    const std::size_t count = samples.size();
    std::cout << "samples: " << count << '\n';
    std::cout << "p50: " << samples[count / 2] << '\n';
    std::cout << "p90: " << samples[count * 9 / 10] << '\n';
    std::cout << "p99: " << samples[count * 99 / 100] << '\n';
    std::cout << "p99.9: " << samples[count * 999 / 1000] << '\n';
    std::cout << "max: " << samples.back() << '\n';
    std::cout << "min: " << samples.front() << '\n';
}

static void producer(
    Ring& ring, std::atomic<bool>& start, const Options& options) {
    pin_current_thread(options.producer_cpu);

    while (!start.load(std::memory_order_acquire)) {
    }

    std::size_t cached_tail = ring.tail.value.load(std::memory_order_relaxed);
    uint64_t next_send = read_lfence_rdtsc() + options.pacing_cycles;
    for (std::size_t sequence = 0; sequence < options.samples; ++sequence) {
        while (read_lfence_rdtsc() < next_send) {
        }
        next_send += options.pacing_cycles;

        const std::size_t head = ring.head.value.load(std::memory_order_relaxed);
        const std::size_t next_head = (head + 1) & (RING_CAPACITY - 1);

        if (next_head == cached_tail) {
            do {
                cached_tail = ring.tail.value.load(std::memory_order_acquire);
            } while (next_head == cached_tail);
        }

        Event& event = ring.events[head];
        event.timestamp = read_lfence_rdtsc();
        event.sequence = sequence;
        event.payload[0] = static_cast<char>(sequence);

        ring.head.value.store(next_head, std::memory_order_release);
    }
}

static void consumer(
    Ring& ring, std::atomic<bool>& start, const Options& options, std::vector<uint64_t>& latencies) {
    pin_current_thread(options.consumer_cpu);

    start.store(true, std::memory_order_release);

    for (std::size_t received = 0; received < options.samples; ++received) {
        const std::size_t tail = ring.tail.value.load(std::memory_order_relaxed);

        while (ring.head.value.load(std::memory_order_acquire) == tail) {
        }

        const Event& event = ring.events[tail];
        const uint64_t sent = event.timestamp;
        const uint64_t now = read_lfence_rdtsc();
        latencies[received] = now - sent;

        const std::size_t next_tail = (tail + 1) & (RING_CAPACITY - 1);
        ring.tail.value.store(next_tail, std::memory_order_release);
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        const Options options = parse_options(argc, argv);
        Ring ring;
        std::vector<uint64_t> latencies(options.samples);
        std::atomic<bool> start { false };

        if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
            throw std::runtime_error(std::string("mlockall failed: ") + std::strerror(errno));
        }

        std::fill(latencies.begin(), latencies.end(), 0);
        for (Event& event : ring.events) {
            event.timestamp = 0;
        }

        std::thread consumer_thread(consumer, std::ref(ring), std::ref(start), std::cref(options),
            std::ref(latencies));
        std::thread producer_thread(producer, std::ref(ring), std::ref(start), std::cref(options));

        producer_thread.join();
        consumer_thread.join();

        std::cout << "spsc_baseline\n";
        std::cout << "producer_cpu: " << options.producer_cpu << '\n';
        std::cout << "consumer_cpu: " << options.consumer_cpu << '\n';
        std::cout << "pacing_cycles: " << options.pacing_cycles << '\n';
        print_percentiles(latencies);

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
