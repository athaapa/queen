#include "benchmark.hpp"
#include "framebuffer.hpp"
#include "mathutil.hpp"
#include "program.hpp"
#include "serial.hpp"
#include "time.hpp"
#include <stdint.h>

constexpr int N = 4096;
constexpr int BATCHES[] = { 64 };

static uint64_t latencies[N];

struct Event {
    uint64_t id;
    uint64_t value;
};

extern "C" uint64_t queen_event_loop_entry(uint64_t id, uint64_t value);

static void run_event_loop_batch(
    const char* name, queen::program::Entry entry, const Event events[N], int batch);
static void print_latency_report(const char* name, int batch, uint64_t checksum);
static void write_report_label(const char* label);
static void write_report_value(uint64_t value);
static void write_report_newline();
static void swap(uint64_t& a, uint64_t& b);
static void sort_latencies();
static uint64_t kernel_embedded_entry(uint64_t id, uint64_t value);

void queen::benchmark::run_tsc_overhead() {
    for (int i = 0; i < N; i++) {
        uint64_t start = queen::read_tsc_ordered();
        uint64_t end = queen::read_tsc_ordered();
        latencies[i] = end - start;
    }

    print_latency_report("tsc_overhead", 0, 0);
}

void queen::benchmark::run_event_loop() {
    static Event events[N];
    for (int i = 0; i < N; i++) {
        events[i].id = i % 31;
        events[i].value = i % 67;
    }

    for (uint64_t i = 0; i < sizeof(BATCHES) / sizeof(BATCHES[0]); i++) {
        run_event_loop_batch("kernel_embedded_entry", kernel_embedded_entry, events, BATCHES[i]);
        run_event_loop_batch("linked_program", queen_event_loop_entry, events, BATCHES[i]);
    }
}

static void run_event_loop_batch(
    const char* name, queen::program::Entry entry, const Event events[N], int batch) {
    uint64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        uint64_t start = queen::read_tsc_ordered();
        for (int j = 0; j < batch; j++) {
            const Event& event = events[(i * batch + j) % N];
            uint64_t x = entry(event.id, event.value);
            checksum += x;
        }
        uint64_t end = queen::read_tsc_ordered();

        latencies[i] = (end - start) / batch;
    }

    print_latency_report(name, batch, checksum);
}

static void print_latency_report(const char* name, int batch, uint64_t checksum) {
    uint64_t mx = 0;
    uint64_t mn = UINT64_MAX;
    for (int i = 0; i < N; i++) {
        mx = queen::mathutil::max(mx, latencies[i]);
        mn = queen::mathutil::min(mn, latencies[i]);
    }

    sort_latencies();

    write_report_label(name);
    write_report_newline();

    if (batch != 0) {
        write_report_label("batch: ");
        write_report_value(batch);
        write_report_newline();
    }

    write_report_label("p50: ");
    write_report_value(latencies[N / 2]);
    write_report_newline();

    write_report_label("p99: ");
    write_report_value(latencies[N * 99 / 100]);
    write_report_newline();

    write_report_label("p999: ");
    write_report_value(latencies[N * 999 / 1000]);
    write_report_newline();

    write_report_label("max: ");
    write_report_value(mx);
    write_report_newline();

    write_report_label("min: ");
    write_report_value(mn);
    write_report_newline();

    write_report_label("checksum: ");
    write_report_value(checksum);
    write_report_newline();
}

static void write_report_label(const char* label) {
    queen::serial::write(label);
    queen::framebuffer::write(label);
}

static void write_report_value(uint64_t value) {
    queen::serial::write_decimal(value);
    queen::framebuffer::write_decimal(value);
}

static void write_report_newline() {
    queen::serial::write("\n");
    queen::framebuffer::write("\n");
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

static uint64_t kernel_embedded_entry(uint64_t id, uint64_t value) { return id + 3 * value; }
