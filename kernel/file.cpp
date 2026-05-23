#include "file.hpp"

#include <cstdarg>
#include <cstddef>
#include <cstdio>

size_t PrintToFD(FileDescriptor &fd, const char *format, ...) {
    va_list ap;
    int result;
    char s[128];

    va_start(ap, format);
    result = vsprintf(s, format, ap);
    va_end(ap);

    fd.Write(s, result);
    return result;
}

size_t ReadDelim(FileDescriptor &fd, char delim, char *buf, size_t len) {
    for (size_t i = 0; i < len - 1; i++) {
        if (fd.Read(&buf[i], 1) == 0) {
            buf[i] = '\0';
            return i;
        }
        if (buf[i] == delim) {
            buf[++i] = '\0';
            return i;
        }
    }
    buf[len - 1] = '\0';
    return len - 1;
}
