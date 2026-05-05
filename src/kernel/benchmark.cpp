#include "benchmark.hpp"
#include "mathutil.hpp"
#include "serial.hpp"
#include "time.hpp"
#include <stdint.h>

constexpr int N = 4096;

static uint64_t latencies[N];

struct Event {
    uint64_t id;
    uint64_t value;
};

static uint64_t handle_event(const Event& event);
static void print_latency_report(const char* name, uint64_t checksum);
static void swap(uint64_t& a, uint64_t& b);
static void sort_latencies();

void queen::benchmark::run_tsc_overhead() {
    for (int i = 0; i < N; i++) {
        uint64_t start = queen::read_tsc_ordered();
        uint64_t end = queen::read_tsc_ordered();
        latencies[i] = end - start;
    }

    print_latency_report("tsc_overhead", 0);
}

void queen::benchmark::run_event_loop() {
    static Event events[N];
    for (int i = 0; i < N; i++) {
        events[i].id = i % 31;
        events[i].value = i % 67;
    }

    uint64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        uint64_t start = queen::read_tsc_ordered();
        uint64_t x = handle_event(events[i]);
        checksum += x;
        uint64_t end = queen::read_tsc_ordered();

        latencies[i] = (end - start);
    }

    print_latency_report("event_loop", checksum);
}

static uint64_t handle_event(const Event& event) { return event.id + 3 * event.value; }

static void print_latency_report(const char* name, uint64_t checksum) {
    uint64_t mx = 0;
    uint64_t mn = UINT64_MAX;
    for (int i = 0; i < N; i++) {
        mx = queen::mathutil::max(mx, latencies[i]);
        mn = queen::mathutil::min(mn, latencies[i]);
    }

    sort_latencies();

    queen::serial::write(name);
    queen::serial::write("\n");
    queen::serial::write("p50: ");
    queen::serial::write_decimal(latencies[N / 2]);
    queen::serial::write("\n");

    queen::serial::write("p99: ");
    queen::serial::write_decimal(latencies[N * 99 / 100]);
    queen::serial::write("\n");

    queen::serial::write("p999: ");
    queen::serial::write_decimal(latencies[N * 999 / 1000]);
    queen::serial::write("\n");

    queen::serial::write("max: ");
    queen::serial::write_decimal(mx);
    queen::serial::write("\n");

    queen::serial::write("min: ");
    queen::serial::write_decimal(mn);
    queen::serial::write("\n");

    queen::serial::write("checksum: ");
    queen::serial::write_decimal(checksum);
    queen::serial::write("\n");
}

static void swap(uint64_t& a, uint64_t& b) {
    uint64_t temp = a;
    a = b;
    b = temp;
}

static void sort_latencies() {
    for (int i = 1; i < N; i++) {
        int j = i;
        while (j > 0 && latencies[j] < latencies[j - 1]) {
            swap(latencies[j], latencies[j - 1]);
            j--;
        }
    }
}
