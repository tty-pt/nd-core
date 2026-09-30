# ~/axil-nd-core — the <name> game module, ported to libxylem.
#
# The whole build contract lives in the engine's nd-mod.mk (MODS.md §5.0.4);
# this file only says which engine to build against. nd-mod.mk locates its own
# headers, so the only thing to decide here is where nd-mod.mk itself lives.
#
# PREFIX selects an installed engine; if there isn't one, fall back to an
# axil-nd checkout beside this repo (MODS.md §4's one-repo-per-module layout),
# which is what a dev tree looks like. An earlier revision hardcoded one
# developer's absolute checkout path here, which built fine for them and
# nowhere else.
PREFIX ?= /usr
ND_MOD_MK := $(firstword $(wildcard $(PREFIX)/share/axil-nd/nd-mod.mk ../axil-nd/nd-mod.mk))
ifeq ($(ND_MOD_MK),)
$(error nd-mod.mk not found: looked in $(PREFIX)/share/axil-nd/ and ../axil-nd/ \
	-- install the engine, or check out axil-nd beside this repo)
endif
include $(ND_MOD_MK)

# MOD is the module stem: the engine names this module by that same stem in
# axil-nd's mods.load, and xy_load() appends .so itself. Set explicitly rather
# than relying on nd-mod.mk's axil-nd-% strip, so a renamed checkout still
# builds the right artifact.
MOD := core
