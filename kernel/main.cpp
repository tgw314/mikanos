/**
 * @file main.cpp
 *
 * カーネル本体のプログラムを書いたファイル
 */

#include <cstdint>

#include "frame_buffer_config.hpp"

struct PixelColor {
    uint8_t r, g, b;
};

/** WritePixel は1つの点を描画する
 * @retval     0 成功
 * @retval not 0 失敗
 */

int WritePixel(const FrameBufferConfig &config, int x, int y,
               const PixelColor &c) {
    const int pixel_position = x + config.pixels_per_scan_line * y;
    uint8_t *p = &config.frame_buffer[4 * pixel_position];

    switch (config.pixel_format) {
        case kPixelRGBResv8BitPerColor:
            p[0] = c.r;
            p[1] = c.g;
            p[2] = c.b;
            break;
        case kPixelBGRResv8BitPerColor:
            p[0] = c.b;
            p[1] = c.g;
            p[2] = c.r;
            break;
        default:
            return -1;
    }
    return 0;
}

extern "C" void KernelMain(const FrameBufferConfig &frame_buffer_config) {
    for (int x = 0; x < frame_buffer_config.horizontal_resolution; x++) {
        for (int y = 0; y < frame_buffer_config.vertical_resolution; y++) {
            WritePixel(frame_buffer_config, x, y, {255, 255, 255});
        }
    }
    for (int x = 0; x < 200; x++) {
        for (int y = 0; y < 100; y++) {
            WritePixel(frame_buffer_config, 100 + x, 100 + y, {0, 255, 0});
        }
    }
    for (;;) __asm__("hlt");
}
