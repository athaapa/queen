#include "framebuffer.hpp"
#include "limine.h"
#include "serial.hpp"
#include <stdint.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

static uint64_t fb_addr;
static uint64_t fb_height;
static uint64_t fb_width;
static uint64_t fb_pitch;
static uint64_t fb_bpp;
static bool fb_available;

static uint64_t cursor_y;
static uint64_t cursor_x;

constexpr uint8_t FONT_SCALE = 2;
constexpr uint64_t CHAR_W = 8 * FONT_SCALE;
constexpr uint64_t CHAR_H = 8 * FONT_SCALE;
constexpr uint32_t FG = 0x00ffffff;
constexpr uint32_t BG = 0x00000000;

static const uint8_t* glyph_for(char c);
static void draw_glyph(uint64_t x, uint64_t y, const uint8_t glyph[8]);
static void draw_char(uint64_t x, uint64_t y, char c);
static void put_pixel(uint64_t x, uint64_t y, uint32_t color);

static constexpr uint8_t GLYPH_UNKNOWN[8] = {
    0b01111110,
    0b01000010,
    0b01011010,
    0b01011010,
    0b01000010,
    0b01011010,
    0b01111110,
    0b00000000,
};

static constexpr uint8_t GLYPH_SPACE[8] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
};

static constexpr uint8_t GLYPH_COLON[8] = {
    0b00000000,
    0b00011000,
    0b00011000,
    0b00000000,
    0b00000000,
    0b00011000,
    0b00011000,
    0b00000000,
};

static constexpr uint8_t GLYPH_UNDERSCORE[8] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b01111110,
    0b00000000,
};

static constexpr uint8_t GLYPH_DIGITS[10][8] = {
    { 0b00111100, 0b01100110, 0b01101110, 0b01110110, 0b01100110, 0b01100110, 0b00111100,
        0b00000000 },
    { 0b00011000, 0b00111000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b01111110,
        0b00000000 },
    { 0b00111100, 0b01100110, 0b00000110, 0b00011100, 0b00110000, 0b01100000, 0b01111110,
        0b00000000 },
    { 0b00111100, 0b01100110, 0b00000110, 0b00011100, 0b00000110, 0b01100110, 0b00111100,
        0b00000000 },
    { 0b00001100, 0b00011100, 0b00101100, 0b01001100, 0b01111110, 0b00001100, 0b00001100,
        0b00000000 },
    { 0b01111110, 0b01100000, 0b01111100, 0b00000110, 0b00000110, 0b01100110, 0b00111100,
        0b00000000 },
    { 0b00111100, 0b01100110, 0b01100000, 0b01111100, 0b01100110, 0b01100110, 0b00111100,
        0b00000000 },
    { 0b01111110, 0b00000110, 0b00001100, 0b00011000, 0b00110000, 0b00110000, 0b00110000,
        0b00000000 },
    { 0b00111100, 0b01100110, 0b01100110, 0b00111100, 0b01100110, 0b01100110, 0b00111100,
        0b00000000 },
    { 0b00111100, 0b01100110, 0b01100110, 0b00111110, 0b00000110, 0b01100110, 0b00111100,
        0b00000000 },
};

static constexpr uint8_t GLYPH_LETTERS[26][8] = {
    { 0b00000000, 0b00111100, 0b00000110, 0b00111110, 0b01100110, 0b01100110, 0b00111110,
        0b00000000 },
    { 0b01100000, 0b01100000, 0b01111100, 0b01100110, 0b01100110, 0b01100110, 0b01111100,
        0b00000000 },
    { 0b00000000, 0b00111100, 0b01100110, 0b01100000, 0b01100000, 0b01100110, 0b00111100,
        0b00000000 },
    { 0b00000110, 0b00000110, 0b00111110, 0b01100110, 0b01100110, 0b01100110, 0b00111110,
        0b00000000 },
    { 0b00000000, 0b00111100, 0b01100110, 0b01111110, 0b01100000, 0b01100110, 0b00111100,
        0b00000000 },
    { 0b00011100, 0b00110110, 0b00110000, 0b01111100, 0b00110000, 0b00110000, 0b00110000,
        0b00000000 },
    { 0b00000000, 0b00111110, 0b01100110, 0b01100110, 0b00111110, 0b00000110, 0b00111100,
        0b00000000 },
    { 0b01100000, 0b01100000, 0b01111100, 0b01100110, 0b01100110, 0b01100110, 0b01100110,
        0b00000000 },
    { 0b00011000, 0b00000000, 0b00111000, 0b00011000, 0b00011000, 0b00011000, 0b00111100,
        0b00000000 },
    { 0b00001100, 0b00000000, 0b00001100, 0b00001100, 0b00001100, 0b01101100, 0b00111000,
        0b00000000 },
    { 0b01100000, 0b01100000, 0b01100110, 0b01101100, 0b01111000, 0b01101100, 0b01100110,
        0b00000000 },
    { 0b00111000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00111100,
        0b00000000 },
    { 0b00000000, 0b01101100, 0b01111110, 0b01111110, 0b01101010, 0b01100010, 0b01100010,
        0b00000000 },
    { 0b00000000, 0b01111100, 0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b01100110,
        0b00000000 },
    { 0b00000000, 0b00111100, 0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b00111100,
        0b00000000 },
    { 0b00000000, 0b01111100, 0b01100110, 0b01100110, 0b01111100, 0b01100000, 0b01100000,
        0b00000000 },
    { 0b00000000, 0b00111110, 0b01100110, 0b01100110, 0b00111110, 0b00000110, 0b00000110,
        0b00000000 },
    { 0b00000000, 0b01111100, 0b01100110, 0b01100000, 0b01100000, 0b01100000, 0b01100000,
        0b00000000 },
    { 0b00000000, 0b00111110, 0b01100000, 0b00111100, 0b00000110, 0b01100110, 0b00111100,
        0b00000000 },
    { 0b00110000, 0b00110000, 0b01111100, 0b00110000, 0b00110000, 0b00110110, 0b00011100,
        0b00000000 },
    { 0b00000000, 0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b00111110,
        0b00000000 },
    { 0b00000000, 0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b00111100, 0b00011000,
        0b00000000 },
    { 0b00000000, 0b01100010, 0b01100010, 0b01101010, 0b01111110, 0b01111110, 0b01101100,
        0b00000000 },
    { 0b00000000, 0b01100110, 0b00111100, 0b00011000, 0b00111100, 0b01100110, 0b01100110,
        0b00000000 },
    { 0b00000000, 0b01100110, 0b01100110, 0b01100110, 0b00111110, 0b00000110, 0b00111100,
        0b00000000 },
    { 0b00000000, 0b01111110, 0b00001100, 0b00011000, 0b00110000, 0b01100000, 0b01111110,
        0b00000000 },
};

void queen::framebuffer::init() {
    if (framebuffer_request.response == nullptr
        || framebuffer_request.response->framebuffer_count == 0) {
        queen::serial::write_line("no framebuffer");
        return;
    }

    limine_framebuffer* fb = framebuffer_request.response->framebuffers[0];
    fb_addr = reinterpret_cast<uint64_t>(fb->address);
    fb_width = fb->width;
    fb_height = fb->height;
    fb_pitch = fb->pitch;
    fb_bpp = fb->bpp;
    fb_available = true;

    queen::serial::write("fb address: ");
    queen::serial::write_hex(reinterpret_cast<uint64_t>(fb->address));
    queen::serial::write("\n");

    queen::serial::write("fb width: ");
    queen::serial::write_decimal(fb->width);
    queen::serial::write("\n");

    queen::serial::write("fb height: ");
    queen::serial::write_decimal(fb->height);
    queen::serial::write("\n");

    queen::serial::write("fb pitch: ");
    queen::serial::write_decimal(fb->pitch);
    queen::serial::write("\n");

    queen::serial::write("fb bpp: ");
    queen::serial::write_decimal(fb->bpp);
    queen::serial::write("\n");
}

uint64_t queen::framebuffer::virtual_base() { return fb_addr; }
uint64_t queen::framebuffer::size_bytes() { return fb_pitch * fb_height; }
bool queen::framebuffer::available() { return fb_available; }
void queen::framebuffer::fill(uint32_t color) {
    uint8_t* base = reinterpret_cast<uint8_t*>(fb_addr);
    for (uint64_t y = 0; y < fb_height; y++) {
        uint32_t* row = reinterpret_cast<uint32_t*>(base + y * fb_pitch);

        for (uint64_t x = 0; x < fb_width; x++) {
            row[x] = color;
        }
    }
}

void queen::framebuffer::write(const char* s) {
    while (*s != '\0') {
        put_char(*s);
        s++;
    }
}

void queen::framebuffer::write_hex(uint64_t value) {
    write("0x");
    int i = 60;
    while (i >= 0) {
        uint8_t byte = (value >> i) & 0xF;
        const char c = (byte < 10) ? (byte + '0') : ((byte - 10) + 'a');
        put_char(c);
        i -= 4;
    }
}

void queen::framebuffer::write_decimal(uint64_t value) {
    if (value == 0) {
        put_char('0');
        return;
    }

    uint64_t temp = value;
    uint64_t divisor = 1;

    while (temp >= 10) {
        temp /= 10;
        divisor *= 10;
    }

    while (divisor > 0) {
        uint64_t digit = value / divisor;
        put_char(static_cast<char>(digit + '0'));

        value %= divisor;
        divisor /= 10;
    }
}

void queen::framebuffer::put_char(char c) {
    if (!fb_available || fb_bpp != 32) {
        return;
    }

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        return;
    }
    draw_char(cursor_x * CHAR_W, cursor_y * CHAR_H, c);
    cursor_x++;

    if ((cursor_x + 1) * CHAR_W >= fb_width) {
        cursor_x = 0;
        cursor_y++;
    }
}

static void draw_char(uint64_t x, uint64_t y, char c) { draw_glyph(x, y, glyph_for(c)); }

static void draw_glyph(uint64_t x, uint64_t y, const uint8_t glyph[8]) {
    for (uint64_t row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];

        for (uint64_t col = 0; col < 8; col++) {
            bool on = bits & (1 << (7 - col));
            for (uint64_t dy = 0; dy < FONT_SCALE; dy++) {
                for (uint64_t dx = 0; dx < FONT_SCALE; dx++) {
                    put_pixel(x + col * FONT_SCALE + dx, y + row * FONT_SCALE + dy, on ? FG : BG);
                }
            }
        }
    }
}

static const uint8_t* glyph_for(char c) {
    if (c >= 'A' && c <= 'Z') {
        c = static_cast<char>(c - 'A' + 'a');
    }

    if (c >= '0' && c <= '9') {
        return GLYPH_DIGITS[c - '0'];
    }

    if (c >= 'a' && c <= 'z') {
        return GLYPH_LETTERS[c - 'a'];
    }

    switch (c) {
    case ' ':
        return GLYPH_SPACE;
    case ':':
        return GLYPH_COLON;
    case '_':
        return GLYPH_UNDERSCORE;
    default:
        return GLYPH_UNKNOWN;
    }
}

static void put_pixel(uint64_t x, uint64_t y, uint32_t color) {
    uint8_t* base = reinterpret_cast<uint8_t*>(fb_addr);
    uint32_t* pixel = reinterpret_cast<uint32_t*>(base + y * fb_pitch + x * 4);
    *pixel = color;
}
