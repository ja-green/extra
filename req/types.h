#ifndef AX_TYPES_H
#define AX_TYPES_H

#pragma once

#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// #include "platform.h"

#ifdef PFM_WIN
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <process.h>
#include <windows.h>
#endif

#ifndef AX_BINNAME
#define AX_BINNAME "passgen"
#endif

#ifndef AX_VERSION
#define AX_VERSION "undefined"
#endif

#define BUFLEN 1024

typedef enum {
    FALSE = 0,
    TRUE  = !FALSE,
} bool_t;

#define likely(x)   __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)

#define PFMT64x "llx"
#define PFMT64d "lld"
#define PFMT64u "llu"
#define PFMT64o "llo"
#define PFMTSZx "zx"
#define PFMTSZd "zd"
#define PFMTSZu "zu"
#define PFMTSZo "zo"
#define LDBLFMT "Lf"
#define HHXFMT  "hhx"
#define PFMT32x "x"
#define PFMT32d "d"
#define PFMT32u "u"
#define PFMT32f "f"
#define PFMT32o "o"
#define PFMT8s  "s"
#define PFMT8c  "c"
#define PFMTXp  "p"

#ifdef __GNUC__
#define _attr_inline     __attribute__((always_inline)) inline
#define _attr_malloc     __attribute__((malloc))
#define _attr_allocsz(x) __attribute__((alloc_size(x)))
#define _attr_pure       __attribute__((pure))
#define _attr_const      __attribute__((const))
#define _attr_used       __attribute__((used))
#define _attr_unused     __attribute__((unused))
#define _attr_runused    __attribute__((warn_unused_result))
#define _attr_packed     __attribute__((packed))
#define _attr_cold       __attribute__((cold))
#define _attr_hot        __attribute__((hot))
#define _attr_deprecated __attribute__((deprecated))
#define _attr_fmt(x,y,z) __attribute__((format(x,y,z)))
#define _attr_fmtarg(x)  __attribute__((format_arg(x)))
#define _attr_noreturn   __attribute__((noreturn))
#else
#define _attr_inline     inline
#define _attr_malloc
#define _attr_allocsz(x)
#define _attr_pure
#define _attr_const
#define _attr_used
#define _attr_unused
#define _attr_runused
#define _attr_packed
#define _attr_cold
#define _attr_hot
#define _attr_deprecated
#define _attr_fmt(x,y,z)
#define _attr_fmtarg(x)
#define _attr_noreturn
#endif

#ifdef AX_API
#undef AX_API
#endif
#ifdef AX_INLINE
#define AX_API inline
#else
#if defined(__GNUC__) && __GNUC__ >= 4
#define AX_API __attribute__((visibility("default")))
#elif defined(_MSC_VER)
#define AX_API __declspec(dllexport)
#else
#define AX_API
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* AX_TYPES_H */
