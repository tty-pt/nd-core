/* nd-core.h — nd-core's public interface, for modules that decorate its icons.
 *
 * Caller-facing header. Implementers include <nd/core-types.h>, not this header.
 */

#ifndef ND_CORE_H
#define ND_CORE_H

#include <ttypt/xy.h>
#include <nd/core-types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Register `fn` to decorate icons. Returns 0 on success, non-zero if the table
 * is full or `fn` is NULL. Each successful registration logs "core_icon_decorate
 * #N" through WARN, which is what the engine's test suite greps to confirm a
 * decorator module took effect. */
XY_DECL(int, core_icon_decorate, core_icon_fn, fn);

typedef struct bio (*core_noise_fn)(struct bio bio, uint32_t he, uint32_t w,
	uint32_t tm, uint32_t cl);
typedef sic_str_t (*core_empty_tile_fn)(view_tile_t t, unsigned side, sic_str_t ss);

/* OR'd into `side` by the ASCII map path (view.c) only. Decorators strip it
 * with `side & CORE_ET_SIDE_MASK` and may draw less (or differently) there;
 * the structured map_data path never sets it. */
#ifndef CORE_ET_ASCII
#define CORE_ET_ASCII 0x80u
#define CORE_ET_SIDE_MASK 0x03u
#endif

XY_DECL(int, core_noise_decorate, core_noise_fn, fn);
XY_DECL(int, core_empty_tile_decorate, core_empty_tile_fn, fn);

#ifdef __cplusplus
}
#endif

#endif /* ND_CORE_H */
