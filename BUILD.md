# Tested/Confirmed Build Processes

## Prerequisites
To build `nxes`, some tooling is required, such as `make`, a C compiler toolchain,
a YACC-compatible parser generator, and `readline`, while optional, is most likely
desired for interactive use. `editline` should also work, but I have not tested this to
be certain.

My preferred C toolchain at this time is clang/LLVM, but any C toolchain with support
for at least ISO C99 should work just as well.

### Void (musl libc)

```sh
$ CC=clang LD=ld.lld ./configure --with-readline
$ bmake YACC=byacc ADDCFLAGS="-D_DEFAULT_SOURCE -D_POSIX_SOURCE=200809L -std=c99"
$ sudo make install
$ exec es
; echo <={$&version}
nxes version 0.0-alpha0 20200804
```

### DragonFly BSD

```sh
$ CC=clang LD=lld LDFLAGS="-L/usr/lib -L/usr/local/lib" es_cv_abused_getenv=no ./configure --with-readline
$ make
$ sudo make install
```
