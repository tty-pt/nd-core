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

#ifdef __cplusplus
}
#endif

#endif /* ND_CORE_H */
