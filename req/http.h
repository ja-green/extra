#ifndef AX_HTTP_H
#define AX_HTTP_H

#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

int do_request(int, const char *, const char *);

#ifdef __cplusplus
}
#endif

#endif /* AX_HTTP_H */
