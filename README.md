# nxes(1) - Exile Heavy Industries Extensible Shell
This is a fork of the public domain `es(1)` shell, the original, from 1997 can be fetched from [here](ftp://ftp.sys.toronto.edu/pub/es)
as [es-0.9-beta1.tar.gz](ftp://ftp.sys.toronto.edu/pub/es/es-0.9-beta1.tar.gz).

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
***

Consult the manual page (`roff` formatted) "es.1" for complete details.

Please read through README.orig to see the original text, especially as this is under construction.
