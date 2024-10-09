#pragma once

#include <string>

enum class TokenType {
    ILLEGAL,
    TOKEN_EOF,
    COMMENT,

    // Identifiers + literals
    IDENT,
    INT,
    FLOAT,
    STRING,
    ACCESS,

    // Operators
    ASSIGN,
    PLUS,
    MINUS,
    PLUS_EQ,
    MIN_EQ,
    TIMES_EQ,
    DIVIDE_EQ,

    BANG,
    ASTERISK,
    SLASH,

    LT,
    GT,
    EQ,
    NOT_EQ,

    // Delimiters
    COMMA,
    SEMICOLON,
    COLON,

    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,

    // Keywords
    FUNCTION,
    LET,
    TRUE,
    FALSE,
    IF,
    ELSE,
    RETURN,
    FOR,
    BREAK,
    IN
};

std::string TokenTypeToString(TokenType t);

struct Token {
    TokenType Type;
    std::string Literal;
    size_t LineNumber;
    int Position;

    Token() {}
    Token(TokenType type, std::string literal, size_t lineNumber, int position)
        : Type(type),
          Literal(literal),
          LineNumber(lineNumber),
          Position(position) {}
};
