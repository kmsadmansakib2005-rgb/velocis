# Velocis

**Velocis** is a mini programming language and compiler project developed for the BUP CSE Compiler Laboratory.

## Project Status

🚧 Under development.

## Planned Technologies

- **Flex** — lexical analysis
- **Bison** — syntax analysis
- **Abstract Syntax Tree (AST)** — program representation
- **Semantic analysis** — type and variable checking
- **LLVM** — intermediate representation and code generation

## Current Progress

- [x] Initial lexer rules for keywords, identifiers, literals, operators, and punctuation
- [x] Initial Bison parser structure
- [x] Basic declaration and arithmetic-expression grammar
- [ ] Complete lexical error handling
- [ ] Operator precedence and grammar validation
- [ ] AST construction
- [ ] Semantic analysis
- [ ] LLVM integration
- [ ] Testing and documentation

## Example Syntax

```text
int age = 20;
string name = sakib;
print(age + 5);
```

*Note: The language syntax and compiler implementation are still being developed.*

## Build Requirements

- Flex or WinFlexBison
- Bison or WinFlexBison
- GCC-compatible C compiler
- LLVM (planned)

## Academic Context

Developed as a project for the Compiler Laboratory course at Bangladesh University of Professionals (BUP).
