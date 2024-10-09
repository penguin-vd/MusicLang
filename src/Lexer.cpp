#include "Lexer.hpp"

#include <cctype>
#include <cstdio>

#include "Token.hpp"

std::string TokenTypeToString(TokenType t) {
    switch (t) {
        case TokenType::ILLEGAL:
            return "ILLEGAL";
        case TokenType::TOKEN_EOF:
            return "TOKEN_EOF";
        case TokenType::COMMENT:
            return "COMMENT";
        case TokenType::IDENT:
            return "IDENT";
        case TokenType::INT:
            return "INT";
        case TokenType::FLOAT:
            return "FLOAT";
        case TokenType::STRING:
            return "STRING";
        case TokenType::ASSIGN:
            return "ASSIGN";
        case TokenType::PLUS:
            return "PLUS";
        case TokenType::MINUS:
            return "MINUS";
        case TokenType::PLUS_EQ:
            return "PLUS_EQ";
        case TokenType::MIN_EQ:
            return "MIN_EQ";
        case TokenType::TIMES_EQ:
            return "TIMES_EQ";
        case TokenType::DIVIDE_EQ:
            return "DIVIDE_EQ";
        case TokenType::BANG:
            return "BANG";
        case TokenType::ASTERISK:
            return "ASTERISK";
        case TokenType::SLASH:
            return "SLASH";
        case TokenType::LT:
            return "LT";
        case TokenType::GT:
            return "GT";
        case TokenType::EQ:
            return "EQ";
        case TokenType::NOT_EQ:
            return "NOT_EQ";
        case TokenType::COMMA:
            return "COMMA";
        case TokenType::SEMICOLON:
            return "SEMICOLON";
        case TokenType::COLON:
            return "COLON";
        case TokenType::LPAREN:
            return "LPAREN";
        case TokenType::RPAREN:
            return "RPAREN";
        case TokenType::LBRACE:
            return "LBRACE";
        case TokenType::RBRACE:
            return "RBRACE";
        case TokenType::LBRACKET:
            return "LBRACKET";
        case TokenType::RBRACKET:
            return "RBRACKET";
        case TokenType::FUNCTION:
            return "FUNCTION";
        case TokenType::LET:
            return "LET";
        case TokenType::TRUE:
            return "TRUE";
        case TokenType::FALSE:
            return "FALSE";
        case TokenType::IF:
            return "IF";
        case TokenType::ELSE:
            return "ELSE";
        case TokenType::RETURN:
            return "RETURN";
        case TokenType::FOR:
            return "FOR";
        case TokenType::IN:
            return "IN";
        case TokenType::ACCESS:
            return "ACCESS";
        default:
            return "UNKNOWN";
    }
}

void Lexer::ReadChar() {
    if (readPosition >= Input.length()) {
        ch = EOF;
    } else {
        ch = Input[readPosition];
    }
    position = readPosition;
    readPosition++;
}

Token Lexer::NextToken() {
    Token tok;
    SkipWhitespace();

    switch (ch) {
        case '=':
            if (PeekChar() == '=') {
                ReadChar();
                tok =
                    Token(TokenType::EQ, "==", line, position - positionOffset);
                break;
            }
            tok =
                Token(TokenType::ASSIGN, "=", line, position - positionOffset);
            break;
        case ';':
            tok = Token(TokenType::SEMICOLON, ";", line,
                        position - positionOffset);
            break;
        case '(':
            tok =
                Token(TokenType::LPAREN, "(", line, position - positionOffset);
            break;
        case ')':
            tok =
                Token(TokenType::RPAREN, ")", line, position - positionOffset);
            break;
        case '{':
            tok =
                Token(TokenType::LBRACE, "{", line, position - positionOffset);
            break;
        case '}':
            tok =
                Token(TokenType::RBRACE, "}", line, position - positionOffset);
            break;
        case ',':
            tok = Token(TokenType::COMMA, ",", line, position - positionOffset);
            break;
        case '+':
            if (PeekChar() == '=') {
                ReadChar();
                tok = Token(TokenType::PLUS_EQ, "+=", line,
                            position - positionOffset);
                break;
            }
            tok = Token(TokenType::PLUS, "+", line, position - positionOffset);
            break;
        case '-':
            if (PeekChar() == '=') {
                ReadChar();
                tok = Token(TokenType::MIN_EQ, "-=", line,
                            position - positionOffset);
                break;
            } else if (PeekChar() == '>') {
                ReadChar();
                tok = Token(TokenType::ACCESS, "->", line,
                            position - positionOffset);
                break;
            }
            tok = Token(TokenType::MINUS, "-", line, position - positionOffset);
            break;
        case '!':
            if (PeekChar() == '=') {
                ReadChar();
                tok = Token(TokenType::NOT_EQ, "!=", line,
                            position - positionOffset);
                break;
            }
            tok = Token(TokenType::BANG, "!", line, position - positionOffset);
            break;
        case '*':
            if (PeekChar() == '=') {
                ReadChar();
                tok = Token(TokenType::TIMES_EQ, "*=", line,
                            position - positionOffset);
                break;
            }
            tok = Token(TokenType::ASTERISK, "*", line,
                        position - positionOffset);
            break;
        case '/':
            if (PeekChar() == '/') {
                tok = Token(TokenType::COMMENT, SkipLine(), line,
                            position - positionOffset);
                break;
            } else if (PeekChar() == '=') {
                tok = Token(TokenType::DIVIDE_EQ, "/=", line,
                            position - positionOffset);
                break;
            }
            tok = Token(TokenType::SLASH, "/", line, position - positionOffset);
            break;
        case '<':
            tok = Token(TokenType::LT, "<", line, position - positionOffset);
            break;
        case '>':
            tok = Token(TokenType::GT, ">", line, position - positionOffset);
            break;
        case '"':
            tok = Token(TokenType::STRING, ReadString(), line,
                        position - positionOffset);
            break;
        case '[':
            tok = Token(TokenType::LBRACKET, "[", line,
                        position - positionOffset);
            break;
        case ']':
            tok = Token(TokenType::RBRACKET, "]", line,
                        position - positionOffset);
            break;
        case ':':
            tok = Token(TokenType::COLON, ":", line, position - positionOffset);
            break;
        case EOF:
            tok = Token(TokenType::TOKEN_EOF, "\0", line,
                        position - positionOffset);
            break;
        default:
            if (isalpha(ch) || ch == '_') {
                std::string literal = ReadIdentifier();
                TokenType tokenType = LookupIdent(literal);
                return Token(tokenType, literal, line,
                             position - positionOffset);
            } else if (isdigit(ch)) {
                return ReadNumber();
            }

            tok = Token(TokenType::ILLEGAL, std::to_string(ch), line,
                        position - positionOffset);
            break;
    }

    ReadChar();
    return tok;
}

void Lexer::SkipWhitespace() {
    while (isspace(ch)) {
        if (ch == '\n') {
            line++;
            positionOffset = position;
        }
        ReadChar();
    }
}

char Lexer::PeekChar() {
    if (readPosition >= Input.length()) {
        return EOF;
    }
    return Input[readPosition];
}

std::string Lexer::SkipLine() {
    int oldPos = position + 1;
    while (ch != '\n' && ch != EOF) {
        ReadChar();
    }
    return Input.substr(oldPos, position - oldPos);
}

std::string Lexer::ReadString() {
    int oldPos = position + 1;
    while (true) {
        ReadChar();
        if (ch == '"' || ch == EOF) break;
    }
    return Input.substr(oldPos, position - oldPos);
}

std::string Lexer::ReadIdentifier() {
    int oldPos = position;
    while (isalnum(ch) || ch == '_') {
        ReadChar();
    }
    return Input.substr(oldPos, position - oldPos);
}

TokenType Lexer::LookupIdent(std::string literal) {
    if (keywords.find(literal) == keywords.end()) {
        return TokenType::IDENT;
    }

    return keywords[literal];
}

Token Lexer::ReadNumber() {
    int oldPos = position;
    while (isdigit(ch)) {
        ReadChar();
    }
    return Token(TokenType::INT, Input.substr(oldPos, position - oldPos), line,
                 position - positionOffset);
}
