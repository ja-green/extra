#include "err.h"

#define PREFIX_FATAL "fatal: "

static void vreportf(const char *prefix, const char *str, va_list args) {
    fputs(prefix, stderr);
    vfprintf(stderr, str, args);
    fputs("\n", stderr);
}

void die(const char *s, ...) {
    va_list args;

    va_start(args, s);
    vreportf(PREFIX_FATAL, s, args);
    va_end(args);

    exit(1);
}
