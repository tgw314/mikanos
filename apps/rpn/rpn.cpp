#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "../../kernel/logger.hpp"

int stack_ptr;
long stack[100];

long Pop() { return stack[stack_ptr--]; }

void Push(long value) { stack[++stack_ptr] = value; }

extern "C" int64_t SyscallLogString(LogLevel, const char *);

extern "C" int main(int argc, char **argv) {
    stack_ptr = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "+") == 0) {
            long b = Pop();
            long a = Pop();
            Push(a + b);
            SyscallLogString(kWarn, "+");
            continue;
        }
        if (strcmp(argv[i], "-") == 0) {
            long b = Pop();
            long a = Pop();
            Push(a - b);
            SyscallLogString(kWarn, "-");
            continue;
        }

        long a = atol(argv[i]);
        Push(a);
        SyscallLogString(kWarn, "#");
    }

    if (stack_ptr < 0) {
        return 0;
    }
    SyscallLogString(kWarn, "\nhello, this is rpn\n");
    for (;;);
}
