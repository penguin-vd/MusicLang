#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Ast.hpp"
#include "Lexer.hpp"

using PrefixParseFn = std::function<std::shared_ptr<Expression>()>;
using InfixParseFn =
    std::function<std::shared_ptr<Expression>(std::shared_ptr<Expression>)>;

enum class Precedence {
    LOWEST,
    EQUALS,       // ==
    LESSGREATER,  // > or <
    SUM,          // +
    PRODUCT,      // *
    PREFIX,       // -X or !X
    CALL,         // myFunction(X)
    INDEX         // list[X]
};

class Parser {
   private:
    Lexer lexer;
    Token curToken;
    Token peekToken;
    std::unordered_map<TokenType, PrefixParseFn> prefixParseFns;
    std::unordered_map<TokenType, InfixParseFn> infixParseFns;
    std::unordered_map<TokenType, Precedence> precedences = std::unordered_map<TokenType, Precedence>({
        {TokenType::EQ, Precedence::EQUALS}, {TokenType::NOT_EQ, Precedence::EQUALS}, {TokenType::LT, Precedence::LESSGREATER},
        {TokenType::GT, Precedence::LESSGREATER}, {TokenType::PLUS, Precedence::SUM}, {TokenType::MINUS, Precedence::SUM},
        {TokenType::SLASH, Precedence::PRODUCT}, {TokenType::ASTERISK, Precedence::PRODUCT}, {TokenType::LPAREN, Precedence::CALL},
        {TokenType::LBRACKET, Precedence::INDEX}
    });

    void NextToken();
    bool ExpectPeek(TokenType type);
    bool PeekTokenIs(TokenType type) const;
    bool IsAssignOp() const;
    bool CurTokenIs(TokenType t) const;
    void PeekError(TokenType t);
    void NoPrefixParseFnError(TokenType t);
    Precedence PeekPrecedence() const;
    Precedence CurPrecedence() const;

    std::shared_ptr<Statement> ParseStatement();
    std::shared_ptr<LetStatement> ParseLetStatement();
    std::shared_ptr<ReturnStatement> ParseReturnStatement();
    std::shared_ptr<BreakStatement> ParseBreakStatement();
    std::shared_ptr<Statement> ParseAssignStatement();
    std::shared_ptr<ExpressionStatement> ParseExpressionStatement();
    std::shared_ptr<Expression> ParseExpression(Precedence precedence);
    std::shared_ptr<Expression> ParseInfixExpression(
        std::shared_ptr<Expression> left);
    std::shared_ptr<Expression> ParsePrefixExpression();
    std::shared_ptr<Identifier> ParseIdentifier();
    std::shared_ptr<Expression> ParseIntegerLiteral();
    std::shared_ptr<Expression> ParseBoolean();
    std::shared_ptr<Expression> ParseGroupedExpression();
    std::shared_ptr<Expression> ParseIfExpression();
    std::shared_ptr<FunctionLiteral> ParseFunctionStatement();
    std::shared_ptr<Expression> ParseStringLiteral();
    std::shared_ptr<Expression> ParseArrayLiteral();
    std::shared_ptr<Expression> ParseHashLiteral();
    std::shared_ptr<Expression> ParseForExpression();
    std::shared_ptr<Expression> ParseWhileExpression();
    std::shared_ptr<Expression> ParseCallExpression(
        std::shared_ptr<Expression> function);
    std::shared_ptr<Expression> ParseIndexExpression(
        std::shared_ptr<Expression> left);
    std::shared_ptr<BlockStatement> ParseBlockStatement();
    std::shared_ptr<ForIterative> ParseIterativeExpression();
    std::shared_ptr<Statement> ParseAccessStatement();
    std::vector<std::shared_ptr<Expression>> ParseExpressionList(TokenType end);
    std::vector<std::shared_ptr<Identifier>> ParseFunctionParameters();

   public:
    Parser(Lexer l);
    std::shared_ptr<AstProgram> ParseProgram();
    std::vector<std::string> Errors;
    void RegisterPrefix(TokenType tokenType, PrefixParseFn fn);
    void RegisterInfix(TokenType tokenType, InfixParseFn fn);
};
