# Compiler's Design and Construction
This repository contains the code for a compiler for the Jack programming language from the book Elements of Computing Systems. The compiler consists of 5 main components: the lexical analyser (Lexer.cpp), the parser (Parser.cpp), the symbol table (SymbolTable.cpp), the semantic analyser (Implemented in Parser), and code generation (Implemented in Parser).

## Compiler Usage Instructions
This code use's makefiles. To compile the compiler, open a terminal, ‘cd’ to the directory of the compiler and type the command ‘make’, an executable called
‘compiler’ should be created. The compiler is invoked at the command line. To run the compiler on a Jack program directory called myprog that is located in the same directory as the compiler, the compiler would be invoked by typing this at the command line:
~~~
./compiler myprog
~~~
