#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include "CompilerHeaders.h"
#include "SymbolTable.h"

/****************** Parser class definitions *****************/
class Parser {
private:
    Lexer l;
    std::vector <SymbolTable> symbolTables;

    // Variables used to keep track of where we are while parsing
    int currentSymbolTable;
    std::string currentClass;
    std::string currentSubroutine;
    std::string currentSubroutineType;
    std::string currentSubroutineKind;

    typedef struct {
        std::string filename;
        std::string type;
        std::string name;
        int lineNum;
        std::string LHS;
        std::string RHS;
        bool resolved = false;
        std::vector <std::string> arguments; // for subroutines
        bool argsMatch = false; // for subroutines
    } declaration;
    std::vector <declaration> varDeclarations; // for resolving variables from other classes
    std::vector <declaration> subroutineCalls; // for resolving subroutines
    std::vector <declaration> assignments; // for evaluating LHS, RHS compatibility
    std::vector <declaration> returns; // for evaluating subroutine return expressions
    std::vector <declaration> arrayIndices; // for evaluating array indices expressions
    bool foundIfReturn;
    bool foundElseReturn; // Used for all code paths check
    /* Used to temporarily store expressions as they are parsed until the expression is
     * complete, then its placed in one of the declarations where it belongs. There are
     * 2 containers because subroutines may call expressionlist and then expressions would clash */
    std::vector <std::string> expression;
    std::vector <std::string> arguments;

    // For creating labels for code generation
    int labelCounter = 0;


public:
    Parser();
    bool Init(std::string filename);
    void ClassDeclar();

    // Used for Semantics checking
    void AddJackOS();
    void ResolveAllDeclars();

    // Output vm files
    typedef struct {
        std::string filename;
        std::vector <std::string> vmCode;
    } VmFile;
    std::vector <VmFile> vmFiles;
    void WriteVmFiles(std::string path);

// Encapsulate these as they should never be called randomly
private:
    // Used for Semantics checking
    void ResolveVarDeclar(std::string type);
    void ResolveSubroutineCall(Symbol s);
    void ResolveSubroutinesReturnType(std::vector<declaration> &v, Symbol s);
    void EvaluateExpressions(std::vector<declaration> &v);
    bool CheckCompatibility(std::string type1, std::string type2);
    void CheckReturnsCompatibility();
    void CheckArrayIndices();

    // Error and Warnings reporting
    void Error(Token t, std::string message);
    void Warning(Token t, std::string message);
    void ResolveError(declaration d, std::string message);
    void ResolveWarning(declaration d, std::string message);

    // Used for Code Generation
    void WriteCode(std::string vmCode);
    void RemovePopCode(Symbol s);
    std::string CreateLabel();
    std::string GetNumOfArgs(std::string name, std::string type);

    // Productions functions for the parser
    void MemberDeclar();
    void ClassVarDeclar();
    void Type();
    void SubroutineDeclar();
    void ParamList();
    void SubroutineBody();
    void Statement();
    void VarDeclarStatement();
    void LetStatement();
    void IfStatement();
    void WhileStatement();
    void DoStatement();
    void SubroutineCall();
    void ExpressionList();
    void ReturnStatement();
    void Expression();
    void RelationalExpression();
    void ArithmeticExpression();
    void Term();
    void Factor();
    void Operand();
};


#endif
