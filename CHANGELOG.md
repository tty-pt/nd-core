## 1.0.0

- **nd-core is now an installable library rather than a build artifact of the
  engine.** It builds and installs one file, `lib/libnd-core.so`, plus the public
  header as `include/ttypt/nd-core.h`, following the same layout as `axil-tty`
  and `axil-auth`. Previously `make` produced a `core.so` named by the engine's
  `mods.load` and installed nothing. There is no `lib/nd-core.so` symlink: the
  engine's `mods.load` names this module `libnd-core`, and a symlink would in any
  case have been dropped from the OpenBSD package, whose packing list is built
  from `find usr -type f`.

- **The link line is libxylem alone.** `LDLIBS := -lxylem`; the engine is not
  linked. nd-core only *provides* `on_icon`, and libxylem keys a module by
  (resolved path, region id), so an `xy_load("libaxil-nd")` from inside a module
  is a different key and would re-run the engine's `xy_install` in a child
  region. `NEEDED` is `libxylem.so` and `libc.so.6`, matching `axil-tty`.

- **`core.h` moved to `include/ttypt/nd-core.h`** and is now included as
  `<ttypt/nd-core.h>`. `mk/portable.mk` puts `$(pwd)/include` ahead of
  `$(PREFIX)/include`, so one spelling resolves to this checkout in a dev build
  and to the installed header otherwise.

- **The game's service API is included as `<nd/xy.h>`**, from the engine's
  `$(PREFIX)/include/nd/` — the same include root as `<ttypt/xy.h>`, so this
  library needs no private `-I` for the game headers at all, against either an
  installed engine or a checkout beside it.

- **Dropped the `nd-mod.mk` dependency.** nd-core resolves the game's headers
  itself, the way every other house library does. `nd-mod.mk` is the SIC-era
  engine module build contract and goes away with SIC; it is still installed by
  the engine for the remaining module repos and is deleted separately.

- **`mods.load` names the stem, not a path.** The line is now `nd-core` instead
  of `../axil-nd-core/core`. A bare name with no in-tree module resolves to an
  installed library by soname, which is what makes "installable" true. As before,
  the stem carries no `.so` — `xy_load()` appends the suffix itself.
