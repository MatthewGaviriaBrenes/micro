# Micro Compiler

Compiler for the **Micro** language, developed in C, that generates
x86-64 assembly for Linux. Based on Chapter 2 of *Crafting a Compiler with C*
(Fischer & LeBlanc), with two additional extensions: **constant folding**
(Exercise 8) and **conditional expressions** (Exercise 10).

## Requirements

- Linux (tested on x86-64)
- `gcc` (used both to compile the project and to assemble/link the generated
  Micro programs)
- `make`

## Building

```bash
make
```

This generates the `micro` executable in the project root.

To remove binaries and generated files:

```bash
make clean
```

To run the included test suite:

```bash
make test
```

## Usage

```bash
./micro program.micro
```

The compiler:

1. Performs lexical and syntactic analysis on `program.micro`.
2. If there are errors, it reports them and exits without generating any
   additional files.
3. If the program is valid, it generates `program.s` (x86-64 assembly) **in the
   same directory** as the source file.
4. Assembles and links `program.s` with `gcc` (via `system()`), producing an
   executable with the same base name (`program`).
5. Runs the resulting program.

Example:

```bash
./micro examples/basic.micro
```

This generates `examples/basic.s` and `examples/basic`, then runs the latter.

Only **one command-line argument** is accepted: the path to the `.micro` file.
There is no other way to interact with the compiler.

## The Micro Language

- A single data type: `integer`.
- Implicit identifiers (no prior declaration required), with a maximum length
  of 32 characters.
- Integer literals: sequences of digits.
- Comments: begin with `--` and continue to the end of the line.
- Reserved words: `begin`, `end`, `read`, `write`.
- Every statement ends with `;`.
- The program is delimited by `begin` ... `end`.
- Assignment operator: `:=`.
- Arithmetic operators: `+`, `-`.

### Basic Example

```micro
begin
    read(A, B);
    C := A + B;
    write(C);
end
```

### Constant Folding

If both operands of an expression are literals, the value is computed at
compile time, and no code is generated for the operation:

```micro
C := 10 + 20 - 5;   -- translated directly to: movl $25, C(%rip)
```

### Conditional Expressions

Syntax: `( E1 | E2 | E3 )`. If `E1` is nonzero, the value of the expression is
`E2`; if `E1` is zero, the value is `E3`.

```micro
begin
    A := 5;
    B := (A | 100 | 200);   -- B = 100, because A != 0
    write(B);
end
```

They can be nested:

```micro
D := ( (A | 1 | 0) | 100 | 200 );
```

The book assumes a fictitious `Skip A` instruction to implement this feature.
Since we generate real x86-64 code, we translated it into the standard pattern
of comparison and conditional branching, equivalent to a low-level `if/else`.

## Architecture

```text
program.micro
     |
     v
  scanner   (lexical analysis)
     |
     v
   parser   (syntactic analysis, recursive descent)
     |
     v
 semantics  (constant folding, identifier checking)
     |
     v
symbol table
     |
     v
  codegen   (x86-64 code generation)
     |
     v
program.s
     |
     v
   gcc      (assembly + linking, via system())
     |
     v
 executable
     |
     v
 execution
```

### Modules

| Module | Files | Responsibility |
|---|---|---|
| Scanner | `scanner.c/.h` | Tokenizes the source code and detects lexical errors |
| Parser | `parser.c/.h` | Analyzes the syntactic structure and coordinates calls to the semantics and code generation modules |
| Symbol Table | `symtab.c/.h` | Registers implicit identifiers |
| Semantics | `semantics.c/.h` | Performs constant folding on `+`/`-` operations |
| Codegen | `codegen.c/.h` | Generates x86-64 instructions |
| Main | `main.c` | Orchestrates the complete workflow: parsing, generation, assembly, linking, and execution |

## Error Handling

- **Lexical errors**: invalid characters and identifiers that are too long
  (>32 characters).
- **Syntax errors**: unexpected tokens, with recovery through synchronization
  up to `;`, `end`, or the end of the file. This allows multiple errors to be
  reported in a single pass instead of stopping at the first one.
- In both cases, the compiler exits with a nonzero status code and does not
  generate or leave behind partial files.

## Testing

The `tests/` directory contains `.micro` programs covering:

- Valid cases (assignments, expressions, `read`/`write`, and comments).
- Invalid cases (missing `;`, `begin`, `end`, parentheses, or commas).
- Constant folding.
- Simple and nested conditional expressions.
- Identifiers at the length limit (32 characters) and above the limit.

## What Comes from the Book vs. What We Developed

**From the book (Fischer & LeBlanc, Chapter 2):**

- The compiler’s general structure (scanner → parser → semantics → codegen).
- The base Micro grammar and the recursive descent parsing approach.
- The concepts of `current_token`, `match()`, and semantic action symbols
  integrated into the parser routines.
- The statement and conceptual idea of constant folding (Exercise 8) and
  conditional expressions (Exercise 10), including the exact syntax
  `( E1 | E2 | E3 )`.

**Developed/adapted by us:**

- Generation of real x86-64 assembly (the book does not specify this
  architecture in detail).
- Translation of the fictitious `Skip A` instruction from Exercise 10 into
  real x86 instructions.
- Complete integration through `main.c`: argument handling, path derivation,
  assembly/linking/execution via `system()`.
- Syntax error recovery through statement-level synchronization.