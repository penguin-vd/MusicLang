#include "Parser.hpp"

#include <memory>

#include "Ast.hpp"
#include "Lexer.hpp"
#include "Token.hpp"

Parser::Parser(Lexer l) : lexer(l) {
    curToken = lexer.NextToken();
    peekToken = lexer.NextToken();
    Errors = std::vector<std::string>();

    RegisterPrefix(TokenType::IDENT, std::bind(&Parser::ParseIdentifier, this));
    RegisterPrefix(TokenType::INT, std::bind(&Parser::ParseIntegerLiteral, this));
    RegisterPrefix(TokenType::BANG, std::bind(&Parser::ParsePrefixExpression, this));
    RegisterPrefix(TokenType::MINUS, std::bind(&Parser::ParsePrefixExpression, this));
    RegisterPrefix(TokenType::TRUE, std::bind(&Parser::ParseBoolean, this));
    RegisterPrefix(TokenType::FALSE, std::bind(&Parser::ParseBoolean, this));
    RegisterPrefix(TokenType::LPAREN, std::bind(&Parser::ParseGroupedExpression, this));
    RegisterPrefix(TokenType::IF, std::bind(&Parser::ParseIfExpression, this));
    RegisterPrefix(TokenType::STRING, std::bind(&Parser::ParseStringLiteral, this));
    RegisterPrefix(TokenType::LBRACKET, std::bind(&Parser::ParseArrayLiteral, this));
    RegisterPrefix(TokenType::LBRACE, std::bind(&Parser::ParseHashLiteral, this));
    RegisterPrefix(TokenType::FOR, std::bind(&Parser::ParseForExpression, this));
    RegisterPrefix(TokenType::ACCESS, std::bind(&Parser::ParseAccessExpression, this));

    RegisterInfix(TokenType::EQ, std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1));
    RegisterInfix(TokenType::NOT_EQ, std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1));
    RegisterInfix(TokenType::LT, std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1));
    RegisterInfix(TokenType::GT, std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1));
    RegisterInfix(TokenType::PLUS, std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1));
    RegisterInfix(TokenType::MINUS, std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1));
    RegisterInfix(TokenType::SLASH, std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1));
    RegisterInfix(TokenType::ASTERISK, std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1));
    RegisterInfix(TokenType::LPAREN, std::bind(&Parser::ParseCallExpression, this, std::placeholders::_1));
    RegisterInfix(TokenType::LBRACKET, std::bind(&Parser::ParseIndexExpression, this, std::placeholders::_1));
}

void Parser::NextToken() {
    curToken = peekToken;
    peekToken = lexer.NextToken();
}

std::shared_ptr<AstProgram> Parser::ParseProgram() {
    auto program = std::make_shared<AstProgram>();
    while (curToken.Type != TokenType::TOKEN_EOF) {
        auto stmt = ParseStatement();
        if (stmt != nullptr) {
            program->Statements.push_back(stmt);
        }
        NextToken();
    }
    return program;
}

std::shared_ptr<Statement> Parser::ParseStatement() {
    switch (curToken.Type) {
        case TokenType::LET:
            return ParseLetStatement();
        case TokenType::RETURN:
            return ParseReturnStatement();
        case TokenType::IDENT:
            return ParseAssignStatement();
        case TokenType::BREAK:
            return ParseBreakStatement();
        case TokenType::FUNCTION:
            return ParseFunctionStatement();
        case TokenType::COMMENT:
        case TokenType::SEMICOLON:
            return nullptr;
        default:
            return ParseExpressionStatement();
    }
}

std::shared_ptr<Identifier> Parser::ParseIdentifier() {
    return std::make_shared<Identifier>(curToken, curToken.Literal);
}

std::shared_ptr<LetStatement> Parser::ParseLetStatement() {
    auto stmt = std::make_shared<LetStatement>(curToken);
    if (!ExpectPeek(TokenType::IDENT)) return nullptr;

    stmt->Name = std::make_shared<Identifier>(curToken, curToken.Literal);

    if (!ExpectPeek(TokenType::ASSIGN)) return nullptr;

    NextToken();
    stmt->Value = ParseExpression(Precedence::LOWEST);

    if (PeekTokenIs(TokenType::SEMICOLON)) NextToken();
    return stmt;
}

std::shared_ptr<ReturnStatement> Parser::ParseReturnStatement() {
    auto stmt = std::make_shared<ReturnStatement>(curToken);
    NextToken();
    stmt->Value = ParseExpression(Precedence::LOWEST);
    if (PeekTokenIs(TokenType::SEMICOLON)) NextToken();
    return stmt;
}

std::shared_ptr<Statement> Parser::ParseAssignStatement() {
    auto stmt = std::make_shared<AssignStatement>(curToken);
    if (PeekTokenIs(TokenType::LBRACKET)) {
        stmt->Name = ParseExpression(Precedence::LOWEST);
        if (IsAssignOp()) {
            auto expStmt = std::make_shared<ExpressionStatement>(curToken);
            expStmt->TheExpression = stmt->Name;
            return expStmt;
        }
    } else
        stmt->Name = std::make_shared<Identifier>(curToken, curToken.Literal);

    if (IsAssignOp()) {
        return ParseExpressionStatement();
    }
    NextToken();
    stmt->Operator = curToken.Literal;
    NextToken();
    stmt->Value = ParseExpression(Precedence::LOWEST);

    if (PeekTokenIs(TokenType::SEMICOLON)) NextToken();
    return stmt;
}

std::shared_ptr<Expression> Parser::ParseAccessExpression() {
    Token token = curToken;

    NextToken();
    if (!ExpectPeek(TokenType::IDENT)) {
        Errors.push_back("at line: " + std::to_string(curToken.LineNumber) +
                         ", wrong use of access token");
        return nullptr;
    }

    auto exp = std::make_shared<AccessExpression>(token);
    exp->Parent = std::make_shared<Identifier>(token, token.Literal);
    exp->TheStatement = ParseStatement();
    if (exp->TheStatement == nullptr) {
        return nullptr;
    }

    return exp;
}

std::shared_ptr<BreakStatement> Parser::ParseBreakStatement() {
    auto stmt = std::make_shared<BreakStatement>(curToken);
    NextToken();
    if (PeekTokenIs(TokenType::SEMICOLON)) NextToken();
    return stmt;
}

std::shared_ptr<Statement> Parser::ParseExpressionStatement() {
    auto stmt = std::make_shared<ExpressionStatement>(curToken);
    stmt->TheExpression = ParseExpression(Precedence::LOWEST);
    if (PeekTokenIs(TokenType::SEMICOLON)) NextToken();
    return stmt;
}

std::shared_ptr<Expression> Parser::ParseExpression(Precedence precedence) {
    std::function<std::shared_ptr<Expression> ()> prefix;
    if (PeekTokenIs(TokenType::ACCESS)) {
        prefix = prefixParseFns[TokenType::ACCESS];
    } else {
        prefix = prefixParseFns[curToken.Type];
    }

    if (!prefix) {
        NoPrefixParseFnError(curToken.Type);
        return nullptr;
    }

    std::shared_ptr<Expression> leftExp = prefix();

    while (!PeekTokenIs(TokenType::SEMICOLON) && precedence < PeekPrecedence()) {
        auto infix = infixParseFns[peekToken.Type];
        if (!infix) return leftExp;

        NextToken();
        leftExp = infix(leftExp);
    }
    return leftExp;
}

std::shared_ptr<Expression> Parser::ParseInfixExpression(
    std::shared_ptr<Expression> left) {
    auto expr =
        std::make_shared<InfixExpression>(curToken, curToken.Literal, left);
    auto precedence = CurPrecedence();
    NextToken();
    expr->Right = ParseExpression(precedence);
    return expr;
}

std::shared_ptr<Expression> Parser::ParsePrefixExpression() {
    auto expr = std::make_shared<PrefixExpression>(curToken, curToken.Literal);
    NextToken();
    expr->Right = ParseExpression(Precedence::PREFIX);
    return expr;
}

std::shared_ptr<Expression> Parser::ParseIntegerLiteral() {
    auto lit = std::make_shared<IntegerLiteral>(curToken);
    try {
        lit->Value = std::stol(curToken.Literal);
    } catch (...) {
        Errors.push_back("at line: " + std::to_string(curToken.LineNumber) +
                         ", could not parse " + curToken.Literal +
                         " as integer");
        return nullptr;
    }
    return lit;
}

std::shared_ptr<Expression> Parser::ParseBoolean() {
    return std::make_shared<BooleanExpression>(curToken,
                                               CurTokenIs(TokenType::TRUE));
}

std::shared_ptr<Expression> Parser::ParseGroupedExpression() {
    NextToken();
    auto exp = ParseExpression(Precedence::LOWEST);
    if (!ExpectPeek(TokenType::RPAREN)) return nullptr;
    return exp;
}

std::shared_ptr<Expression> Parser::ParseIfExpression() {
    auto expr = std::make_shared<IfExpression>(curToken);
    if (!ExpectPeek(TokenType::LPAREN)) return nullptr;

    NextToken();
    expr->Condition = ParseExpression(Precedence::LOWEST);
    if (!ExpectPeek(TokenType::RPAREN)) return nullptr;
    if (!ExpectPeek(TokenType::LBRACE)) return nullptr;

    expr->Consequence = ParseBlockStatement();

    if (PeekTokenIs(TokenType::ELSE)) {
        NextToken();
        if (!ExpectPeek(TokenType::LBRACE)) return nullptr;
        expr->Alternative = ParseBlockStatement();
    }

    return expr;
}

std::shared_ptr<BlockStatement> Parser::ParseBlockStatement() {
    auto block = std::make_shared<BlockStatement>(curToken);
    NextToken();

    while (!CurTokenIs(TokenType::RBRACE) &&
           !CurTokenIs(TokenType::TOKEN_EOF)) {
        auto stmt = ParseStatement();
        if (stmt != nullptr) {
            block->Statements.push_back(stmt);
        }
        NextToken();
    }
    return block;
}

std::shared_ptr<Expression> Parser::ParseCallExpression(
    std::shared_ptr<Expression> function) {
    auto exp = std::make_shared<CallExpression>(curToken, function);
    exp->Arguments = ParseExpressionList(TokenType::RPAREN);
    return exp;
}

std::vector<std::shared_ptr<Expression>> Parser::ParseExpressionList(
    TokenType end) {
    std::vector<std::shared_ptr<Expression>> list;
    if (PeekTokenIs(end)) {
        NextToken();
        return list;
    }
    NextToken();
    list.push_back(ParseExpression(Precedence::LOWEST));

    while (PeekTokenIs(TokenType::COMMA)) {
        NextToken();
        NextToken();
        list.push_back(ParseExpression(Precedence::LOWEST));
    }
    if (!ExpectPeek(end)) return std::vector<std::shared_ptr<Expression>>();
    return list;
}

std::shared_ptr<Expression> Parser::ParseIndexExpression(
    std::shared_ptr<Expression> left) {
    auto expr = std::make_shared<IndexExpression>(curToken, left);
    NextToken();
    expr->Index = ParseExpression(Precedence::LOWEST);
    if (!ExpectPeek(TokenType::RBRACKET)) return nullptr;
    return expr;
}

std::shared_ptr<FunctionLiteral> Parser::ParseFunctionStatement() {
    auto lit = std::make_shared<FunctionLiteral>(curToken);

    if (!ExpectPeek(TokenType::IDENT)) return nullptr;
    lit->Ident = ParseIdentifier();

    if (!ExpectPeek(TokenType::LPAREN)) return nullptr;

    lit->Parameters = ParseFunctionParameters();
    if (!ExpectPeek(TokenType::LBRACE)) return nullptr;

    lit->Body = ParseBlockStatement();
    return lit;
}

std::vector<std::shared_ptr<Identifier>> Parser::ParseFunctionParameters() {
    std::vector<std::shared_ptr<Identifier>> parameters;

    if (PeekTokenIs(TokenType::RPAREN)) {
        NextToken();
        return parameters;
    }
    NextToken();

    auto ident = std::make_shared<Identifier>(curToken, curToken.Literal);
    parameters.push_back(ident);

    while (PeekTokenIs(TokenType::COMMA)) {
        NextToken();
        NextToken();
        ident = std::make_shared<Identifier>(curToken, curToken.Literal);
        parameters.push_back(ident);
    }
    if (!ExpectPeek(TokenType::RPAREN))
        return std::vector<std::shared_ptr<Identifier>>();

    return parameters;
}

std::shared_ptr<Expression> Parser::ParseStringLiteral() {
    return std::make_shared<StringLiteral>(curToken, curToken.Literal);
}

std::shared_ptr<Expression> Parser::ParseArrayLiteral() {
    auto array = std::make_shared<ArrayLiteral>(curToken);
    array->Elements = ParseExpressionList(TokenType::RBRACKET);
    return array;
}

std::shared_ptr<Expression> Parser::ParseHashLiteral() {
    auto hash = std::make_shared<HashLiteral>(curToken);
    while (!PeekTokenIs(TokenType::RBRACE)) {
        NextToken();
        auto key = ParseExpression(Precedence::LOWEST);
        if (!ExpectPeek(TokenType::COLON)) return nullptr;
        NextToken();
        auto value = ParseExpression(Precedence::LOWEST);
        hash->Pairs[key] = value;

        if (!PeekTokenIs(TokenType::RBRACE) && !ExpectPeek(TokenType::COMMA))
            return nullptr;
    }
    if (!ExpectPeek(TokenType::RBRACE)) return nullptr;
    return hash;
}

std::shared_ptr<Expression> Parser::ParseForExpression() {
    auto expr = std::make_shared<ForExpression>(curToken);
    if (!ExpectPeek(TokenType::LPAREN)) return nullptr;

    auto iter = ParseIterativeExpression();
    if (iter == nullptr) return nullptr;
    expr->Iterative = iter;

    if (!ExpectPeek(TokenType::LBRACE)) return nullptr;
    expr->Body = ParseBlockStatement();
    return expr;
}

std::shared_ptr<ForIterative> Parser::ParseIterativeExpression() {
    if (!ExpectPeek(TokenType::IDENT)) return nullptr;

    auto ident = std::make_shared<Identifier>(curToken, curToken.Literal);
    if (!ExpectPeek(TokenType::IN)) return nullptr;
    NextToken();

    auto array = ParseExpression(Precedence::LOWEST);
    if (array == nullptr) return nullptr;

    NextToken();
    auto forExp = std::make_shared<ForIterative>();
    forExp->Index = ident;
    forExp->Array = array;
    return forExp;
}

void Parser::RegisterPrefix(TokenType tokenType, PrefixParseFn fn) {
    prefixParseFns[tokenType] = fn;
}

void Parser::RegisterInfix(TokenType tokenType, InfixParseFn fn) {
    infixParseFns[tokenType] = fn;
}

bool Parser::CurTokenIs(TokenType t) const { return curToken.Type == t; }

bool Parser::PeekTokenIs(TokenType t) const { return peekToken.Type == t; }

bool Parser::ExpectPeek(TokenType t) {
    if (PeekTokenIs(t)) {
        NextToken();
        return true;
    } else {
        PeekError(t);
        return false;
    }
}

void Parser::PeekError(TokenType t) {
    Errors.push_back("at line: " + std::to_string(curToken.LineNumber) +
                     ", expected next token to be " + TokenTypeToString(t) +
                     ", got " + TokenTypeToString(peekToken.Type) + " instead");
}

void Parser::NoPrefixParseFnError(TokenType t) {
    Errors.push_back("at line: " + std::to_string(curToken.LineNumber) +
                     ", no prefix parse function for " + TokenTypeToString(t) +
                     " found");
}

Precedence Parser::PeekPrecedence() const {
    auto it = precedences.find(peekToken.Type);
    if (it != precedences.end()) {
        return it->second;
    }
    return Precedence::LOWEST;
}

Precedence Parser::CurPrecedence() const {
    auto it = precedences.find(curToken.Type);
    if (it != precedences.end()) {
        return it->second;
    }
    return Precedence::LOWEST;
}

bool Parser::IsAssignOp() const {
    return !PeekTokenIs(TokenType::ASSIGN) &&
           !PeekTokenIs(TokenType::PLUS_EQ) &&
           !PeekTokenIs(TokenType::MIN_EQ) &&
           !PeekTokenIs(TokenType::TIMES_EQ) &&
           !PeekTokenIs(TokenType::DIVIDE_EQ);
}
