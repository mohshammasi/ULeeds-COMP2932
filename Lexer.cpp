#include <iostream>
#include <ctype.h>
#include "CompilerHeaders.h"


/************** Token class implementation **************/

Token::Token() {
    lexeme = "";
}

/************** Lexer class implementation **************/
// Lexer globals
std::string keywordsArray[] = {"class", "constructor", "method", "function",
                               "int", "boolean", "char", "void", "var", "static",
                               "field", "let", "do", "if", "else", "while",
                               "return", "true", "false", "null", "this"};
char symbolsArray[] = {'(', ')', '[', ']', '{', '}', ',', ';', '=', '.', '+', '-',
                       '*', '/', '&', '|', '~', '<', '>'};


bool Lexer::ExtractSourceFile(std::string sourceFile) {
    char c = 0;
    inputStream.open(sourceFile.c_str());
    if (inputStream.is_open()) {
        // Copy the entire file into the vector
        c = (char) inputStream.get();
        while (c != EOF) {
            charsVector.push_back(c);
            c = (char) inputStream.get();
        }
        charsVector.push_back(c); // Add EOF
        inputStream.close();
        lineNum = 1;
        return true;
    }
    else {
        std::cout << "Unable to open file " + sourceFile <<std::endl;
        return false;
    }
}


bool Lexer::ProduceTokens() {
    unsigned int i = 0;
    while (i < charsVector.size()) {
        Token token;

        // Consume whitespace
        while ((charsVector[i] != EOF) && isspace((char)charsVector[i])) {
            if (charsVector[i] == '\n')
                lineNum++;
            i++;
        }


        // Ignore Comments of type (//, /* */, and /** */)
        // First check if its a single line comment
        if ((charsVector[i] == '/') && (charsVector[i+1] == '/')) {
            while ((charsVector[i] != '\n'))
                i++;
            continue; // To go back to check for whitespace
        }
        // Multi line comments (/** */ and /* */)
        else if ((charsVector[i] == '/') && (charsVector[i+1] == '*')) {
            int commentLineNum = lineNum;
            /* The loop runs as long as the the previous char and current char are
            not '*' and '/' respectively. Evaluates it once with an '||' and once
            with an '&&' so it doesnt stop when hitting only 1 of the '/' or '*' */
            while (( (charsVector[i] != '/') || (charsVector[i-1] != '*') ) ||
            ( (charsVector[i] != '/') && (charsVector[i-1] != '*') )) {
                if (charsVector[i] == EOF) {
                    std::cout << "Error: line " << commentLineNum << ", unexpected "
                              << "EOF character. Multi-line comment missing closing "
                              << "'*/'." <<std::endl;
                    return false;
                }
                else if (charsVector[i] == '\n')
                    lineNum++;
                i++;
            }
            i++; // The loop stops at '/' so move to the next char.
            continue; // To go back to check for whitespace
        }


        // If we hit EOF
        if (charsVector[i] == EOF ) {
            token.lineNum = lineNum;
            token.type = Token::eof;
            tokensVector.push_back(token);
            charsVector.clear();
            return true;
        }


        // If its a String literal
        if (charsVector[i] == '"') {
            i++;
            while (charsVector[i] != '"') {
                if (charsVector[i] == EOF) {
                    std::cout << "Error: line " << lineNum << ", unexpected EOF "
                              << "character. String literal missing closing '\"'."
                              <<std::endl;
                    return false;
                }
                else if (charsVector[i] == '\n') {
                    std::cout << "Error: line " << lineNum << ", newline "
                              << "character in string literal." <<std::endl;
                    return false;
                }
                token.lexeme += (char)charsVector[i];
                i++;
            }
            i++; // Stopped at the closing '"' so move to next char.
            token.lineNum = lineNum;
            token.type = Token::string_literal;
            tokensVector.push_back(token);
            continue;
        }


        // If its a Keyword or an Identifier
        if (isalpha((char)charsVector[i]) || charsVector[i] == '_') {
            while ((charsVector[i] != EOF) &&
            ((isalpha((char)charsVector[i])) || (isdigit((char)charsVector[i])))) {
                token.lexeme += (char)charsVector[i];
                i++;
            }

            // Find out if its a keyword or an identifier
            bool isKeyword = false;
            for (int j=0; j < NUM_JACK_KEYWORDS; j++) {
                if (token.lexeme == keywordsArray[j])
                    isKeyword = true;
            }

            token.lineNum = lineNum;
            if (isKeyword) {
                token.type = Token::keyword;
                tokensVector.push_back(token);
                continue;
            }
            else {
                token.type = Token::identifier;
                tokensVector.push_back(token);
                continue;
            }
        }


        // If its a number
        if (isdigit((char)charsVector[i])) {
            while (((char)charsVector[i] != EOF) && (isdigit((char)charsVector[i]))) {
                token.lexeme += (char)charsVector[i];
                i++;
            }
            token.lineNum = lineNum;
            token.type = Token::constant;
            tokensVector.push_back(token);
            continue;
        }


        // if its a symbol allowed in JACK
        bool isValidSymbol = false;
        for (int j=0; j < NUM_JACK_SYMBOLS; j++) {
            if (charsVector[i] == symbolsArray[j])
                isValidSymbol = true;
        }

        if (isValidSymbol) {
            token.lexeme += (char)charsVector[i];
            i++;
            token.lineNum = lineNum;
            token.type = Token::symbol;
            tokensVector.push_back(token);
            continue;
        }


        // None of the above such as an invalid symbol of JACK e.g. '?' or '!'
        /* 'continue' statements don't allow the loop to reach this point unless
        all possibilities are checked and the current char doesnt satisfy any of
        the above. */
        std::cout << "Error, line " << lineNum << ", invalid symbol '"
                  << charsVector[i] << "'." <<std::endl;
        return false;
    }
    return false; // Didnt reach EOF, lexer failed.
}


Token Lexer::GetNextToken() {
    if (tokensVector.empty()) {
      Token eofToken;
      eofToken.type = Token::eof;
      return eofToken;
    }
    else {
      Token currentToken = tokensVector[0];
      tokensVector.erase(tokensVector.begin());
      return currentToken;
    }
}


Token Lexer::PeekNextToken() {
    if (tokensVector.empty()) {
      Token eofToken;
      eofToken.type = Token::eof;
      return eofToken;
    }
    else
      return tokensVector[0];
}


// Only used when the parsing of the file is finished and EOF is left in the vector.
void Lexer::DeleteEOF() {
    tokensVector.erase(tokensVector.begin());
}
