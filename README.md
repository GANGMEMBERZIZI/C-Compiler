# ACWJ — A Compiler Writing Journey

A educational compiler that translates a C-like language into x86-64 and ARM assembly. Written in C, built from scratch with hand-written lexer, parser, AST, and code generator.

## Language Features

| Feature | Example |
|---------|---------|
| Types | `char`, `int`, `long`, pointers |
| Variables | `int x;` / `char *p;` |
| Arrays | `int arr[10];` |
| Functions | `int foo(int x) { return x; }` |
| If/Else | `if (x > 0) { ... } else { ... }` |
| While | `while (x < 10) { ... }` |
| For | `for (i=0; i<10; i=i+1) { ... }` |
| Pointers | `*p = 5;` / `x = *p;` / `&x` |
| Strings | `char *s = "hello";` |

Built-in runtime: `printint(n)` and `printchar(c)`.

## Build & Test

```bash
# x86-64 (native)
make test

# ARM (cross-compile + QEMU)
make armtest

# Build compilers only
make comp1      # x86-64 version
make comp1arm   # ARM version

# Clean
make clean
```

### Prerequisites
- **x86 test**: `gcc`
- **ARM test**: `arm-linux-gnueabi-gcc`, `qemu-arm`

## Architecture

```
Source (.txt)
    │
    ▼
┌──────────┐
│  scan.c  │  Lexer — tokenizes input
└──────────┘
    │
    ▼
┌──────────┐
│  expr.c  │  Parser (Pratt) — builds AST
│  stmt.c  │  Statement parser
│  decl.c  │  Declaration parser
└──────────┘
    │
    ▼
┌──────────┐
│  gen.c   │  Code generator — walks AST, emits assembly
│  cg.c    │  x86-64 backend
│ cg_arm.c │  ARM backend
└──────────┘
    │
    ▼
Assembly (.s) ──► gcc + lib/printint.c ──► executable
```

## Project Structure

| File | Role |
|------|------|
| `main.c` | Entry point, CLI, initialization |
| `scan.c` | Lexer / tokenizer |
| `expr.c` | Expression parser (Pratt parsing) |
| `stmt.c` | Statement parser (if, while, for, compound) |
| `decl.c` | Declaration parser (variables, functions, arrays) |
| `tree.c` | AST node construction helpers |
| `types.c` | Type system (widening, scaling, dereference) |
| `gen.c` | Architecture-neutral code generator |
| `cg.c` | x86-64 code generation backend |
| `cg_arm.c` | ARM (32-bit) code generation backend |
| `sym.c` | Symbol table management |
| `misc.c` | Utility functions (fatal errors, token matching) |
| `defs.h` | Token/AST/type enums and structs |
| `data.h` | Global variables |
| `decl.h` | Function declarations |
| `lib/printint.c` | Runtime library (printint, printchar) |

## Example

Input (`input.txt`):
```c
char  c;
char *str;

int main() {
  c = '\n'; printint(c);

  for (str = "Hello world\n"; *str != 0; str = str + 1) {
    printchar(*str);
  }
  return(0);
}
```

Output:
```
$ make armtest
10
Hello world
```
