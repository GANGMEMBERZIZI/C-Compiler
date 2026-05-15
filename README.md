# ACWJ — A Compiler Writing Journey

A educational compiler that translates a C-like language into x86-64 and ARM assembly. Written in C, built from scratch with hand-written lexer, parser, AST, and code generator.

## Language Features

| Category | Features |
|----------|----------|
| Types | `char`, `int`, `long`, pointers (`char *`, `int *`, `long *`) |
| Variables | `int x;` / `char *p;` |
| Arrays | `int arr[10];` |
| Functions | `int foo(int x) { return x; }` / `void f() { }` |
| Arithmetic | `+`, `-`, `*`, `/`, negation (`-x`) |
| Bitwise | `&` (and), `\|` (or), `^` (xor), `~` (invert) |
| Shifts | `<<` (left), `>>` (right) |
| Logical | `!` (not), `&&` (and), `\|\|` (or) |
| Comparison | `==`, `!=`, `<`, `>`, `<=`, `>=` |
| If/Else | `if (x > 0) { ... } else { ... }` |
| While | `while (x < 10) { ... }` |
| For | `for (i=0; i<10; i=i+1) { ... }` |
| Pointers | `*p = 5;` / `x = *p;` / `&x` |
| Strings | `char *s = "hello";` |
| Inc/Dec | `++x`, `--x`, `x++`, `x--` |

Built-in runtime: `printint(n)`.

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
│  expr.c  │  Expression parser (Pratt parsing)
│  stmt.c  │  Statement parser
│  decl.c  │  Declaration parser
└──────────┘
    │
    ▼
┌──────────┐
│  tree.c  │  AST node construction
│  types.c │  Type system (widening, scaling, dereference)
│  sym.c   │  Symbol table management
└──────────┘
    │
    ▼
┌──────────┐
│  gen.c   │  Architecture-neutral code generator
│  cg.c    │  x86-64 backend (registers: r8-r11, AT&T syntax)
│ cg_arm.c │  ARM32 backend (registers: r4-r7)
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
| `gen.c` | Architecture-neutral code generator (walks AST) |
| `cg.c` | x86-64 code generation backend |
| `cg_arm.c` | ARM 32-bit code generation backend |
| `sym.c` | Symbol table management |
| `misc.c` | Utility functions (fatal errors, token matching) |
| `defs.h` | Token/AST/type enums and shared structs |
| `data.h` | Global variables |
| `decl.h` | Function declarations |
| `lib/printint.c` | Runtime library (printint) |

## Example

Input (`input.txt`):
```c
int a;
int b;
int c;
int main() {
  a= 42; b= 19;
  printint(a & b);
  printint(a | b);
  printint(a ^ b);
  printint(1 << 3);
  printint(63 >> 3);
  return(0);
}
```

Output:
```
$ make armtest
2
59
57
8
7
```
