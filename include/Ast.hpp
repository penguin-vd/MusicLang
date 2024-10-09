#pragma once
#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Token.hpp"

class Node {
   public:
    virtual std::string TokenLiteral() = 0;
    virtual std::string ToString() = 0;
    virtual ~Node() {}
};

class Statement : public Node {
   public:
    virtual void StatementNode() = 0;
};

class Expression : public Node {
   public:
    virtual void ExpressionNode() = 0;
};

struct AstProgram : public Node {
    std::vector<std::shared_ptr<Statement>> Statements;
    std::string TokenLiteral() override {
        if (!Statements.empty()) {
            return Statements[0]->TokenLiteral();
        }
        return "";
    }
    std::string ToString() override {
        std::string temp = "";
        for (const auto& statement : Statements) {
            temp += statement->ToString() + "\n";
        }
        return temp;
    }
};

// Expressions
struct Identifier : public Expression {
    Token TheToken;
    std::string Value;

    Identifier(Token t, std::string v) : TheToken(t), Value(v) {}
    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }
    std::string ToString() override { return TheToken.Literal; }
};

struct IntegerLiteral : public Expression {
    Token TheToken;
    int Value;

    IntegerLiteral(Token t) : TheToken(t) {}
    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }

    std::string ToString() override { return TheToken.Literal; }
};

struct PrefixExpression : public Expression {
    Token TheToken;
    std::string Operator;
    std::shared_ptr<Expression> Right;

    PrefixExpression(Token t, std::string op) : TheToken(t), Operator(op) {}
    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }

    std::string ToString() override {
        return "(" + Operator + Right->ToString() + ")";
    }
};

struct InfixExpression : public Expression {
    Token TheToken;
    std::string Operator;

    std::shared_ptr<Expression> Left;
    std::shared_ptr<Expression> Right;

    InfixExpression(Token t, std::string op, std::shared_ptr<Expression> left)
        : TheToken(t), Operator(op), Left(left) {}
    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }

    std::string ToString() override {
        return "(" + Left->ToString() + " " + Operator + " " +
               Right->ToString() + ")";
    }
};

struct BooleanExpression : public Expression {
    Token TheToken;
    bool Value;

    BooleanExpression(Token t, bool v) : TheToken(t), Value(v) {}
    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }

    std::string ToString() override { return TheToken.Literal; }
};

struct IndexExpression : public Expression {
    Token TheToken;
    std::shared_ptr<Expression> Left;
    std::shared_ptr<Expression> Index;

    IndexExpression(Token t, std::shared_ptr<Expression> left)
        : TheToken(t), Left(left) {}
    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }

    std::string ToString() override {
        return "({" + Left->ToString() + "}[{" + Index->ToString() + "}])";
    }
};

struct CallExpression : public Expression {
    Token TheToken;
    std::shared_ptr<Expression> Function;
    std::vector<std::shared_ptr<Expression>> Arguments;

    CallExpression(Token t, std::shared_ptr<Expression> func)
        : TheToken(t), Function(func) {}
    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }

    std::string ToString() override {
        std::string temp = Function->ToString() + "(";
        for (size_t i = 0; i < Arguments.size(); ++i) {
            temp += Arguments[i]->ToString();
            temp += ", ";
        }
        if (!Arguments.empty()) temp.resize(temp.size() - 2);
        temp += ")";
        return temp;
    }
};

// Statements
struct LetStatement : public Statement {
    Token TheToken;
    std::shared_ptr<Identifier> Name;
    std::shared_ptr<Expression> Value;
    LetStatement(Token t) : TheToken(t) {}
    void StatementNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }

    std::string ToString() override {
        return TheToken.Literal + " " + Name->ToString() + " = " +
               Value->ToString();
    }
};

struct AssignStatement : public Statement {
    Token TheToken;
    std::shared_ptr<Expression> Name;
    std::string Operator;
    std::shared_ptr<Expression> Value;

    AssignStatement(Token t) : TheToken(t) {}

    void StatementNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }

    std::string ToString() override {
        return Name->ToString() + " " + Operator + " " + Value->ToString();
    }
};

struct ExpressionStatement : public Statement {
    Token TheToken;
    std::shared_ptr<Expression> TheExpression;

    ExpressionStatement(Token t) : TheToken(t) {}
    void StatementNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }

    std::string ToString() override { return TheExpression->ToString(); }
};

struct BlockStatement : public Statement {
    Token TheToken;
    std::vector<std::shared_ptr<Statement>> Statements;

    BlockStatement(Token t) : TheToken(t) {}
    void StatementNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }
    std::string ToString() override {
        std::string tmp = "{ ";
        for (const auto& statement : Statements) {
            tmp += statement->ToString() + "; ";
        }
        tmp += "}";
        return tmp;
    }
};

struct ReturnStatement : public Statement {
    Token TheToken;
    std::shared_ptr<Expression> ReturnValue;

    ReturnStatement(Token t) : TheToken(t) {}

    void StatementNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }

    std::string ToString() override {
        return TheToken.Literal + " " + ReturnValue->ToString() + ";";
    }
};

struct BreakStatement : public Statement {
    Token TheToken;

    BreakStatement(Token t) : TheToken(t) {}
    void StatementNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }
    std::string ToString() override { return "break"; }
};

struct AccessStatement : public Statement {
    Token TheToken;

    std::shared_ptr<Expression> Name;
    std::shared_ptr<Expression> Parent;
    std::shared_ptr<Statement> Statement;

    AccessStatement(Token t) : TheToken(t) {}
    void StatementNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }
    std::string ToString() override {
        return Parent->ToString() + "->" + Statement->ToString();
    }
};

// Block Expressions
struct IfExpression : public Expression {
    Token TheToken;
    std::shared_ptr<Expression> Condition;
    std::shared_ptr<BlockStatement> Consequence;
    std::shared_ptr<BlockStatement> Alternative;

    IfExpression(Token t) : TheToken(t) {}
    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }
    std::string ToString() override {
        std::string tmp = "if";
        tmp += " " + Condition->ToString();
        tmp += " {" + Consequence->ToString() + "}";
        if (Alternative != nullptr) {
            tmp += "else {" + Alternative->ToString() + "}";
        }

        return tmp;
    }
};

struct ForIterative : public Expression {
    Token TheToken;
    std::shared_ptr<Identifier> Index;
    std::shared_ptr<Expression> Array;

    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }

    std::string ToString() override {
        return Index->ToString() + " in " + Array->ToString();
    }
};

struct ForExpression : public Expression {
    Token TheToken;
    std::shared_ptr<ForIterative> Iterative;
    std::shared_ptr<BlockStatement> Body;

    ForExpression(Token t) : TheToken(t) {}
    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }

    std::string ToString() override {
        return "for(" + Iterative->ToString() + ") {" + Body->ToString() + "}";
    }
};

struct FunctionLiteral : public Statement {
    Token TheToken;
    std::shared_ptr<Identifier> Ident;
    std::vector<std::shared_ptr<Identifier>> Parameters;
    std::shared_ptr<BlockStatement> Body;

    void StatementNode() override {}

    FunctionLiteral(Token t) : TheToken(t) {}
    std::string TokenLiteral() override { return TheToken.Literal; }
    std::string ToString() override {
        std::string temp = TokenLiteral() + " ";
        temp += Ident->ToString() + "(";
        for (size_t i = 0; i < Parameters.size(); i++) {
            temp += Parameters[i]->ToString();
            temp += ", ";
        }
        if (!Parameters.empty()) temp.resize(temp.size() - 2);
        temp += ") " + Body->ToString();
        return temp;
    }
};

struct StringLiteral : public Expression {
    Token TheToken;
    std::string Value;

    StringLiteral(Token t, std::string v) : TheToken(t), Value(v) {}
    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }
    std::string ToString() override { return TheToken.Literal; }
};

struct ArrayLiteral : public Expression {
    Token TheToken;
    std::vector<std::shared_ptr<Expression>> Elements;

    ArrayLiteral(Token t) : TheToken(t) {}
    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }
    std::string ToString() override {
        std::string temp = "[";
        for (size_t i = 0; i < Elements.size(); ++i) {
            temp += Elements[i]->ToString();
            temp += ", ";
        }
        if (!Elements.empty()) temp.resize(temp.size() - 2);
        temp += "]";
        return temp;
    }
};

struct HashLiteral : public Expression {
    Token TheToken;
    std::map<std::shared_ptr<Expression>, std::shared_ptr<Expression>> Pairs;

    HashLiteral(Token t) : TheToken(t) {}
    void ExpressionNode() override {}
    std::string TokenLiteral() override { return TheToken.Literal; }
    std::string ToString() override {
        std::string temp = "{";
        for (const auto& [key, value] : Pairs) {
            temp += key->ToString();
            temp += ":";
            temp += value->ToString();
            temp += ", ";
        }
        if (!Pairs.empty()) temp.resize(temp.size() - 2);
        temp += "}";
        return temp;
    }
};
