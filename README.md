# axil-nd-core

`nd-core` for [axil-nd](../axil-nd), ported from SIC to libxylem.

Builds the `struct icon` the client uses to draw an entity: the glyph, its
colour, and the bitmask of actions the entity supports. It is the reason the
slice has `nd-core` in it at all — it is the only module here that returns a
**struct by value across the bus**, so it is the proof that the XY event
contract survives a non-scalar return.

## Build

```sh
make
```

Produces `core.so`. `nd-mod.mk` (from the engine, installed to
`/usr/share/axil-nd/nd-mod.mk`) holds the entire build contract; the local
`Makefile` only names the engine to build against. `PREFIX` selects the
installed headers — including the file from an `axil-nd` checkout instead makes
it self-locate, so a dev tree needs no configuration.

## Install

The engine loads modules named in **its** `mods.load`, by stem, not by path
into this repo:

```
../axil-nd-core/core
```

`xy_load()` appends `.so` itself, so the line must be `core`, never `core.so` —
`core.so` makes the engine look for `core.so.so`. From axil-nd, `make mods`
builds every module in that list, and `./test.sh` asserts this one loaded and
fired.

## What it does

`on_icon(ref, type, player_ref)` returns a `struct icon` for `ref`. Rooms get a
fixed glyph; the module reads the entity's skeleton and type name to pick the
rest. It fires on the MCP item path (`_fbcp_item` → `eng_object_icon`), i.e.
when the client asks for an entity's icon — not at connect.

## Notes from the port

* **A real bug was fixed rather than transcribed.** The original called
  `nd_get(HD_OBJ, &what, &what.location)` — passing `what.location` as the
  *key* before `what` was ever populated, i.e. reading uninitialised stack.
  That is why its `ref` parameter was marked `__attribute__((unused))` while
  `player_ref`, also so marked, was actually used. Ported to
  `nd_get(HD_OBJ, &what, &ref)`, the evident intent, which also retires both
  stale attributes.
* It gained an `xy_install`. The original had neither `mod_install` nor
  `mod_open` and relied on SIC's `.sic_auto_init` section; with XY the install
  is explicit. It only logs.
* This TU `XY_IMPL`s `on_icon`, so it must **not** `#include` a header that
  `XY_DECL`s `on_icon` (`papi/nd-hooks.h`). An `XY_IMPL` and an `XY_DECL` of the
  same name in one TU is the XY equivalent of the old `SIC_DEF`/`SIC_DECL`
  collision, and it is a build error at best and a silently wrong dispatch at
  worst.

## License

BSD 2-Clause, carried over from `tty-pt/nd-core`. See `LICENSE`.
