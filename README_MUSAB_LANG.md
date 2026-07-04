# Musab Language Compiler

Customized chibicc for the Musab programming language.

## Language Features

### Basic Syntax

```musab
use stdlib;

const PI: float = 3.141592;
var age: int = 18;
var name: string = "Musab";
var nums: int[];

struct User {
    name: string;
    age: int;
}

enum Color {
    Red,
    Green,
    Blue
}

fn add(a: int, b: int): int {
    return a + b;
}

fn factorial(n: int): int {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

fn main() {
    io.out("Hello, World!");
}
```

## Compiler Architecture

1. **Tokenizer** (`musab_tokenize.c`)
   - Lexical analysis
   - Converts source code to tokens
   - Supports Musab keywords: var, const, fn, struct, enum, use

2. **Parser** (`musab_parse.c`)
   - Syntax analysis
   - Builds Abstract Syntax Tree (AST)
   - Handles declarations, expressions, and statements

3. **Type System** (`musab_type.c`)
   - Type checking
   - Supports: int, float, string, bool, void
   - Arrays and structs

4. **Code Generator** (`musab_codegen.c`)
   - Generates x86-64 assembly
   - Stack management
   - Function calls

## Building

```bash
make
```

## Usage

```bash
./musab-cc program.ms -o program
./program
```

## Supported Types

- `int` - 64-bit integer
- `float` - 64-bit floating point
- `string` - String type
- `bool` - Boolean (true/false)
- `Type[]` - Arrays
- `struct` - Composite types
- `enum` - Enumeration types

## Control Structures

- `if ... else if ... else`
- `switch ... case ... default`
- `while` loops
- `for` loops
- `break` and `continue`
- `return` statements

## Built-in Functions

- `io.out(...)` - Print output
- `io.in()` - Read input

## Development Status

- [x] Tokenizer for Musab syntax
- [x] Basic type system
- [ ] Parser implementation
- [ ] Semantic analysis
- [ ] Code generation
- [ ] Built-in I/O functions
- [ ] Standard library

## References

- Based on chibicc: https://github.com/rui314/chibicc
- C11 standard
- x86-64 System V ABI
