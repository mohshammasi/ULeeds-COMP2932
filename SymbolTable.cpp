#include "CompilerHeaders.h"

SymbolTable::SymbolTable() {
    staticCounter = 0;
    fieldsCounter = 0;
    argumentsCounter = 0;
    localsCounter = 0;
}


// Determine the offset of the passed symbol based on its kind and add it
void SymbolTable::AddSymbol(Symbol newSymbol) {
    if (newSymbol.kind == Symbol::STATIC)
        newSymbol.offset = staticCounter++;
    else if (newSymbol.kind == Symbol::field)
        newSymbol.offset = fieldsCounter++;
    else if (newSymbol.kind == Symbol::argument)
        newSymbol.offset = argumentsCounter++;
    else if (newSymbol.kind == Symbol::var)
        newSymbol.offset = localsCounter++;

    table.push_back(newSymbol);
}


bool SymbolTable::FindSymbol(std::string name) {
    if (table.size() > 0) {
        for (Symbol s: table) {
            if (s.name == name)
                return true;
        }
        return false;
    }
    else
        return false;
}


// All functions below are only called after FindSymbol
std::string SymbolTable::GetSymbolOffset(std::string name) {
    for (Symbol s: table) {
        if (s.name == name)
            return std::to_string(s.offset);
    }
    return "";
}


Symbol::symbolKind SymbolTable::GetSymbolKind(std::string name) {
    for (Symbol s: table) {
        if (s.name == name)
            return s.kind;
    }
    Symbol s;
    return s.kind;
}


std::string SymbolTable::GetSymbolType(std::string name) {
    for (Symbol s: table) {
        if (s.name == name)
            return s.type;
    }
    return "";
}


void SymbolTable::SetInitialised(std::string name) {
    for (Symbol &s: table) {
        if (s.name == name)
            s.initialised = true;
    }
}


bool SymbolTable::IsInitialised(std::string name) {
    for (Symbol s: table) {
        if (s.name == name)
            return s.initialised;
    }
    return false;
}


// Used during development
void SymbolTable::PrintSymbolTable() {
    for (Symbol s: table) {
        std::cout << s.name << ", " << s.type << ", ";
        switch (s.kind) {
            case 0:
                std::cout << "static" << ", ";
                break;

            case 1:
                std::cout << "field" << ", ";
                break;

            case 2:
                std::cout << "argument" << ", ";
                break;

            case 3:
                std::cout << "var" << ", ";
                break;

            case 4:
                std::cout << "subroutine" << ", ";
                break;

            case 5:
                std::cout << "classtype" << ", ";
                break;
        }
        std::cout << s.offset << ", " << s.initialised << ", ";
        for (std::string a: s.arguments) {
            std::cout << a << " ";
        }
        std::cout << std::endl;
    }
}
