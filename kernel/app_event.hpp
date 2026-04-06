#pragma once

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

struct AppEvent {
    enum Type {
        kQuit,
    } type;
};

#ifdef __cplusplus
}
#endif  // __cplusplus
