# nd-core

The icon builder for [axil-nd](../axil-nd), ported from SIC to libxylem.

`on_icon` returns the `struct icon` the client uses to draw an object: its glyph,
its colour, and the bitmask of actions the object supports. It is the reason the
slice has `nd-core` in it at all — it is the only module here that returns a
**struct by value across the bus**, so it is the proof that the XY event contract
survives a non-scalar return.

nd-core is also the **sole owner** of that icon. Modules that want to amend an
icon register a decorator instead of co-implementing `on_icon`, because XY
cannot observe a co-implemented chain. See `include/ttypt/nd-core.h`.

## Install

```sh
make install
```

Installs one file:

```
lib/libnd-core.so
include/ttypt/nd-core.h
```

There is deliberately no `lib/nd-core.so` symlink. `xy_load()` appends `.so`
itself and does not retry with a `lib` prefix, so the engine's `mods.load` names
this module `libnd-core` and `dlopen`s `libnd-core.so` — the same name the
engine already resolves for `libaxil-nd` and `libxylem`. A soname symlink would
also have been silently dropped from the OpenBSD package: `tty-pt/ci` builds the
packing list from `find usr -type f`, which never lists a symlink, so the package
would have shipped the library under one name and asked the loader for another.

Also packaged for deb, apk, rpm, brew and openbsd from a `v*` tag.

## Build from source

```sh
make
```

Needs [libxylem](https://github.com/tty-pt/libxylem) (headers already in
`$(PREFIX)/include`) and the engine's game API, `<nd/xy.h>`, from either an
`axil-nd` checkout beside this repo or an installed `axil-nd`:

```sh
git clone https://github.com/tty-pt/axil-nd-core && cd axil-nd-core
git clone https://github.com/tty-pt/axil-nd ../axil-nd
make
```

`<nd/xy.h>` installs to `$(PREFIX)/include/nd/`, the same directory that already
carries `<ttypt/xy.h>`, so an **installed** engine needs no `-I` of its own here.
Against a checkout beside this repo it is `-I../axil-nd/include`; both paths are
on `CFLAGS` at once and a missing `-I` is ignored, so the same command works
either way.

## Using it

A decorator is a function that receives the running icon and returns the
amended one — it must not re-derive the icon it is decorating, or it silently
drops whatever the owner set.

```c
#include <ttypt/xy-mod.h>

#include <ttypt/nd-core.h>

static struct icon
my_decorator(struct icon i, unsigned ref __attribute__((unused)), unsigned type,
	unsigned player_ref __attribute__((unused)))
{
	/* `ref` and `player_ref` are unused in the simplest possible decorator,
	 * which is why they are annotated rather than dropped: the house builds
	 * modules with -Wall -Wextra -Wpedantic, so an unused parameter is a
	 * warning, not a style choice. Both have to be marked — the attribute
	 * binds to the parameter it follows, so trailing it after the last one
	 * only silences that one. */
	if (type == TYPE_ENTITY)
		i.ch = '?';
	return i;
}

XY_MODULE_API void
xy_install(void)
{
	/* Refcounted: a no-op when the engine already loaded us from
	 * mods.load, and required when it did not. */
	if (xy_load("libnd-core") != XY_OK)
		return;
	if (core_icon_decorate(my_decorator) != 0)
		WARN("my-module: decorator table full\n");
}
```

Registration order *is* chain order, so a module earlier in `mods.load`
decorates earlier. Each successful registration logs `core_icon_decorate #N`,
which is what the engine's suite greps to confirm a decorator took effect.

One TU must not both `XY_IMPL` and `XY_DECL` the same name — that is the XY
equivalent of the old `SIC_DEF`/`SIC_DECL` collision, a build error at best and a
silently wrong dispatch at worst. Hence the `CORE_IMPL` guard in the header.

## Testing

There is no `test.sh` here. Behaviour is asserted by the engine's own suite,
which builds every module in its `mods.load`, boots, and greps stderr:

```sh
# stage nd-core, then run the engine's suite against that prefix
make PREFIX=/tmp/stage install
cd ../axil-nd
PREFIX=/tmp/stage LD_LIBRARY_PATH=/tmp/stage/lib ./test.sh
```

Both variables are needed because the two halves of the check reach nd-core
differently. `PREFIX` is a **build** variable: `axil-nd`'s module rule passes
`-I$(XY_INC)`, so `shop` needs `<ttypt/nd-core.h>` from the install, and without
it the suite dies compiling before it ever boots. `LD_LIBRARY_PATH` is a
**runtime** variable: `mods.load` says `libnd-core`, and `xy_load("libnd-core")`
becomes a `dlopen("libnd-core.so")` resolved through the normal loader search
path, so the `.so` has to be findable by the linker.

After `make install` into a real prefix both are unnecessary and the suite is
just `make && ./test.sh`.

It checks `nd-core: on_icon TYPE_ROOM -> ch='-'` (the struct return crossed the
bus, read *after* the decorator chain) and `nd-core: core_icon_decorate #1` (a
decorator in another module reached this build). A shared runner that a module
repo can use on its own is wanted; see `MODS.md` in the engine.

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
* The suite marker is printed *after* the decorator chain runs. Printed from
  inside `on_icon` it was a canary that could not see its own chain: a decorator
  that replaced every room's glyph still produced the line, and the suite stayed
  green.

## License

BSD 2-Clause, carried over from `tty-pt/nd-core`. See `LICENSE`.
