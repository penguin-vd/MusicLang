#pragma once

#include <map>
#include <string>

#include "Token.hpp"

class Lexer {
   public:
    Lexer(std::string input) : Input(input) { ReadChar(); }

    Token NextToken();

    std::string Input;

   private:
    void ReadChar();
    char PeekChar();
    std::string SkipLine();
    std::string ReadString();
    std::string ReadIdentifier();
    TokenType LookupIdent(std::string literal);
    Token ReadNumber();
    void SkipWhitespace();

    std::map<std::string, TokenType> keywords = {
        {"function", TokenType::FUNCTION},
        {"let", TokenType::LET},
        {"true", TokenType::TRUE},
        {"false", TokenType::FALSE},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"return", TokenType::RETURN},
        {"break", TokenType::BREAK},
        {"for", TokenType::FOR},
        {"in", TokenType::IN}};

    size_t position = 0;
    size_t readPosition = 0;
    char ch;
    size_t line = 1;
    int positionOffset = 0;
};
