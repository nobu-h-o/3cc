# 3cc - C Compiler with LLVM Backend

A modern C-like compiler built with C++, LLVM, Flex, and Bison. 3cc is part of a compiler evolution series demonstrating the progression from basic hand-written compilers to modern compiler infrastructures.

## Compiler Evolution Series

This project is the third iteration in a compiler learning series:

### 1cc → 2cc → 3cc

| Stage | 1cc | 2cc | 3cc |
|-------|-----|-----|-----|
| **Lexer** | Hand-written tokenizer | **Flex** | Flex |
| **Parser** | Recursive descent | **Bison** | Bison |
| **Language** | C | C | **C++** |
| **Code Gen** | x86-64 Assembly | ARM64 Assembly | **LLVM IR** |
| **Target** | x86-64 only | ARM64 only | **Multi-arch** |
| **Build** | Make | Make | **CMake** |

### Major Transitions

**1cc → 2cc: Introduction of Formal Tools**

The transition from 1cc to 2cc introduced professional compiler-building tools:

- **Flex (lexer.l)**: Replaced manual character-by-character scanning with declarative pattern matching
  ```c
  // 1cc: Manual tokenization
  if (strncmp(p, "return", 6) == 0 && !isalnum(p[6])) {
      cur = cur->next = new_token(TK_RETURN, p, p + 6);
      p += 6;
  }

  // 2cc: Flex pattern matching
  "return"    { return RETURN; }
  ```

- **Bison (parser.y)**: Replaced hand-written recursive descent parser with formal grammar
  ```c
  // 1cc: Manual parsing
  Node *add(Token **rest, Token *tok) {
      Node *node = mul(&tok, tok);
      while (equal(tok, "+")) {
          tok = tok->next;
          node = new_binary(ND_ADD, node, mul(&tok, tok));
      }
      *rest = tok;
      return node;
  }

  // 2cc: Bison grammar
  expr:
      expr ADD term { $$ = ast_binary_op(AST_ADD, $1, $3); }
      | term { $$ = $1; };
  ```

**2cc → 3cc: LLVM Backend**

The transition from 2cc to 3cc modernized the code generation:

- **LLVM IR**: Replaced direct assembly output with portable intermediate representation
  ```c
  // 2cc: Direct ARM64 assembly
  printf("  mov w0, #%d\n", value);
  printf("  str w0, [sp, %d]\n", stack_depth);

  // 3cc: LLVM C++ API
  auto *value = llvm::ConstantInt::get(*context, llvm::APInt(32, number, true));
  builder->CreateStore(value, alloca);
  ```

- **Multi-architecture**: Single codebase compiles to x86-64, ARM64, etc.
- **Optimization**: Access to LLVM's optimization passes
- **C++ STL**: Modern data structures and memory management

## Why This Progression Matters

### 1cc: Foundation
- **Purpose**: Understand basic compiler concepts
- **Learning**: Tokenization, parsing, assembly generation
- **Trade-off**: Complete control but verbose and error-prone

### 2cc: Professional Tools
- **Purpose**: Learn industry-standard tools (Flex/Bison)
- **Learning**: Formal grammars, pattern matching, separation of concerns
- **Trade-off**: Less control but more maintainable and extensible

### 3cc: Modern Infrastructure
- **Purpose**: Learn modern compiler backends and portability
- **Learning**: IR design, multi-architecture compilation, optimization
- **Trade-off**: Higher abstraction but professional-grade output

## Features

3cc supports a simplified C-like language:

- **Data Types**: 32-bit signed integers
- **Variables**: Local and global variables
- **Operators**:
  - Arithmetic: `+`, `-`, `*`, `/`
  - Comparison: `<`, `>`, `<=`, `>=`, `==`, `!=`
- **Control Flow**:
  - `while` loops
  - `for` loops with full init/condition/increment support
- **Functions**:
  - Function definitions with parameters
  - Function calls with argument passing
  - Recursive functions
- **I/O**: Built-in `print()` function
- **Return statements**: `return` keyword for function results

## Requirements

- **C++ Compiler**: clang++ or g++ with C++20 support
- **LLVM**: Version 14 or later
- **Flex**: Lexical analyzer generator
- **Bison**: Parser generator
- **CMake**: Version 3.20 or later

### Installing Dependencies

**macOS (using Homebrew):**
```bash
brew install llvm flex bison cmake
```

**Ubuntu/Debian:**
```bash
sudo apt-get install llvm-dev flex bison cmake clang
```

## Building

**Using CMake:**

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

The compiler executable will be created at `build/3cc`.

## Usage

```bash
./3cc "<source_code>" [output_file]
```

### Examples

**Simple return value:**
```bash
./3cc "main() { return 42; }" program.o
clang -o program program.o
./program
echo $?  # Prints: 42
```

**Arithmetic and output:**
```bash
./3cc "main() { x=5; print(x*x); return 0; }" program.o
clang -o program program.o
./program  # Prints: 25
```

**Recursive factorial:**
```bash
./3cc "fact(n) { if (n <= 1) return 1; return n * fact(n-1); } main() { return fact(5); }" program.o
clang -o program program.o
./program
echo $?  # Prints: 120
```

**Fibonacci:**
```bash
./3cc "fib(n) { if (n <= 1) return n; return fib(n-1) + fib(n-2); } main() { return fib(7); }" program.o
clang -o program program.o
./program
echo $?  # Prints: 13
```

## Language Syntax

### Basic Structure

Every program must have a `main()` function:

```c
main() {
    return 0;
}
```

### Variables

Variables are implicitly declared on first assignment:

```c
main() {
    x = 5;
    y = 10;
    return x + y;  // Returns 15
}
```

### Global Variables

Global variables must be declared before functions:

```c
g = 100;

add(x) {
    return x + g;
}

main() {
    return add(5);  // Returns 105
}
```

### Control Flow

**While loops:**
```c
main() {
    sum = 0;
    i = 1;
    while (i <= 10) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;  // Returns 55
}
```

**For loops:**
```c
main() {
    sum = 0;
    for (i = 1; i <= 10; i = i + 1) {
        sum = sum + i;
    }
    return sum;  // Returns 55
}
```

### Functions

**Basic functions:**
```c
mul(a, b) {
    return a * b;
}

main() {
    return mul(3, 4);  // Returns 12
}
```

**Recursive functions:**
```c
sum(n) {
    if (n <= 0) return 0;
    return n + sum(n-1);
}

main() {
    return sum(10);  // Returns 55
}
```

### Print Function

```c
main() {
    for (i = 1; i <= 5; i = i + 1) {
        print(i);
    }
    return 0;
}
```

## LLVM IR Output

The compiler generates both an object file and human-readable LLVM IR:

```bash
./3cc "main() { return 42; }" program.o
# Creates:
#   program.o  - Object file (machine code)
#   program.ll - LLVM IR (human-readable)
```

Inspect the LLVM IR:

```bash
cat program.ll
```

Example output:
```llvm
define i32 @main() {
entry:
  %retval = alloca i32
  store i32 42, i32* %retval
  %0 = load i32, i32* %retval
  ret i32 %0
}
```

This is useful for:
- Understanding how source code maps to IR
- Debugging compilation issues
- Learning LLVM IR design
- Comparing optimization effects

## Running Tests

```bash
cd test
./test.sh
```

The test suite validates:
- Arithmetic operations
- Variable assignments (local and global)
- Comparison operators
- Control flow (while, for)
- Function calls with multiple parameters
- Recursive functions
- Print statements
- Nested loops

## Project Structure

```
3cc/
├── CMakeLists.txt      # Build configuration
├── README.md           # This file
├── lexer.l             # Flex lexer (pattern matching)
├── parser.y            # Bison parser (formal grammar)
├── ast.h/.cpp          # Abstract Syntax Tree
├── symtab.h/.cpp       # Symbol table management
├── codegen.h/.cpp      # LLVM IR code generator
├── main.cpp            # Compiler driver
└── test/
    └── test.sh         # Test suite
```

## Implementation Comparison

### Lexical Analysis

**1cc (Hand-written):**
```c
// Manual character scanning
while (*p) {
    if (isspace(*p)) { p++; continue; }
    if (isdigit(*p)) { /* create number token */ }
    if (strncmp(p, "return", 6) == 0) { /* create return token */ }
    if (isalpha(*p)) { /* create identifier token */ }
    if (ispunct(*p)) { /* create punctuation token */ }
}
```

**2cc/3cc (Flex):**
```flex
%%
"return"    { return RETURN; }
"while"     { return WHILE; }
[0-9]+      { yylval.number = atoi(yytext); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.string = strdup(yytext); return IDENTIFIER; }
"+"         { return ADD; }
"-"         { return SUB; }
%%
```

### Parsing

**1cc (Recursive Descent):**
```c
// Manual parsing functions
Node *add(Token **rest, Token *tok) {
    Node *node = mul(&tok, tok);
    while (equal(tok, "+") || equal(tok, "-")) {
        if (equal(tok, "+")) {
            tok = tok->next;
            node = new_binary(ND_ADD, node, mul(&tok, tok));
        }
    }
    *rest = tok;
    return node;
}
```

**2cc/3cc (Bison):**
```yacc
%%
expr:
    expr ADD term { $$ = ast_binary_op(AST_ADD, $1, $3); }
    | expr SUB term { $$ = ast_binary_op(AST_SUB, $1, $3); }
    | term { $$ = $1; }
    ;
%%
```

### Code Generation

**1cc (x86-64 Assembly):**
```c
printf("  mov rax, %d\n", node->val);
printf("  push rax\n");
printf("  pop rdi\n");
printf("  add rax, rdi\n");
```

**2cc (ARM64 Assembly):**
```c
printf("  mov w0, #%d\n", value);
printf("  str w0, [sp, %d]\n", stack_depth);
printf("  ldr w0, [sp, %d]\n", stack_depth);
printf("  add w0, w0, w1\n");
```

**3cc (LLVM IR):**
```cpp
auto *value = llvm::ConstantInt::get(*context, llvm::APInt(32, number, true));
auto *lhs_val = builder->CreateLoad(llvm::Type::getInt32Ty(*context), lhs);
auto *rhs_val = builder->CreateLoad(llvm::Type::getInt32Ty(*context), rhs);
auto *result = builder->CreateAdd(lhs_val, rhs_val, "addtmp");
```

## Learning Path

If you're learning compiler construction:

1. **Start with 1cc**: Understand the fundamentals by implementing everything from scratch
2. **Move to 2cc**: Learn how Flex and Bison simplify lexer/parser implementation
3. **Study 3cc**: Understand modern backends and how LLVM provides portability

Each stage builds on the previous one, introducing industry-standard tools and techniques.

## Code Reuse from 2cc

The frontend (lexer and parser) were reused from 2cc with minimal changes:

1. **Lexer** (`lexer.l`): Nearly identical, only header includes changed from C to C++
2. **Parser** (`parser.y`): Same grammar rules, updated for C++ headers
3. **AST Structure**: Same node types, ported to C++ classes
4. **Symbol Table**: Same interface, reimplemented using C++ STL containers

The major rewrite was the **code generator**, which was completely changed from ARM64 assembly to LLVM IR.

## Advantages of Each Approach

### 1cc Advantages
- Complete understanding of every component
- No external dependencies (just C compiler)
- Direct control over assembly output
- Simple to debug

### 2cc Advantages
- Formal grammar specification (easier to extend)
- Automatic error handling in parser
- Pattern-based lexing (more maintainable)
- Industry-standard tools

### 3cc Advantages
- Multi-architecture support (compile once, run anywhere)
- Professional optimization passes
- Human-readable IR for debugging
- Integration with LLVM ecosystem
- Modern C++ features and safety

## Future Enhancements

Potential improvements for 3cc:

- [ ] Additional data types (char, float, pointers)
- [ ] Arrays and strings
- [ ] Structs
- [ ] If/else statements
- [ ] Logical operators (`&&`, `||`, `!`)
- [ ] Bitwise operators
- [ ] Pointer arithmetic
- [ ] Standard library integration
- [ ] Better error messages with line numbers
- [ ] LLVM optimization level flags (-O0, -O1, -O2, -O3)

## References

- [LLVM Documentation](https://llvm.org/docs/)
- [Flex Manual](https://github.com/westes/flex)
- [Bison Manual](https://www.gnu.org/software/bison/manual/)
- [chibicc](https://github.com/rui314/chibicc) - Inspiration for compiler design
- [低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)

## License

This is an educational project for learning compiler construction.
