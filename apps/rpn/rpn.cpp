#include <cstdlib>
#include <cstring>
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
            continue;
        }
        if (strcmp(argv[i], "-") == 0) {
            long b = Pop();
            long a = Pop();
            Push(a - b);
            continue;
        }

        long a = atol(argv[i]);
        Push(a);
    }
    if (stack_ptr < 0) {
        return 0;
    }
    return static_cast<int>(Pop());
}
