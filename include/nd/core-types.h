#ifndef ND_CORE_TYPES_H
#define ND_CORE_TYPES_H

/*
 * nd/core-types.h — Shared types and decorator callback definitions for nd-core.
 * Contains zero XY_DECLs.
 */

#include <ttypt/xy.h>
#include <nd/xy-types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct icon (*core_icon_fn)(struct icon i, unsigned ref, unsigned type,
	unsigned player_ref);

#ifdef __cplusplus
}
#endif

#endif /* ND_CORE_TYPES_H */
