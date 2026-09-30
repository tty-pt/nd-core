/* core.h — nd-core's public interface, for modules that decorate its icons.
 *
 * Follows the ~/site/mods dependency style rather than a hook everybody shares.
 * Measured there: 163 hooks, each with exactly ONE implementor, zero
 * co-implemented hooks. A consumer names the owner and calls it:
 *
 *	#include "../axil-nd-core/core.h"
 *	...
 *	xy_load("../axil-nd-core/core");   // ensure the callee is resident
 *	core_icon_decorate(my_decorator);
 *
 * so a consumer never co-implements `on_icon`. That matters because XY cannot
 * express the old `sic_call` chain: every co-implementor runs, but only the LAST
 * return survives and `xy.last()` cannot read the previous one mid-dispatch
 * (libxylem-dispatch.c:14-22, `xy_last_ran` is 0 until the dispatch ends).
 * Measured, not inferred -- see MODS.md §7.
 *
 * So nd-core is the sole owner of `on_icon` and holds the ordered decorator
 * table. It builds the base icon, then threads it through each registered
 * decorator in registration order, and returns the result. That is the old
 * `sic_call` semantics (nd/src/interface.c:378-404) restored inside the model,
 * with the order made explicit instead of incidental to load order.
 *
 * Order is registration order, i.e. the order `xy_install()` of each decorator
 * runs. Put a module earlier in mods.load to decorate earlier.
 *
 * Keep this header includable from a TU that XY_IMPLs `core_icon_decorate`:
 * the CORE_IMPL guard is the XY equivalent of the old SIC_DEF/SIC_DECL
 * collision, and the same rule MODS.md §5.0.1 records for papi/nd-hooks.h.
 */
#ifndef CORE_H
#define CORE_H

#include <ttypt/xy.h>

#include "papi/nd-xy-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A decorator amends an icon the owner already built. It RECEIVES the running
 * icon as its first argument and returns the amended one -- the same shape as
 * the old `struct icon on_icon(struct icon i, unsigned ref, unsigned type)`,
 * which is the whole point: a decorator must not have to re-derive the icon it
 * is decorating, or it silently drops whatever the owner set. `ref` is the
 * object, `type` its type and `player_ref` the viewer. */
typedef struct icon (*core_icon_fn)(struct icon i, unsigned ref, unsigned type,
	unsigned player_ref);

#ifndef CORE_IMPL
/* Register `fn` to decorate icons. Returns 0 on success, non-zero if the table
 * is full or `fn` is NULL. Each successful registration logs "core_icon_decorate
 * #N" through WARN, which is what the engine's test suite greps to confirm a
 * decorator module took effect. */
XY_DECL(int, core_icon_decorate, core_icon_fn, fn);
#endif /* !CORE_IMPL */

#ifdef __cplusplus
}
#endif

#endif /* CORE_H */
