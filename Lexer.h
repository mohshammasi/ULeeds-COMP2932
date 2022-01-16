#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <fstream>
#include <vector>

/************** Token class definitions **************/
class Token {
public:
    int lineNum;
    enum tokenTypes {keyword, symbol, identifier, string_literal, constant, eof};
    std::string lexeme;
    tokenTypes type;
public:
    Token();
};

/************** Lexer class definitions **************/
class Lexer {
private:
    int lineNum;
    std::ifstream inputStream;
    std::vector<char> charsVector;
    std::vector<Token> tokensVector;
public:
    bool ExtractSourceFile(std::string sourceFile);
    bool ProduceTokens();
    Token GetNextToken();
    Token PeekNextToken();
    void DeleteEOF();
};

#endif
