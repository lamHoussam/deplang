# DepLang

Following the LLVM kaleidoscope tutorial: <https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl01.html>

## Description

The goal of this project is to make a programming language that implements strong typing with the concept of dependant types.
The idea is to try to exclude other classes of programming errors through converting some runtime errors to compile time errors. Through dependant types, the compiler will be able to ensure a certain degree of correctness of a program.
The inspiration came after reading the following paper: <https://www.cs.cmu.edu/~fp/papers/popl99.pdf>

## Important features of the language

- Strong typing
- Dependant types
- Pattern Matching

## Syntax of the language

!!! Needs to be modified !!!

```
program = statement_list
statement_list = statement ";" statement_list

statement = variable_declaration
            | variable_assignment
            | function_call
            | function_prototype

variable_declaration = "let" identifier (":" identifier)? "=" expression
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

type_declaration = "type" identifier "=" type_definitions ";"


identifier = ([a-z]|[A-Z])([a-z]|[A-Z]|[0-9])
```

## Type System

The main idea of the language is to implement a certain degree of dependant types as described in the paper.
One of the most important points is implementing type constructors.
We can see types as unions of values, for example: 
```
type Color = 'RED' | 'BLUE' | 'GREEN';
```

and so all values of type Color would have one of the three given values, this looks like a simple enum type.
but we can also define types as a union of 2 types, for example:

```
type Value = float | list<float>;
```
This resembles union types in C.
We can also define types as products of 2 types, for example:

```
type Complex = float * float;
```

With all these different type definitions, we should add pattern matching to the language.

We can try to use the following syntax for dependant types: 


```
type List{T: type, n: int} where n > 0 = [] | T * List{T, n-1};
```

This way we say that the List type depends on the type T which will be the type of the elements of the list, and also depends on the value n (where n > 0) which is the length of the list, that can either be a '[]' or a list with n-1 elements and an element of type T

Now let's implement functions for the List type:

```
List{T, n}::append(List{T, m: int} other) -> List{T, m + n} {
    return match other {
        case [] -> self | 
        case el * l -> (self * el)::append(l)
    }
}

List{T, n}::get_element_at(i: int{(forall x: int)::(x < n)}) -> T {
    return match self {
        case [] -> default(T) |
        case el * l -> if (i == 0) { el } else { l::get_element_at(i-1) }
    }
}

```

