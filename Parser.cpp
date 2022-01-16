#include <cstdlib>
#include <vector>
#include <dirent.h>
#include <sstream>
#include "CompilerHeaders.h"

Parser::Parser() {
    SymbolTable s;
    symbolTables.push_back(s);
    currentSymbolTable = 0; // Program SymbolTable i.e. across files
}


// Initialise the lexer and produce the tokens
bool Parser::Init(std::string filename) {
    if(l.ExtractSourceFile(filename)) {
        if (l.ProduceTokens()) {
            return true;
        }
        else {
            std::cout << "Tokens production failed." << std::endl;
            return false;
        }
    }
    else {
        std::cout << "Source file extraction failed." << std::endl;
        return false;
    }
}


void Parser::AddJackOS() {
    /* Create a VmFile object to write the code generated from JackOS to, this is to
     * silence it while its parsing the JackOS files, the VmFile object will be deleted */
    VmFile file;
    vmFiles.push_back(file);

    DIR *jackOS;
    struct dirent *jackFile;
    char *path = (char*)"JackOS/";
    if ((jackOS = opendir(path)) != nullptr) {
        while ((jackFile = readdir(jackOS)) != nullptr) {
            std::string filename = jackFile->d_name;
            if (filename.substr(filename.find_last_of(".") + 1) == "jack") {
                std::string filePath = path + filename;
                bool init = Init(filePath);
                if (init) {
                    ClassDeclar();
                }
            }
        }
        closedir(jackOS);
        /* Clear all the lists used to store 'resolvable' declarations that
           were added from JackOS files */
        varDeclarations.clear();
        arrayIndices.clear();
        subroutineCalls.clear();
        assignments.clear();
        returns.clear();
        vmFiles.clear();
    }
    else {
        // Could not open directory
        perror("");
        std::cout << "Couldn't open directory " << path << std::endl;
        exit(0);
    }
}


// Calls all the semantic checks functions and issues errors/warnings at the end
void Parser::ResolveAllDeclars() {
    // Some expressions contain function calls, so resolve the call to its return type
    /* Im treating field and static variables as subroutines because they are
     * accessed using '.' operator same as subroutines */
    for (Symbol s: symbolTables[0].table) {
        if (s.kind == Symbol::subroutine || s.kind == Symbol::field ||
            s.kind == Symbol::STATIC) {
            ResolveSubroutinesReturnType(subroutineCalls, s);
            ResolveSubroutinesReturnType(assignments, s);
            ResolveSubroutinesReturnType(returns, s);
            ResolveSubroutinesReturnType(arrayIndices, s);
            /* When writing code for calls I write extra pop statements after each
             * call for void functions, if the call wasnt for a void function I remove it */
            RemovePopCode(s);
        }
    }

    // Evaluate all the expressions found in the program
    EvaluateExpressions(subroutineCalls); // expressions found in calls
    EvaluateExpressions(assignments); // expressions found in assignments
    EvaluateExpressions(returns); // expressions found in return statements
    EvaluateExpressions(arrayIndices); // expressions found in array indices
    CheckReturnsCompatibility();
    CheckArrayIndices();

    /* Resolve all variable declarations with identifier types and all subroutine
     * calls using the program SymbolTable */
    for (Symbol s: symbolTables[0].table) {
        if (s.kind == Symbol::subroutine || s.kind == Symbol::field ||
            s.kind == Symbol::STATIC)
            ResolveSubroutineCall(s);
        else if (s.kind == Symbol::identifier)
            ResolveVarDeclar(s.name);
    }

    // Check if assignments LHS and RHS are compatible
    for (declaration &d: assignments) {
        bool compatible = CheckCompatibility(d.LHS, d.RHS);
        if (compatible)
            d.argsMatch = true;
    }

    // Error report for any variable not resolved
    for (declaration d: varDeclarations) {
        if (!d.resolved)
            ResolveError(d, "Unknown type '" + d.type + "'.");
    }

    // Error report for any subroutine call not resolved, or call arguments not matching
    for (declaration d: subroutineCalls) {
        if (d.type.empty()) { // I dont store types for methods and functions
            if (d.resolved && !d.argsMatch)
                ResolveWarning(d, "call arguments do not match subroutine declaration.");
            else if (!d.resolved)
                ResolveError(d, "Unknown subroutine '" + d.name + "()'.");
        }
        else { // constructor
            if (d.resolved && !d.argsMatch)
                ResolveWarning(d, "call arguments do not match constructor declaration.");
            else if (!d.resolved)
                ResolveError(d, "Unknown constructor '" + d.type + "." + d.name + "()'.");
        }
    }

    // Generate warnings for any incompatible assignment statement
    // Cant be too strict and issue it as an error because of Jack
    for (declaration d: assignments) {
        if (!d.argsMatch)
            ResolveWarning(d, "The type '" + d.LHS + "' is not compatible with '" + d.RHS + "'.");
    }

    // Generate warnings for any incompatible return statement
    // Cant be too strict and issue it as an error because of Jack
    for (declaration d: returns) {
        if (!d.argsMatch)
            ResolveWarning(d, "The type '" + d.type + "' is not compatible with " + d.arguments[0] + "'.");
    }
}


/* Resolve identifier types found, Main is stored as a class in the program Symbol
 * table so skip it as its not a valid type. */
void Parser::ResolveVarDeclar(std::string type) {
    for (declaration &d: varDeclarations) {
        if (type == "Main")
            continue;
        else if (d.type == type)
            d.resolved = true;
    }
}


/* Resolve all subroutine calls found, constructors are seperated from functions
 * and methods because their type is stored. */
void Parser::ResolveSubroutineCall(Symbol s) {
    for (unsigned int i=0; i < subroutineCalls.size(); i++) {
        if (subroutineCalls[i].type.empty()) { // means its a function or method
            if (subroutineCalls[i].name == s.name) { // Match subroutine name first
                subroutineCalls[i].resolved = true; // Subroutine is found in the program ST.
                if (subroutineCalls[i].arguments.size() == s.arguments.size()) { // Match argument size
                    subroutineCalls[i].argsMatch = true; // Assume arguments match to start with
                    for (unsigned int j=0; j < subroutineCalls[i].arguments.size(); j++) {
                        if (subroutineCalls[i].arguments[j] != s.arguments[j]) {
                            bool compatible;
                            compatible = CheckCompatibility(s.arguments[j], subroutineCalls[i].arguments[j]);
                            if (!compatible)
                                subroutineCalls[i].argsMatch = false;
                        }
                    }
                }
            }
        }
        else {
            /* If the subroutine call has a type (Type is the class name), means
             * its a constructor. (I am only storing call types for constructors,
             * implementation choice) */
            if ((subroutineCalls[i].name == s.name) && (subroutineCalls[i].type == s.type)) {
                subroutineCalls[i].resolved = true; // Subroutine is found in the program ST.
                if (subroutineCalls[i].arguments.size() == s.arguments.size()) { // Match argument size
                    subroutineCalls[i].argsMatch = true; // Assume arguments match to start with
                    for (unsigned int j=0; j < subroutineCalls[i].arguments.size(); j++) {
                        if (subroutineCalls[i].arguments[j] != s.arguments[j]) {
                            bool compatible = CheckCompatibility(s.arguments[j], subroutineCalls[i].arguments[j]);
                            if (!compatible)
                                subroutineCalls[i].argsMatch = false;
                        }
                    }
                }
            }
        }
    }
}


/* Some expressions have subroutine calls inside of them, resolve them to their
 * type to be able to evaluate the expressions compatibility. */
void Parser::ResolveSubroutinesReturnType(std::vector<declaration> &list, Symbol s) {
    for (declaration &d: list) {
        for (std::string &t: d.arguments) {
            if (t == s.name)
                t = s.type;
        }
    }
}


/* The return expression is stored in arguments, the type of the expression is left in
 * arguments[0] after it is evaluated so compare for compatibility. */
void Parser::CheckReturnsCompatibility() {
    for (declaration &d: returns) {
        bool compatible;
        if (d.arguments.size() == 0) // no return type, means void
            compatible = CheckCompatibility(d.type, "");
        else
            compatible = CheckCompatibility(d.type, d.arguments[0]);

        if (compatible)
            d.argsMatch = true;
    }
}


/* For the semantic check array index must evaluate to 'int', only the 'int' or
 * 'char' or 'ArrayEntry' types are allowed. */
void Parser::CheckArrayIndices() {
    for (declaration &d: arrayIndices) {
        if (d.arguments[0] != "int" && d.arguments[0] != "char" &&
            d.arguments[0] != "ArrayEntry")
            ResolveError(d, "Array index must evaluate to an 'int' value.");
    }
}


/* Evaluate expressions, every time an operator is encountered both sides of it
 * are checked for type compatibility and deleted, reported if incompatible. */
void Parser::EvaluateExpressions(std::vector<Parser::declaration> &v) {
    for (unsigned int i=0; i < v.size(); i++) {
        for (unsigned int j=0; j < v[i].arguments.size(); j++) {
            if (v[i].arguments[j] == "*" || v[i].arguments[j] == "/" ||
                v[i].arguments[j] == "+" || v[i].arguments[j] == "-" ||
                v[i].arguments[j] == "<" || v[i].arguments[j] == ">" ||
                v[i].arguments[j] == "=") {
                bool compatible = CheckCompatibility(v[i].arguments[j-1], v[i].arguments[j+1]);
                if (compatible) {
                    v[i].arguments.erase(v[i].arguments.begin() + j); // Delete the operator
                    v[i].arguments.erase(v[i].arguments.begin() + j);  // Delete the type
                    j=0; // Reset the counter because something got deleted
                }
                else {
                    ResolveError(v[i], "Cant perform operation '" + v[i].arguments[j] +
                      "' on non compatible types '" + v[i].arguments[j-1] + "' and '" +
                      v[i].arguments[j+1] + "'.");
                }
            }
        }
        // For LHS and RHS compatibility
        if (!v[i].arguments.empty())
            v[i].RHS = v[i].arguments[0];
    }
}


// The ruleset of types compatibility, all checks use this function.
bool Parser::CheckCompatibility(std::string t1, std::string t2) {
    if ((t1 == "int" || t1 == "char") && (t2 == "int" || t2 == "char"))
        return true;
    else if (t1 == "boolean" && t2 == "boolean")
        return true;
    else if (t2 == "null")
        return true;
    else if (t1 == "ArrayEntry" || t2 == "ArrayEntry")
        return true;
    else if (t1 == "Array") // because array is like the "object" class
        return true;
    else if (t1 == "void" && t2.empty())
        return true;
    else if (t1 == t2)
        return true;
    else
        return false;
}


void Parser::Error(Token t, std::string message) {
    unsigned int index = vmFiles.size() - 1;
    std::cout << vmFiles[index].filename << ".jack: Error, line " << t.lineNum
              << ", at or near '" << t.lexeme << "', " << message <<std::endl;
    exit(0);
}


void Parser::Warning(Token t, std::string message) {
    unsigned int index = vmFiles.size() - 1;
    std::cout << vmFiles[index].filename << ".jack: Warning, line " << t.lineNum
              << ", at or near '" << t.lexeme << "', " << message <<std::endl;
}


void Parser::ResolveError(Parser::declaration d, std::string message) {
    std::cout << d.filename << ".jack: Error, line " << d.lineNum << ", "
              << message << std::endl;
    exit(0);
}


void Parser::ResolveWarning(Parser::declaration d, std::string message) {
    std::cout << d.filename << ".jack: Warning, line " << d.lineNum << ", "
              << message << std::endl;
}


void Parser::WriteCode(std::string vmCode) {
    unsigned long currentFile = vmFiles.size() - 1;
    vmFiles[currentFile].vmCode.push_back(vmCode);
}


/* When writing code for 'do sub', extra 'pop temp 0' statements are inserted for
 * void functions, if the function isnt void remove the pop statement. */
void Parser::RemovePopCode(Symbol s) {
    for (unsigned int i=0; i < vmFiles.size(); i++) {
        for (unsigned int j=0; j < vmFiles[i].vmCode.size(); j++) {
            if (vmFiles[i].vmCode[j] == s.name && s.type == "void")
                vmFiles[i].vmCode.erase(vmFiles[i].vmCode.begin()+j);
            else if (vmFiles[i].vmCode[j] == s.name && s.type != "void") {
                vmFiles[i].vmCode.erase(vmFiles[i].vmCode.begin()+j);
                vmFiles[i].vmCode.erase(vmFiles[i].vmCode.begin()+j);
            }
        }
    }
}


// Write the VM code for each file to a file
void Parser::WriteVmFiles(std::string path) {
    for (VmFile f: vmFiles) {
        std::ofstream file(path + '/' + f.filename + ".vm");
        for (std::string code: f.vmCode)
            file << code << std::endl;
        file.close();
    }
}


// Used for creating labels for code generation
std::string Parser::CreateLabel() {
    return "l" + std::to_string(labelCounter++);
}


/* Gets passed the 'identifier.identifier()'. Second identifier might not exist i.e.
 * 'do something()'. The if statements are based on how I store them. */
std::string Parser::GetNumOfArgs(std::string name, std::string type) {
    unsigned int size = 0;
    std::vector <std::string> functionArgs;
    for (declaration d: subroutineCalls) {
        if (d.name == "new") {
            if (d.type == name) {
                size = d.arguments.size();
                functionArgs = d.arguments;
            }
        }
        else if (d.name == name && type.empty()) {
            size = d.arguments.size();
            functionArgs = d.arguments;
        }
        else if (d.name == type && !name.empty()) {
            size = d.arguments.size();
            functionArgs = d.arguments;
        }
    }

    /* Because the expressions havent been evaluated yet for semantics so just
     * find the size without removing anything */
    for (std::string s: functionArgs) {
        if (s == "*" || s == "/" || s == "+" || s == "-" ||
            s == "<" || s == ">" || s == "=") {
            size = size - 2;
        }
        else if (s == "ArrayEntry") // Because ArrayEntry has index but its only 1 thing
            size = size - 1;
    }
    return std::to_string(size);
}


void Parser::ClassDeclar() {
    labelCounter = 0; // reset labels just for convience of reading the code
    // Create and switch to the SymbolTable for the class scope
    SymbolTable newSymbolTable;
    symbolTables.push_back(newSymbolTable);
    currentSymbolTable = 1;
    Symbol s;
    s.kind = Symbol::identifier;

    Token t = l.GetNextToken();
    if (t.lexeme == "class")
        s.type = t.lexeme;
    else
        Error(t, "Expected keyword 'class'.");

    t = l.GetNextToken();
    if (t.type == t.identifier) {
        currentClass = t.lexeme;
        if (symbolTables[currentSymbolTable-1].FindSymbol(t.lexeme))
            Error(t, "Redeclaration of identifier.");
        s.name = t.lexeme;
        symbolTables[currentSymbolTable-1].AddSymbol(s); // Program ST
    }
    else
        Error(t, "Expected an identifier.");

    t = l.GetNextToken();
    if (t.lexeme == "{")
        ;
    else
        Error(t, "Expected a '{'.");

    t = l.PeekNextToken();
    while (t.lexeme != "}") {
        MemberDeclar();
        t = l.PeekNextToken();
    }
    l.GetNextToken();       // Consume the '}'

    symbolTables.erase(symbolTables.begin() + 1);
    currentSymbolTable = 0; // Switch to the program Symbol Table
    l.DeleteEOF();
}


void Parser::MemberDeclar() {
    Token t = l.PeekNextToken();
    if (t.lexeme == "field" || t.lexeme == "static")
        ClassVarDeclar();
    else if (t.lexeme == "method" || t.lexeme == "function" || t.lexeme == "constructor")
        SubroutineDeclar();
    else
        Error(t, "Expected a class variable or subroutine declaration.");
}


void Parser::ClassVarDeclar() {
    Symbol s;
    s.initialised = true;

    Token t = l.GetNextToken();
    if (t.lexeme == "field" || t.lexeme == "static") {
        if (t.lexeme == "static")
            s.kind = Symbol::STATIC;
        else if (t.lexeme == "field")
            s.kind = Symbol::field;
    }
    else
        Error(t, "Expected keyword 'field' or 'static'.");

    // Get the symbol type
    s.type = l.PeekNextToken().lexeme;
    Type();

    t = l.GetNextToken();
    if (t.type == t.identifier) {
        if (symbolTables[currentSymbolTable].FindSymbol(t.lexeme))
            Error(t, "Redeclaration of identifier.");
        // Add the symbol to the class and program SymbolTable
        s.name = t.lexeme;
        symbolTables[currentSymbolTable].AddSymbol(s);
        symbolTables[currentSymbolTable-1].AddSymbol(s);
    }
    else
        Error(t, "Expected an identifier.");

    t = l.PeekNextToken();
    while (t.lexeme == ",") {
        l.GetNextToken();     // Consume the ','

        t = l.GetNextToken();
        if (t.type == t.identifier) {
            if (symbolTables[currentSymbolTable].FindSymbol(t.lexeme))
                Error(t, "Redeclaration of identifier.");
            // if there are more add to the class and program SymbolTable
            s.name = t.lexeme;
            symbolTables[currentSymbolTable].AddSymbol(s);
            symbolTables[currentSymbolTable-1].AddSymbol(s);
        }
        else
            Error(t, "Expected an identifier.");

        t = l.PeekNextToken();
    }

    t = l.GetNextToken();
    if (t.lexeme == ";")
        ;
    else
        Error(t, "Expected a ';'.");
}


void Parser::Type() {
    Token t = l.GetNextToken();
    if (t.lexeme == "int")
        ;
    else if (t.lexeme == "char")
        ;
    else if (t.lexeme == "boolean")
        ;
    else if (t.type == t.identifier) {
        // For identifier types semantics check
        unsigned int index = vmFiles.size() - 1;
        declaration d;
        d.filename = vmFiles[index].filename;
        d.type = t.lexeme;
        d.lineNum = t.lineNum;
        varDeclarations.push_back(d);
    }
    else
        Error(t, "Unknown type.");
}


void Parser::SubroutineDeclar() {
    // Create and switch to the SymbolTable for the method scope
    SymbolTable newSymbolTable;
    symbolTables.push_back(newSymbolTable);
    currentSymbolTable = 2;

    // For the program SymbolTable
    Symbol s2;
    s2.kind = Symbol::subroutine;

    Token t = l.GetNextToken();
    if (t.lexeme == "method" || t.lexeme == "function" || t.lexeme == "constructor") {
        currentSubroutineKind = t.lexeme;
        if (t.lexeme == "method") {
            // Add the implicit argument of the method to the method SymbolTable
            Symbol s;
            s.name = "this";
            s.type = currentClass;
            s.kind = Symbol::argument;
            symbolTables[currentSymbolTable].AddSymbol(s);
        }
        else if (t.lexeme == "function")
            s2.kind = Symbol::STATIC;
    }
    else
        Error(t, "Expected keyword 'method' or 'function', or 'constructor'.");

    t = l.PeekNextToken();
    if (t.lexeme == "void") {
        l.GetNextToken();      // Consume the void
        s2.type = "void";
        currentSubroutineType = "void";
    }
    else {
        s2.type = l.PeekNextToken().lexeme;
        currentSubroutineType = l.PeekNextToken().lexeme;
        Type();
    }

    t = l.GetNextToken();
    if (t.type == t.identifier) {
        s2.name = t.lexeme;
        currentSubroutine = t.lexeme;
    }
    else
        Error(t, "Expected an identifier.");

    t = l.GetNextToken();
    if (t.lexeme == "(")
        //Happy(t);
        ;
    else
        Error(t, "Expected a '('.");

    // Add method declaration to program SymbolTable, for Semantic Checks
    symbolTables[0].AddSymbol(s2);
    ParamList();

    t = l.GetNextToken();
    if (t.lexeme == ")")
        ;
    else
        Error(t, "Expected a ')'.");

    // Code Generation
    WriteCode("function " + currentClass + "." + currentSubroutine + " ");
    if (currentSubroutineKind == "constructor") {
        int nFields = symbolTables[currentSymbolTable-1].fieldsCounter;
        std::string allocSize;
        allocSize = std::to_string(nFields);
        WriteCode("push constant " + allocSize);
        WriteCode("call Memory.alloc 1");
        WriteCode("pop pointer 0");
    }
    else if (currentSubroutineKind == "method") {
        WriteCode("push argument 0");
        WriteCode("pop pointer 0");
    }
    SubroutineBody();
    // Add the number of locals to the statement
    std::string nVars = std::to_string(symbolTables[currentSymbolTable].localsCounter);
    unsigned long currentFile = vmFiles.size() - 1;
    for (std::string &code: vmFiles[currentFile].vmCode) {
        if (code == "function " + currentClass + "." + currentSubroutine + " ")
            code += nVars;
    }

    symbolTables.erase(symbolTables.begin() + 2); // Delete method table
    currentSymbolTable = 1; // Switch to the class Symbol Table
}


void Parser::ParamList() {
    Token t = l.PeekNextToken();
    if (t.lexeme == ")")
        ;
    else {
        // Add symbol to method SymbolTable
        Symbol s;
        s.kind = Symbol::argument;
        s.type = l.PeekNextToken().lexeme;
        s.initialised = true; // argument symbols are considered initialised by default

        // Add the method arguments in the program SymbolTable
        unsigned long methodIndex = symbolTables[0].table.size() - 1;
        symbolTables[0].table[methodIndex].arguments.push_back(l.PeekNextToken().lexeme);
        Type();

        t = l.GetNextToken();
        if (t.type == t.identifier) {
            // Add the symbol to the method SymbolTable
            s.name = t.lexeme;
            symbolTables[currentSymbolTable].AddSymbol(s);
        }
        else
            Error(t, "Expected an identifier.");

        t = l.PeekNextToken();
        while (t.lexeme == ",") {
            l.GetNextToken();       // Consume the ','

            // If there are more arguments add them to the program SymbolTable
            s.type = l.PeekNextToken().lexeme;
            symbolTables[0].table[methodIndex].arguments.push_back(l.PeekNextToken().lexeme);
            Type();

            t = l.GetNextToken();
            if (t.type == t.identifier) {
                // if there are more add to the method SymbolTable
                s.name = t.lexeme;
                symbolTables[currentSymbolTable].AddSymbol(s);
            }
            else
                Error(t, "Expected an identifier.");

            t = l.PeekNextToken();
        }
    }
}


void Parser::SubroutineBody() {
    Token t = l.GetNextToken();
    if (t.lexeme == "{")
        ;
    else
        Error(t, "Expected a '{'.");

    // Semantic check - all code paths must return a value
    bool foundReturn = false;
    foundIfReturn = false;
    foundElseReturn = false;

    t = l.PeekNextToken();
    while (t.lexeme != "}") {
        if (t.lexeme == "return")
            foundReturn = true;
        Statement();
        t = l.PeekNextToken();
    }
    l.GetNextToken();       // Consume the '}'

    // Void functions dont have to have a return so flag it as true
    if (currentSubroutineType == "void" && !foundReturn) {
        foundReturn = true;
        WriteCode("push constant 0");
        WriteCode("return");
    }

    if (!foundReturn && !(foundIfReturn && foundElseReturn))
        Error(t, "Not all code paths return a value in subroutine '" + currentSubroutine + "'.");
}


void Parser::Statement() {
    Token t = l.PeekNextToken();
    if (t.lexeme == "var")
        VarDeclarStatement();
    else if (t.lexeme == "let")
        LetStatement();
    else if (t.lexeme == "if")
        IfStatement();
    else if (t.lexeme == "while")
        WhileStatement();
    else if (t.lexeme == "do")
        DoStatement();
    else if (t.lexeme == "return")
        ReturnStatement();
    else
        Error(t, "Unknown keyword.");
}


void Parser::VarDeclarStatement() {
    Symbol s;
    s.kind = Symbol::var;

    Token t = l.GetNextToken();
    if (t.lexeme == "var")
        ;
    else
        Error(t, "Expected keyword 'var'.");

    s.type = l.PeekNextToken().lexeme;
    Type();

    t = l.GetNextToken();
    if (t.type == t.identifier) {
        if (symbolTables[currentSymbolTable].FindSymbol(t.lexeme))
            Error(t, "Redeclaration of identifier.");
        // Add the symbol to the method SymbolTable
        s.name = t.lexeme;
        symbolTables[currentSymbolTable].AddSymbol(s);
    }
    else
        Error(t, "Expected an identifier.");

    t = l.PeekNextToken();
    while (t.lexeme == ",") {
        l.GetNextToken();     // Consume the ','

        t = l.GetNextToken();
        if (t.type == t.identifier) {
            if (symbolTables[currentSymbolTable].FindSymbol(t.lexeme))
                Error(t, "Redeclaration of identifier.");
            // if there are more add to the method SymbolTable
            s.name = t.lexeme;
            symbolTables[currentSymbolTable].AddSymbol(s);
        }
        else
            Error(t, "Expected an identifier.");

        t = l.PeekNextToken();
    }

    t = l.GetNextToken();
    if (t.lexeme == ";")
        ;
    else
        Error(t, "Expected a ';'.");
}


void Parser::LetStatement() {
    expression.clear(); // empty the vector used to store expressions to avoid
    Token t = l.GetNextToken();
    unsigned int index = vmFiles.size() - 1;
    declaration d;
    d.filename = vmFiles[index].filename;
    d.lineNum = t.lineNum;
    if (t.lexeme == "let")
        ;
    else
        Error(t, "Expected keyword 'let'.");

    t = l.GetNextToken();
    std::string assignedTo;
    if (t.type == t.identifier) {
        assignedTo = t.lexeme;
        // Variable must be declared before being used
        if (!(symbolTables[currentSymbolTable].FindSymbol(t.lexeme)) &&
            !(symbolTables[currentSymbolTable-1].FindSymbol(t.lexeme))) {
            Error(t, "Variable must be declared before being used.");
        }

        // Find and set the variable as initialised and get type for comparison with RHS
        if (symbolTables[currentSymbolTable].FindSymbol(t.lexeme)) { // Method table
            symbolTables[currentSymbolTable].SetInitialised(t.lexeme);
            d.LHS = symbolTables[currentSymbolTable].GetSymbolType(t.lexeme);
        }
        else if (symbolTables[currentSymbolTable-1].FindSymbol(t.lexeme)) { // Class table
            symbolTables[currentSymbolTable-1].SetInitialised(t.lexeme);
            d.LHS = symbolTables[currentSymbolTable-1].GetSymbolType(t.lexeme);
        }
    }
    else
        Error(t, "Expected an identifier.");

    bool isArrayEntry = false;
    t = l.PeekNextToken();
    if (t.lexeme == "[") {
        isArrayEntry = true;
        d.LHS = "ArrayEntry";
        l.GetNextToken();    // Consume the '['

        // Code Generation
        if (symbolTables[currentSymbolTable].FindSymbol(assignedTo)) { // Method table
            std::string offset = symbolTables[currentSymbolTable].GetSymbolOffset(assignedTo);
            Symbol::symbolKind k;
            k = symbolTables[currentSymbolTable].GetSymbolKind(assignedTo);
            if (k == 0)
                WriteCode("push static " + offset);
            else if (k == 1)
                WriteCode("push this " + offset);
            else if (k == 2)
                WriteCode("push argument " + offset);
            else if (k == 3)
                WriteCode("push local " + offset);
        }
        else if (symbolTables[currentSymbolTable-1].FindSymbol(assignedTo)) { // Class table
            std::string offset = symbolTables[currentSymbolTable-1].GetSymbolOffset(assignedTo);
            Symbol::symbolKind k;
            k = symbolTables[currentSymbolTable-1].GetSymbolKind(assignedTo);
            if (k == 0)
                WriteCode("push static " + offset);
            else if (k == 1)
                WriteCode("push this " + offset);
            else if (k == 2)
                WriteCode("push argument " + offset);
            else if (k == 3)
                WriteCode("pop local " + offset);
        }

        // Semantic Check - Array index expression must evaluate to 'int'
        declaration d2;
        d2.filename = vmFiles[index].filename;
        d2.lineNum = t.lineNum;
        Expression();
        d2.arguments = expression;
        arrayIndices.push_back(d2);
        expression.clear();

        t = l.GetNextToken();
        if (t.lexeme == "]")
            WriteCode("add");
        else
            Error(t, "Expected a ']'.");
    }

    t = l.GetNextToken();
    if (t.lexeme == "=")
        ;
    else
        Error(t, "Expected a '='.");

    Expression();
    // Store the RHS expressions and add them to the list to resolve at the end
    d.arguments = expression;
    assignments.push_back(d);
    expression.clear();

    // If it is assigning to an ArrayEntry write code for the array access
    if (isArrayEntry) {
        WriteCode("pop temp 0");
        WriteCode("pop pointer 1");
        WriteCode("push temp 0");
        WriteCode("pop that 0");
    }

    t = l.GetNextToken();
    if (t.lexeme == ";")
        ;
    else
        Error(t, "Expected a ';'.");

    // ArrayEntry code already generated above, if it is not an ArrayEntry generate code
    if (!isArrayEntry) {
        if (symbolTables[currentSymbolTable].FindSymbol(assignedTo)) { // Method table
            std::string offset = symbolTables[currentSymbolTable].GetSymbolOffset(assignedTo);
            Symbol::symbolKind k;
            k = symbolTables[currentSymbolTable].GetSymbolKind(assignedTo);
            if (k == 0)
                WriteCode("pop static " + offset);
            else if (k == 1)
                WriteCode("pop this " + offset);
            else if (k == 2)
                WriteCode("pop argument " + offset);
            else if (k == 3)
                WriteCode("pop local " + offset);
        }
        else if (symbolTables[currentSymbolTable-1].FindSymbol(assignedTo)) { // Class table
            std::string offset = symbolTables[currentSymbolTable-1].GetSymbolOffset(assignedTo);
            Symbol::symbolKind k;
            k = symbolTables[currentSymbolTable-1].GetSymbolKind(assignedTo);
            if (k == 0)
                WriteCode("pop static " + offset);
            else if (k == 1)
                WriteCode("pop this " + offset);
            else if (k == 2)
                WriteCode("pop argument " + offset);
            else if (k == 3)
                WriteCode("pop local " + offset);
        }
    }
}


void Parser::IfStatement() {
    // Code Generation - labels
    std::string l1, l2;
    Token t = l.GetNextToken();
    if (t.lexeme == "if")
        ;
    else
        Error(t, "Expected keyword 'if'.");

    t = l.GetNextToken();
    if (t.lexeme == "(")
        ;
    else
        Error(t, "Expected a '('.");

    Expression();

    t = l.GetNextToken();
    if (t.lexeme == ")") {
        l1 = CreateLabel();
        WriteCode("not");
        WriteCode("if-goto " + l1);
    }
    else
        Error(t, "Expected a ')'.");

    t = l.GetNextToken();
    if (t.lexeme == "{")
        ;
    else
        Error(t, "Expected a '{'.");

    t = l.PeekNextToken();
    while (t.lexeme != "}") {
        if (t.lexeme == "return")
            foundIfReturn = true;
        Statement();
        t = l.PeekNextToken();
    }
    l.GetNextToken();       // Consume the '}'

    // Code Generation
    l2 = CreateLabel();
    WriteCode("goto " + l2);
    WriteCode("label " + l1);

    t = l.PeekNextToken();
    if (t.lexeme == "else") {
        l.GetNextToken();    // Consume the 'else'

        t = l.GetNextToken();
        if (t.lexeme == "{")
            ;
        else
            Error(t, "Expected a '{'.");

        t = l.PeekNextToken();
        while (t.lexeme != "}") {
            if (t.lexeme == "return")
                foundElseReturn = true;
            Statement();
            t = l.PeekNextToken();
        }
        l.GetNextToken();       // Consume the '}'
    }
    // Code Generation
    WriteCode("label " + l2);
}


void Parser::WhileStatement() {
    // Code Generation - labels
    std::string l1, l2;

    Token t = l.GetNextToken();
    if (t.lexeme == "while") {
        l1 = CreateLabel();
        WriteCode("label " + l1);
    }
    else
        Error(t, "Expected keyword 'while'.");

    t = l.GetNextToken();
    if (t.lexeme == "(")
        ;
    else
        Error(t, "Expected a '('.");

    Expression();

    t = l.GetNextToken();
    if (t.lexeme == ")") {
        // Code Generation - Check loop
        l2 = CreateLabel();
        WriteCode("not");
        WriteCode("if-goto " + l2);
    }
    else
        Error(t, "Expected a ')'.");

    t = l.GetNextToken();
    if (t.lexeme == "{")
        ;
    else
        Error(t, "Expected a '{'.");

    t = l.PeekNextToken();
    while (t.lexeme != "}") {
        Statement();
        t = l.PeekNextToken();
    }
    l.GetNextToken();       // Consume the '}'

    // Code Generation
    WriteCode("goto " + l1);
    WriteCode("label " + l2);
}


void Parser::DoStatement() {
    Token t = l.GetNextToken();
    if (t.lexeme == "do")
        ;
    else
        Error(t, "Expected keyword 'do'.");

    SubroutineCall();
}


void Parser::SubroutineCall() {
    Token t = l.GetNextToken();
    // Semantic check - subroutine calls
    unsigned int index = vmFiles.size() - 1;
    declaration d;
    d.filename = vmFiles[index].filename;
    d.lineNum = t.lineNum;
    std::string identifier1, identifier2;

    if (t.type == t.identifier) {
        identifier1 = t.lexeme;
        d.name = t.lexeme;

        // Code Generation
        if (symbolTables[currentSymbolTable].FindSymbol(t.lexeme)) { // method table
            std::string offset = symbolTables[currentSymbolTable].GetSymbolOffset(t.lexeme);
            Symbol::symbolKind k;
            k = symbolTables[currentSymbolTable].GetSymbolKind(t.lexeme);
            if (k == 0) // static variables
                WriteCode("push static " + offset);
            else if (k == 1) // field variables
                WriteCode("push this " + offset);
            else if (k == 2) // argument variables
                WriteCode("push argument " + offset);
            else if (k == 3) // local variables
                WriteCode("push local " + offset);
        }
        else if (symbolTables[currentSymbolTable-1].FindSymbol(t.lexeme)) { // Class table
            std::string offset = symbolTables[currentSymbolTable-1].GetSymbolOffset(t.lexeme);
            Symbol::symbolKind k;
            k = symbolTables[currentSymbolTable-1].GetSymbolKind(t.lexeme);
            if (k == 0) // static variables
                WriteCode("push static " + offset);
            else if (k == 1) // field variables
                WriteCode("push this " + offset);
            else if (k == 2) // argument variables
                WriteCode("push argument " + offset);
            else if (k == 3) // local variables
                WriteCode("push local " + offset);
        }
    }
    else
        Error(t, "Expected an identifier.");

    t = l.PeekNextToken();
    if (t.lexeme == ".") {
        l.GetNextToken();       // Consume the '.'

        t = l.GetNextToken();
        if (t.type == t.identifier) {
            identifier2 = t.lexeme;
            d.name = t.lexeme;
        }
        else
            Error(t, "Expected an identifier.");
    }

    t = l.GetNextToken();
    if (t.lexeme == "(")
        ;
    else
        Error(t, "Expected a '('.");

    // Add it to the list for resolving at the end
    subroutineCalls.push_back(d);
    ExpressionList();

    t = l.GetNextToken();
    if (t.lexeme == ")")
        ;
    else
        Error(t, "Expected a ')'.");

    t = l.GetNextToken();
    if (t.lexeme == ";")
        ;
    else
        Error(t, "Expected a ';'.");

    // Code Generation
    std::string numOfArgs = GetNumOfArgs(identifier1, identifier2);
    int methodArgs = std::stoi(numOfArgs) + 1;
    std::string methodNumOfArgs = std::to_string(methodArgs);

    // Find the class the symbol belongs to and get it
    std::string type;
    if (symbolTables[currentSymbolTable].FindSymbol(identifier1))
        type = symbolTables[currentSymbolTable].GetSymbolType(identifier1);
    else if (symbolTables[currentSymbolTable-1].FindSymbol(identifier1))
        type = symbolTables[currentSymbolTable-1].GetSymbolType(identifier1);

    if (identifier2.empty()) {
        WriteCode("push pointer 0");
        WriteCode("call " + currentClass + "." + identifier1 + " " + methodNumOfArgs);
        WriteCode(identifier1); // For the RemovePopCode() function
    }
    else if (type.empty()) {
        WriteCode("call " + identifier1 + "." + identifier2 + " " + numOfArgs);
        WriteCode(identifier2);
    }
    else {
        WriteCode("call " + type + "." + identifier2 + " " + methodNumOfArgs);
        WriteCode(identifier2);
    }
    /* If the called function was void then we get rid of the '0' left on top of the
     * stack. At the end of parsing if the function wasnt void this pop is removed */
    WriteCode("pop temp 0");
}


void Parser::ExpressionList() {
    Token t = l.PeekNextToken();
    if (t.lexeme == ")")
        ;
    else {
        // Semantic check - calls must have same number and type of arguments
        unsigned long methodIndex = subroutineCalls.size() - 1;
        arguments.clear();
        Expression();
        subroutineCalls[methodIndex].arguments = arguments;

        Token t = l.PeekNextToken();
        while (t.lexeme == ",") {
            l.GetNextToken();       // Consume the ','

            Expression();
            // If there are more arguments
            subroutineCalls[methodIndex].arguments = arguments;

            t = l.PeekNextToken();
        }
    }
}


void Parser::ReturnStatement() {
    Token t = l.GetNextToken();
    if (t.lexeme == "return")
        ;
    else
        Error(t, "Expected keyword 'return',");

    // Semantic check - return value must be compatible with subroutine type
    unsigned int index = vmFiles.size() - 1;
    declaration d;
    d.filename = vmFiles[index].filename;
    d.lineNum = t.lineNum;
    d.name = currentSubroutine; // Store the subroutine name to which the return belongs
    d.type = currentSubroutineType;
    expression.clear();


    bool thereIsExpression = false; // Flag for void returns
    t = l.PeekNextToken();
    if (t.lexeme != ";") {
        Expression();
        thereIsExpression = true;
        d.arguments = expression;
    }
    returns.push_back(d);

    t = l.GetNextToken();
    if (t.lexeme == ";")
        ;
    else
        Error(t, "Expected a ';'.");

    // Semantic check - Unreachable code
    t = l.PeekNextToken();
    if (t.lexeme != "}")
        Error(t, "Unreachable code.");

    // Code Generation
    if (currentSubroutineType == "void" && !thereIsExpression)
        WriteCode("push constant 0");
    WriteCode("return");
}


void Parser::Expression() {
    RelationalExpression();

    Token t = l.PeekNextToken();
    while (t.lexeme == "&" || t.lexeme == "|") {
        t = l.GetNextToken();    // Consume the '&' or '|'
        RelationalExpression();

        // Code Generation
        if (t.lexeme == "&")
            WriteCode("and");
        else
            WriteCode("or");

        t = l.PeekNextToken();
    }
}


void Parser::RelationalExpression() {
    ArithmeticExpression();

    Token t = l.PeekNextToken();
    while (t.lexeme == "=" || t.lexeme == ">" || t.lexeme == "<") {
        // Store expressions for semantic checks
        expression.push_back(t.lexeme);
        arguments.push_back(t.lexeme);

        t = l.GetNextToken();    // Consume the '=' or '>' or '<'
        ArithmeticExpression();

        // Code Generation
        if (t.lexeme == "=")
            WriteCode("eq");
        else if (t.lexeme == ">")
            WriteCode("gt");
        else
            WriteCode("lt");

        t = l.PeekNextToken();
    }
}


void Parser::ArithmeticExpression() {
    Term();

    Token t = l.PeekNextToken();
    while (t.lexeme == "+" || t.lexeme == "-") {
        // Store expressions for semantic checks
        expression.push_back(t.lexeme);
        arguments.push_back(t.lexeme);

        t = l.GetNextToken();    // Consume the '+' or '-'
        Term();

        // Code Generation
        if (t.lexeme == "+")
            WriteCode("add");
        else
            WriteCode("sub");

        t = l.PeekNextToken();
    }
}


void Parser::Term() {
    Factor();

    Token t = l.PeekNextToken();
    while (t.lexeme == "*" || t.lexeme == "/") {
        // Store Expressions for semantic checks
        expression.push_back(t.lexeme);
        arguments.push_back(t.lexeme);

        t = l.GetNextToken();    // Consume the '*' or '/'
        Factor();

        // Code Generation
        if (t.lexeme == "*")
            WriteCode("call Math.multiply 2");
        else
            WriteCode("call Math.divide 2");

        t = l.PeekNextToken();
    }
}


void Parser::Factor() {
    Token t = l.PeekNextToken();
    if (t.lexeme == "-" || t.lexeme == "~") {
        t = l.GetNextToken();    // Consume the '-' or '~'
        Operand();

        // Code Generation
        if (t.lexeme == "-")
            WriteCode("neg");
        else
            WriteCode("not");
    }
    else
        Operand();
}


void Parser::Operand() {
    Token t = l.GetNextToken();
    if (t.type == t.constant) {
        // Store Expressions for semantic checks
        expression.push_back("int");
        arguments.push_back("int");

        // Code Generation
        WriteCode("push constant " + t.lexeme);
    }
    else if (t.type == t.identifier) {
        std::string copy = t.lexeme;

        // If-else statements for Semantics Check
        Token t2 = l.PeekNextToken();
        if (t2.lexeme != ".") {
            // Semantic Check - Variable declaration
            if (!(symbolTables[currentSymbolTable].FindSymbol(t.lexeme)) &&
                !(symbolTables[currentSymbolTable-1].FindSymbol(t.lexeme))) {
                Error(t, "Variable must be declared before being used.");
            }

            // Semantic Check - store types to evaluate expressions
            std::string type;
            if (symbolTables[currentSymbolTable].FindSymbol(t.lexeme)) { // Method table
                 type = symbolTables[currentSymbolTable].GetSymbolType(t.lexeme);
                 expression.push_back(type);
                 arguments.push_back(type);
            }
            else if (symbolTables[currentSymbolTable-1].FindSymbol(t.lexeme)) { // Class table
                type = symbolTables[currentSymbolTable - 1].GetSymbolType(t.lexeme);
                expression.push_back(type);
                arguments.push_back(type);
            }
        }

        // Code Generation
        if (symbolTables[currentSymbolTable].FindSymbol(t.lexeme)) { // Method table
            std::string offset = symbolTables[currentSymbolTable].GetSymbolOffset(t.lexeme);
            Symbol::symbolKind k;
            k = symbolTables[currentSymbolTable].GetSymbolKind(t.lexeme);
            if (k == 0) // static variables
                WriteCode("push static " + offset);
            else if (k == 1) // field variables
                WriteCode("push this " + offset);
            else if (k == 2) // argument variables
                WriteCode("push argument " + offset);
            else if (k == 3) // local variables
                WriteCode("push local " + offset);
        }
        else if (symbolTables[currentSymbolTable-1].FindSymbol(t.lexeme)) { // Class table
            std::string offset = symbolTables[currentSymbolTable-1].GetSymbolOffset(t.lexeme);
            Symbol::symbolKind k;
            k = symbolTables[currentSymbolTable-1].GetSymbolKind(t.lexeme);
            if (k == 0) // static variables
                WriteCode("push static " + offset);
            else if (k == 1) // field variables
                WriteCode("push this " + offset);
            else if (k == 2) // argument variables
                WriteCode("push argument " + offset);
            else if (k == 3) // local variables
                WriteCode("push local " + offset);
        }

        // Semantic Check - Variable initialisation
        if (symbolTables[currentSymbolTable].FindSymbol(t.lexeme)) { // Method table
            if (!symbolTables[currentSymbolTable].IsInitialised(t.lexeme))
                Warning(t, "Variable not initialised before being used.");
        }
        else if (symbolTables[currentSymbolTable-1].FindSymbol(t.lexeme)) { // Class table
            if (!symbolTables[currentSymbolTable-1].IsInitialised(t.lexeme))
                Warning(t, "Variable not initialised before being used.");
        }

        std::string identifier2;
        t = l.PeekNextToken();
        if (t.lexeme == ".") {
            l.GetNextToken();    // Consume the '.'

            t = l.GetNextToken();
            if (t.type == t.identifier) {
                identifier2 = t.lexeme;

                // Semantic check - resolve subroutine calls
                unsigned int index = vmFiles.size() - 1;
                declaration d;
                d.lineNum = t.lineNum;
                d.filename = vmFiles[index].filename;
                if (t.lexeme == "new") {
                    d.type = copy; // Store type before the '.' if its a constructor
                    d.name = t.lexeme;
                    subroutineCalls.push_back(d);

                    // Semantic check - store expressions for evaluation at the end
                    expression.push_back(copy);
                    arguments.push_back(copy);
                }
                else {
                    d.name = t.lexeme;
                    subroutineCalls.push_back(d);

                    // Semantic check - store expressions for evaluation at the end
                    expression.push_back(t.lexeme);
                    arguments.push_back(t.lexeme);
                }
            }
            else
                Error(t, "Expected an identifier.");
        }

        t = l.PeekNextToken();
        if (t.lexeme == "[") {
            l.GetNextToken();    // Consume the '['

            // Turned out to be an ArrayEntry so delete last stored
            expression.erase(expression.end()-1);
            arguments.erase(arguments.end()-1);
            expression.push_back("ArrayEntry");
            arguments.push_back("ArrayEntry");

            Expression();

            t = l.GetNextToken();
            if (t.lexeme == "]") {
                // Code Generation - Array access
                WriteCode("add");
                WriteCode("pop pointer 1");
                WriteCode("push that 0");
            }
            else
                Error(t, "Expected a ']'.");
        }
        else if (t.lexeme == "(") {
            l.GetNextToken();    // Consume the '('

            unsigned long resize = expression.size();
            ExpressionList();
            // After the 'ExpressionList' call the expressions might clash
            // So we use resize to the original storage to keep the first expression
            expression.resize(resize);

            t = l.GetNextToken();
            if (t.lexeme == ")")
                ;
            else
                Error(t, "Expected a ')'.");

            // Code Generation
            std::string numOfArgs = GetNumOfArgs(copy, identifier2);
            int methodArgs = std::stoi(numOfArgs) + 1;
            std::string methodNumOfArgs = std::to_string(methodArgs);

            // Find the class the symbol belongs to and get it
            std::string type;
            if (symbolTables[currentSymbolTable].FindSymbol(copy))
                type = symbolTables[currentSymbolTable].GetSymbolType(copy);
            else if (symbolTables[currentSymbolTable-1].FindSymbol(copy))
                type = symbolTables[currentSymbolTable-1].GetSymbolType(copy);

            if (identifier2.empty()) {
                WriteCode("push pointer 0");
                WriteCode("call " + currentClass + "." + copy + " " + methodNumOfArgs);
            }
            else if (type.empty())
                WriteCode("call " + copy + "." + identifier2 + " " + numOfArgs);
            else
                WriteCode("call " + type + "." + identifier2 + " " + methodNumOfArgs);
        }
    }
    else if (t.lexeme == "(") {
        Expression();

        t = l.GetNextToken();
        if (t.lexeme == ")")
            ;
        else
            Error(t, "Expected a ')'.");
    }
    else if (t.type == t.string_literal) {
        // Store Expressions for semantic checks
        expression.push_back("String");
        arguments.push_back("String");

        // Code Generation
        WriteCode("push constant " + std::to_string(t.lexeme.length()));
        WriteCode("call String.new 1");
        for (char &c: t.lexeme) {
            WriteCode("push constant " + std::to_string(int(c)));
            WriteCode("call String.appendChar 2");
        }
    }
    else if (t.lexeme == "true") {
        expression.push_back("boolean");
        arguments.push_back("boolean");
        WriteCode("push constant 1");
        WriteCode("neg");
    }
    else if (t.lexeme == "false") {
        expression.push_back("boolean");
        arguments.push_back("boolean");
        WriteCode("push constant 0");
    }
    else if (t.lexeme == "null") {
        expression.push_back("null");
        arguments.push_back("null");
        WriteCode("push constant 0");
    }
    else if (t.lexeme == "this") {
        expression.push_back(currentClass);
        arguments.push_back(currentClass);
        WriteCode("push pointer 0");
    }
    else
        Error(t, "Unknown constant or variable.");
}
