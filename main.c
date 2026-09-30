/* main.c — nd-core, ported to libxylem.
 *
 * on_icon builds the short glyph+action set the client shows for an object,
 * and is the only reason this module exists. It is the slice's proof that a
 * STRUCT RETURN crosses the XY bus: the engine XY_DEFs `on_icon` in
 * src/nd_events.c and memcpy's the result into the BCP frame (src/mcp.c:116),
 * so a wrong ABI here corrupts every visible icon in the game rather than
 * failing loudly.
 *
 * Original: tty-pt/nd-core @ 877 B main.c, standalone (not in the nd-basics
 * superproject). Ported unchanged in behaviour; see README.md for the two
 * corrections.
 *
 * This TU XY_IMPLs on_icon and so must NOT include papi/nd-hooks.h -- an
 * XY_IMPL and an XY_DECL of the same name in one TU is the XY equivalent of
 * the old `SIC_DEF` + `SIC_DECL` collision. The canonical signature lives in
 * that header for reference and for anyone who wants to CALL on_icon.
 *
 * nd-core is also the single owner of the icon CHAIN, via core.h: modules that
 * want to amend an icon (shop, drink, plant, fight) register a decorator
 * instead of co-implementing on_icon, because XY cannot observe a
 * co-implemented chain. See core.h and MODS.md §7.
 */

#include <ttypt/xy-mod.h>

#include "papi/nd-xy.h"

#define CORE_IMPL
#include "core.h"

/* --- the decorator table ---------------------------------------------------
 *
 * Fixed size, indexed, insertion-ordered -- the same shape ~/site/mods uses
 * for its registered modules (index/index.c:28-37). A small fixed table is the
 * point: the chain's order is then a property of this code, not an accident of
 * which .so happened to be dlopen'd first.
 */
#define CORE_ICON_DECORATORS_MAX 16

static core_icon_fn core_icon_decorators[CORE_ICON_DECORATORS_MAX];
static unsigned core_icon_n_decorators;

/* Set when the object being iconned is a TYPE_ROOM, so the once-only suite
 * marker can be printed after the decorator chain has run. */
static int core_icon_mark_room;

XY_IMPL(int, core_icon_decorate, core_icon_fn, fn)
{
	if (!fn)
		return 1;
	if (core_icon_n_decorators == CORE_ICON_DECORATORS_MAX) {
		WARN("nd-core: core_icon_decorate: table full (%u)\n",
			CORE_ICON_DECORATORS_MAX);
		return 1;
	}

	core_icon_decorators[core_icon_n_decorators++] = fn;
	WARN("nd-core: core_icon_decorate #%u\n", core_icon_n_decorators);
	return 0;
}

XY_MODULE_API void
xy_install(void)
{
	/* The original had no mod_install/mod_open at all: SIC's .sic_auto_init
	 * section registered on_icon without the module ever running code. XY
	 * does not need an install hook either -- hooks register themselves --
	 * but the engine's test suite greps axil's stderr for a once-only marker
	 * per module, so this is where it goes. */
	WARN("nd-core: xy_install, on_icon registered\n");
}

/* `ref` is the object being iconned. The original declared both `ref` and
 * `player_ref` __attribute__((unused)) while using player_ref three lines
 * later, and never actually used `ref` -- both stale. */
XY_IMPL(struct icon, on_icon, unsigned, ref, unsigned, type,
	unsigned, player_ref)
{
	struct icon i = {
		.actions = ACT_LOOK,
		.ch = '?',
		.pi = { .fg = WHITE, .flags = BOLD, },
	};

	switch (type) {
	case TYPE_ROOM:
		i.ch = '-';
		i.pi.fg = YELLOW;
		/* Once-only marker for the engine's test suite: on_icon is fired
		 * for every object the client renders, so "this ran" alone proves
		 * nothing. TYPE_ROOM is the one type whose value is fixed by
		 * the engine rather than allocated from HD_TYPE, so a glyph here
		 * proves the whole engine->bus->module->bus->client round trip
		 * including the struct return.
		 *
		 * Printed AFTER the decorator loop, not here. It used to be
		 * printed here, which made it a canary that could not see its own
		 * chain: a decorator that replaced every room's glyph still
		 * produced this exact line, and the suite stayed green. See the
		 * `marked` static below. */
		core_icon_mark_room = 1;
		break;
	case TYPE_ENTITY:
		i.pi.flags = BOLD;
		i.ch = '!';
		i.pi.fg = YELLOW;
		break;
	default: {
		OBJ what, loc;

		/* CORRECTED. The original read
		 *     nd_get(HD_OBJ, &what, &what.location);
		 * i.e. it passed `what.location` as the KEY before `what` had
		 * been populated at all -- an uninitialised stack read, and the
		 * reason `ref` went unused. `ref` is the object the engine is
		 * asking about, so it is the key. Behaviour on any build with
		 * warnings enabled was undefined; this is the evident intent.
		 */
		nd_get(HD_OBJ, &what, &ref);

		if (what.location == player_ref) {
			i.actions |= ACT_DROP;
			break;
		}

		nd_get(HD_OBJ, &loc, &what.location);
		if (loc.type != TYPE_ENTITY)
			i.actions |= ACT_GET;

		break;
	}
	}

	/* Thread the running icon through the decorators, in registration
	 * order. This is the old sic_call() loop (nd/src/interface.c:378-404)
	 * with the order made explicit: each fn sees what the previous one
	 * returned, which is the control the co-implementor route cannot give
	 * (both handlers ran in the probe, but the second's return replaced the
	 * first's and xy.last() read NOTFOUND -- MODS.md §7). */
	for (unsigned n = 0; n < core_icon_n_decorators; n++)
		i = core_icon_decorators[n](i, ref, type, player_ref);

	/* The suite marker, now that the chain has had its say -- so this is
	 * the value that actually reached the client, not the one the owner
	 * built before anyone could amend it. */
	if (core_icon_mark_room) {
		static int marked;

		core_icon_mark_room = 0;
		if (!marked) {
			marked = 1;
			WARN("nd-core: on_icon TYPE_ROOM "
			     "-> ch='%c' actions=0x%x\n", i.ch,
			     (unsigned) i.actions);
		}
	}

	return i;
}
