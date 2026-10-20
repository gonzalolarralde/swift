#include <stdlib.h>
#include <stdio.h>

#if defined(__PICOLIBC__)
// Picolibc's compact stdio archive can define putchar in an object needed by
// other stdio functions. Disable buffering without interposing that symbol.
__attribute__((constructor))
static void disable_stdio_buffering(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
}
#else
int (putchar)(int c) {
    putchar_unlocked(c);
    fflush(stdout);
    return 0;
}
#endif
