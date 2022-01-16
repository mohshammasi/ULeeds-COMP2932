#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <iostream>
#include <vector>

/****************** Symbol and SymbolTable class definitions *****************/
class Symbol {
public:
    enum symbolKind {STATIC, field, argument, var, subroutine, identifier};
    symbolKind kind;
    std::string name;
    std::string type;
    int offset = 0;
    bool initialised = false; // Used for initialisation semantic check
    std::vector <std::string> arguments; // For subroutines in program SymbolTable
};

class SymbolTable {
public:
    SymbolTable();
    std::vector <Symbol> table;
    int staticCounter;
    int fieldsCounter;
    int argumentsCounter;
    int localsCounter;

public:
    void AddSymbol(Symbol newSymbol);
    bool FindSymbol(std::string name);
    std::string GetSymbolOffset(std::string name);
    Symbol::symbolKind GetSymbolKind(std::string name);
    std::string GetSymbolType(std::string name);
    void SetInitialised(std::string name);
    bool IsInitialised(std::string name);
    void PrintSymbolTable();
};

#endif
