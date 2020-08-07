#ifndef AX_ERR_H
#define AX_ERR_H

#pragma once

#include "types.h"

extern int ax_errno;

#ifdef __cplusplus
extern "C" {
#endif

void die(const char *, ...);

#ifdef __cplusplus
}
#endif

#endif /* AX_ERR_H */
