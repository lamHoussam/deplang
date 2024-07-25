# DepLang

Following the LLVM kaleidoscope tutorial: <https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl01.html>

## Description

The goal of this project is to make a programming language that implements strong typing with the concept of dependant types.
The idea is to try to exclude other classes of programming errors through converting some runtime errors to compile time errors. Through dependant types, the compiler will be able to ensure a certain degree of correctness of a program.
The inspiration came after reading the following paper: <https://www.cs.cmu.edu/~fp/papers/popl99.pdf>

## Important features of the language

- Strong typing
- Dependant types

## Syntax of the language

```
program = statement_list
statement_list = statement ";" statement_list

statement = variable_declaration
            | variable_assignment
            | function_call
            | function_prototype

variable_declaration = "let" identifier ("as" identifier)? "=" expression
variable_assignment = identifier "=" expression
function_call = identifier "(" function_arguments_list ")"
function_prototype = "func" identifier "(" function_parameters_list ")" "->" identifier

function_arguments_list = (function_argument ("," function_arguments_list)?)
function_parameters_list = (function_parameter ("," function_parameters_list)?)

function_argument = expression
function_parameter = identifier ":" identifier

expression = ""
            | identifier
            | "(" expression ")"



identifier = ([a-z]|[A-Z])([a-z]|[A-Z]|[0-9])
```

