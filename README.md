# nxes(1) - Exile Heavy Industries Extensible Shell
This is a fork of the public domain `es(1)` shell, the original, from 1997 can be fetched from [here](ftp://ftp.sys.toronto.edu/pub/es)
as [es-0.9-beta1.tar.gz](ftp://ftp.sys.toronto.edu/pub/es/es-0.9-beta1.tar.gz).

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

Note: Semicolon (';') is the default prompt.

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

***

Consult the manual page (`roff` formatted) "es.1" for complete details.
Please read through README.orig to see the original text, especially as this is under construction.
