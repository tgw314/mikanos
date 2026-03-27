#include <cstdlib>
#include <cstring>

#include "../../kernel/graphics.hpp"

auto &printk =
    *reinterpret_cast<int (*)(const char *, ...)>(0x000000000010cc90);
auto &fill_rect =
    *reinterpret_cast<decltype(FillRectangle) *>(0x000000000010dd00);
auto &scrn_writer =
    *reinterpret_cast<decltype(screen_writer) *>(0x000000000024d068);

int stack_ptr;
long stack[100];

long Pop() { return stack[stack_ptr--]; }

void Push(long value) { stack[++stack_ptr] = value; }

extern "C" int main(int argc, char **argv) {
    stack_ptr = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "+") == 0) {
            long b = Pop();
            long a = Pop();
            Push(a + b);
            printk("[%d] <- %ld\n", stack_ptr, a + b);
            continue;
        }
        if (strcmp(argv[i], "-") == 0) {
            long b = Pop();
            long a = Pop();
            Push(a - b);
            printk("[%d] <- %ld\n", stack_ptr, a - b);
            continue;
        }

        long a = atol(argv[i]);
        Push(a);
        printk("[%d] <- %ld\n", stack_ptr, a);
    }

    fill_rect(*scrn_writer, Vector2D<int>{100, 10}, Vector2D<int>{200, 200},
              ToColor(0x00ff00));

    if (stack_ptr < 0) {
        return 0;
    }
    return static_cast<int>(Pop());
}
