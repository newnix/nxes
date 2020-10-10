# nxes(1) - Exile Heavy Industries Extensible Shell
This is a fork of the public domain `es(1)` shell, the original, from 1997 can be fetched from one of the following locations:
	* ftp://ftp.sys.toronto.edu/pub/es/es-0.9-beta1.tar.gz
	* ftp://ftp.sys.utoronto.ca/pub/es/es-0.9-beta1.tar.gz

Can be pronounced as an initialism same as shell like `csh`, `ksh`, `zsh`, etc. ("en ecks ee ess") or 
as "nexus".

### License
See the LICENSE file for full details, but this code is made available under the 0BSD license.
For all intents and purposes, this code remains in the public domain, in the same spirit as
the initial (known) authors: Byron Rakitzis, Paul Haahr, and Soren Dayton.

## Enhancements
So far, this version of `es(1)` has been extended with the following primitive operations:
1. Integer addition/subtraction (`sum`)
2. Integer multiplication (`mul`)
3. Integer division (`div`)
4. Modulus operation (`mod`)
5. Bitwise AND (`band`)
6. Bitwise NOT (`bnot`) (NOTE: Unary operation)
7. Bitwise XOR (`bxor`)
8. Bitwise OR  (`bor`)

### Examples
The additional primitives do not print the results to `stdout`, instead they're returned much like
in a programming language. Their use is therefore quite different from using POSIX `sh` or GNU `bash`,
and shares at least superficial similarity with the LISP family of languages.

Note: Semicolon ('`;`') is the default prompt.

```sh
# Addition
; echo <={sum 1 2 3}
6

# Subtraction
; echo <={sum -1 -2 -3}
-6

# Multiplication
; echo <={mul 2 4 6}
48

# Division
; echo <={div 20 2 2}
5

# Modulus
; echo <={mod 5 3}
2

# Bitwise AND
; echo <={band 1 3}
1

# Bitwise OR
; echo <={band 1 4}
5

# Bitwise XOR
; echo <={bxor 7 3}
4

# Bitwise NOT
; echo <={bnot 0}
-1

```

All of these operations work on signed 64-bit integers, there's not currently any bounds or
sanity checking on the inputs, so use with care if you need such large values in your interactive
shell scripts/sessions.

## Installation
The installation process should be relatively straightforward, though I've had some issues with undefined types
show up when trying to compile on Ubuntu 20.04 LTS. On Void Linux with musl libc, clang 10, bmake, and byacc however,
no such issue has appeared. Any C compiler should work, but I have not had time to validate this yet. 

My process was as follows:

```sh
$ CC=clang LD=ld.lld ./configure
$ make YACC=byacc ADDCFLAGS="-D_DEFAULT_SOURCE -D_POSIX_SOURCE=200809L -std=c99"
$ ./es
; echo <={$&version}
nxes version 0.0-alpha0 20200804
```
---

Consult the manual page (`roff` formatted) "es.1" for complete details.

Please read through README.orig to see the original text, especially as this is under construction.

---

### Goals
Some goals for the future of this fork (in no particular order):
1. Modernize codebase (Drop obsolete hacks, attempt to make future maintenance easier)
2. Add primitives to facilitate use as a glue language or midway point between POSIX `sh` and "real" languages like Racket/C/Python, etc.
3. Work on making `nxes` tail recursive, this is mentioned in the original TODO (#2)
4. Add some basic history manipulation commands such as `!!` and `!-2$` in GNU `bash` to aid in productivity
5. Allow subshells to inherit closures (#1 in original TODO)
6. Potentially expose `%parse` in such a way as to create a simple macro system
7. Simplify and streamline build process
8. Look at moving away from Yacc/Bison for writing the lexer/parser
9. Allow for nested lists, in turn allowing for trees and other derived data structures
10. Look into creating a `printf(1)` builtin for more complex printing needs than just `$&echo`
11. Consider changing variable/function binding syntax using more LISP-like forms, this would free up `=` from being a special character, which should make flags requiring '=' (e.g. `clang -std=c99`) more ergonomic than with the current parsing grammar. (having to do `'='` or similar)

Some non-goals:
1. Become a strict, functional programming language
2. Become a lazy, functional programming language
3. Rewrite in Rust or other language (maybe some day, but this is not a goal)
4. Become feature compatible with GNU `bash` (or really any other shell, some features are nice, but let's keep things small)
