#include "mouse.hpp"

#include "graphics.hpp"

namespace {
const char mouse_cursor_shape[kMouseCursorHeight][kMouseCursorWidth + 1] = {
    // clang-format off
    "@              ",
    "@@             ",
    "@.@            ",
    "@..@           ",
    "@...@          ",
    "@....@         ",
    "@.....@        ",
    "@......@       ",
    "@.......@      ",
    "@........@     ",
    "@.........@    ",
    "@..........@   ",
    "@...........@  ",
    "@............@ ",
    "@......@@@@@@@@",
    "@......@       ",
    "@....@@.@      ",
    "@...@ @.@      ",
    "@..@   @.@     ",
    "@.@    @.@     ",
    "@@      @.@    ",
    "@       @.@    ",
    "         @.@   ",
    "         @@@   ",
    // clang-format on
};
}

void DrawMouseCursor(PixelWriter *pixel_writer, Vector2D<int> position) {
    for (int dy = 0; dy < kMouseCursorHeight; dy++) {
        for (int dx = 0; dx < kMouseCursorWidth; dx++) {
            switch (mouse_cursor_shape[dy][dx]) {
                case '@':
                    pixel_writer->Write(position + Vector2D<int>{dx, dy},
                                        {0, 0, 0});
                    break;
                case '.':
                    pixel_writer->Write(position + Vector2D<int>{dx, dy},
                                        {255, 255, 255});
                    break;
                default:
                    pixel_writer->Write(position + Vector2D<int>{dx, dy},
                                        kMouseTransparentColor);
            }
        }
    }
}
