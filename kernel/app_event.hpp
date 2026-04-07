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
    } type;

    union {
        struct {
            int x, y;
            int dx, dy;
            uint8_t buttons;
        } mouse_move;
    } arg;
};

#ifdef __cplusplus
}
#endif  // __cplusplus
