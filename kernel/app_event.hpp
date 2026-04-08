#pragma once

#ifdef __cplusplus
#include <cstdint>

extern "C" {
#else

#include <stdint.h>
#endif  // __cplusplus

struct AppEvent {
    enum Type {
        kQuit,
        kMouseMove,
        kMouseButton,
    } type;

    union {
        struct {
            int x, y;
            int dx, dy;
            uint8_t buttons;
        } mouse_move;

        struct {
            int x, y;
            int press;  // 1: press, 0: release
            int button;
        } mouse_button;
    } arg;
};

#ifdef __cplusplus
}
#endif  // __cplusplus
