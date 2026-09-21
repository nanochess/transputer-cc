# Transputer C compiler

## by Oscar Toledo G. [https://nanochess.org](https://nanochess.org)

This is an improved version of my transputer C compiler (the same embedded inside my full operating system) but with the following enhancements:

Already available in the cc0 branch:

* ANSI escape sequences for color in the command line.
* Input and output with UNIX conventions.
* All internal uses of char where converted to unsigned char to make it more portable.

Already available in the cc1 branch:

* The expression tree is created dynamically.

Exclusive of this branch cc2:

* All the non-portable pointer management (pointer to int conversions) was rewritten using structs, and some things now use malloc/free.
* It is compilable even on 64-bit machines (tested with macOS) and generates the same assembler output as the transputer emulator.
* All the interface and error messages have been translated to English.

# How to use it

For compiling in a normal command-line environment:

    clang -std=c89 -Wno-error=implicit cc.c -o cc

You can compile the assembler output using tasm (my transputer assembler) and my stdio.len library (see the os_final/tree/lib directory):

    tasm cc2.a cc2.e stdio.len
        
You can put it into the transputer emulator (only for my full operating system), but you need to recompile the emulator for 256 KB. of memory as the malloc/free usage requires extra memory.

I've provide an already compiled cc2.p for immediate usage (it is the cc2.e passed through C:/Ejecutable.p with a 8192 bytes stack). There will be some trash on the screen as my OS doesn't allow for ANSI escape sequences.
