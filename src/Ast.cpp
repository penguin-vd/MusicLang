#include "Ast.hpp"
#include "Enviroment.hpp"
#include "Evaluator.hpp"
#include "Object.hpp"
#include "fmt/core.h"
#include "fmt/format.h"

#include <memory>

template <typename... Args>
shared_ptr<Error> NewerError(string format, Args... args) {
    return make_shared<Error>(fmt::format(format, args...));
}

shared_ptr<Error> NewerError(string format) {
    return make_shared<Error>(format);
}

map<string, std::shared_ptr<IObject>> Builtins = {
    {"exit", make_shared<BuiltinObj>(ExitCall) },
    {"range", make_shared<BuiltinObj>(Range) },
    {"print", make_shared<BuiltinObj>(Print) }
};

map<string, shared_ptr<IObject>> AccessFunctions {
    {"Type", make_shared<AccessFuncObj>(Type) }
};

std::shared_ptr<IObject> AstProgram::Evaluate(std::shared_ptr<Env> env) {
    shared_ptr<IObject> result = Env::NULLOBJ;
    for (auto& stmt : this->Statements) {
        result = stmt->Evaluate(env);
        if (result->Type() == ObjectType::RETURN_VALUE) {
            auto ret = dynamic_pointer_cast<ReturnValue>(result);
            return ret->Value;
        }

        if (result->Type() == ObjectType::EXIT || result->Type() == ObjectType::ERROR) {
            return result;
        }
    }

    return result;
}

std::shared_ptr<IObject> Identifier::Evaluate(std::shared_ptr<Env> env) {
    auto result = env->Get(this->Value);
    if (result != nullptr) {
        return result;
    }
    if (Builtins.find(this->Value) != Builtins.end()) {
        return Builtins[this->Value];
    }
    return NewerError("at {}, identifier '{}' not found", this->TheToken.LineNumber, this->Value);
}

std::shared_ptr<IObject> IntegerLiteral::Evaluate(std::shared_ptr<Env> env) {
    return std::make_shared<Integer>(this->Value);
}

std::shared_ptr<IObject> PrefixExpression::Evaluate(std::shared_ptr<Env> env) {
    auto preRight = this->Right->Evaluate(env);
    if (IsError(preRight)) return preRight;

    return EvalPrefixExpression(this->Operator, preRight, this->TheToken.LineNumber);
}

std::shared_ptr<IObject> InfixExpression::Evaluate(std::shared_ptr<Env> env) {
        auto inLeft = this->Left->Evaluate(env);
        if (IsError(inLeft)) return inLeft;

        auto inRight = this->Right->Evaluate(env);
        if (IsError(inRight)) return inRight;

        return EvalInfixExpression(this->Operator, inLeft, inRight, this->TheToken.LineNumber);
}

std::shared_ptr<IObject> BooleanExpression::Evaluate(std::shared_ptr<Env> env) {
    return NativeBoolToBooleanObj(this->Value);
}

std::shared_ptr<IObject> IndexExpression::Evaluate(std::shared_ptr<Env> env) {
    auto left = this->Left->Evaluate(env);
    if (IsError(left)) return left;

    auto index = this->Index->Evaluate(env);
    if (IsError(index)) return index;

    return EvalIndexExpression(left, index, this->TheToken.LineNumber);
}

std::shared_ptr<IObject> CallExpression::Evaluate(std::shared_ptr<Env> env) {
    auto function = this->Function->Evaluate(env);
    if (IsError(function)) return function;

    auto callArgs = EvalExpressions(this->Arguments, env);
    if (callArgs.size() == 1 && IsError(callArgs[0])) return callArgs[0];

    return ApplyFunction(function, callArgs, env, this->TheToken.LineNumber);
}

std::shared_ptr<IObject> LetStatement::Evaluate(std::shared_ptr<Env> env) {
    auto letVal = this->Value->Evaluate(env);
    if (IsError(letVal)) return letVal;
    if (Builtins.find(this->Name->Value) != Builtins.end()) {
        return NewerError("at line: {0}, {1} is a builtin function", this->TheToken.LineNumber, this->Name->Value);
    }

    env->Set(this->Name->Value, letVal);
    return Env::NULLOBJ;
}

std::shared_ptr<IObject> AssignStatement::Evaluate(std::shared_ptr<Env> env) {
    auto value = this->Value->Evaluate(env);
    if (IsError(value)) return value;

    if (auto exp = dynamic_pointer_cast<Identifier>(this->Name)) {
        auto obj = env->Get(exp->Value);
        if (obj == nullptr) {
            return NewerError("at {0}, variable with name {1} has not been found", this->TheToken.LineNumber, this->Name->ToString());
        }
        value = EvalAssignOperator(obj, value, this->Operator, this->TheToken.LineNumber);
        if (IsError(value)) return value;

        env->Set(exp->Value, value);
    } else if (auto exp = dynamic_pointer_cast<IndexExpression>(this->Name)) {
        auto left = exp->Left->Evaluate(env);
        if (IsError(left)) return left;

        auto index = exp->Index->Evaluate(env);
        if (IsError(index)) return index;

        std::shared_ptr<IObject> res = nullptr;
        if (auto hash = dynamic_pointer_cast<IHashable>(index)) {
            res = env->GetHashObject(exp->Left->TokenLiteral(), hash);
        }
        if (res == nullptr) {
            if (auto i = dynamic_pointer_cast<Integer>(index)) {
                res = env->GetArrayObject(exp->Left->TokenLiteral(), i);
            }
        }

        if (res == nullptr) {
            env->Set(exp->Left->TokenLiteral(), index, value);
        } else {
            value = EvalAssignOperator(res, value, this->Operator, this->TheToken.LineNumber);
            if (IsError(value)) return value;

            env->Set(exp->Left->TokenLiteral(), index, value);
        }
    }

    return Env::NULLOBJ;
}

std::shared_ptr<IObject> ExpressionStatement::Evaluate(std::shared_ptr<Env> env) {
    return this->TheExpression->Evaluate(env);
}

std::shared_ptr<IObject> BlockStatement::Evaluate(std::shared_ptr<Env> env) {
    shared_ptr<IObject> result = Env::NULLOBJ;
    for (auto& stmt : this->Statements) {
        result = stmt->Evaluate(env);
        if (result->Type() == ObjectType::RETURN_VALUE || IsError(result) || result->Type() == ObjectType::BREAK) {
            return result;
        }
    }

    return result;
}

std::shared_ptr<IObject> ReturnStatement::Evaluate(std::shared_ptr<Env> env) {
    auto retVal = this->Value->Evaluate(env);
    if (IsError(retVal)) return retVal;

    return std::make_shared<ReturnValue>(retVal);
}

std::shared_ptr<IObject> BreakStatement::Evaluate(std::shared_ptr<Env> env) {
    return Env::BREAK;
}

std::shared_ptr<IObject> AccessExpression::Evaluate(std::shared_ptr<Env> env) {
    auto parent = Parent->Evaluate(env);
    std::shared_ptr<Env> objEnv = make_shared<Env>(AccessFunctions);
    objEnv->Set("_this", parent);
    auto name = TheStatement->Evaluate(objEnv);
    return name;
}

std::shared_ptr<IObject> IfExpression::Evaluate(std::shared_ptr<Env> env) {
    auto condition = this->Condition->Evaluate(env);
    if (IsError(condition)) return condition;

    if (IsTruthy(condition)) {
        return this->Consequence->Evaluate(env);
    } else if (this->Alternative != nullptr) {
        return this->Alternative->Evaluate(env);
    }

    return Env::NULLOBJ;
}

std::shared_ptr<IObject> ForIterative::Evaluate(std::shared_ptr<Env> env) {
    return Array->Evaluate(env);
}

std::shared_ptr<IObject> ForExpression::Evaluate(std::shared_ptr<Env> env) {
    auto array = this->Iterative->Evaluate(env);
    if (auto arrayObj = dynamic_pointer_cast<ArrayObject>(array)) {
        auto arrEnv = NewEnclosedEnviroment(env);
        for (size_t i = 0; i < arrayObj->Elements.size(); ++i) {
            arrEnv->Set(this->Iterative->Index->Value, arrayObj->Elements[i]);
            auto res = this->Body->Evaluate(arrEnv);
            if (res->Type() == ObjectType::BREAK) break;
        }
    } else if (auto iter = dynamic_pointer_cast<IterObj>(array)) {
        auto iterEnv = NewEnclosedEnviroment(env);
        for (int i = iter->Low; i < iter->High; i += iter->Steps) {
            iterEnv->Set(this->Iterative->Index->Value, make_shared<Integer>(i));
            auto res = this->Body->Evaluate(iterEnv);
            if (res->Type() == ObjectType::BREAK) break;
        }
    } else {
        return NewerError("at {0}, for does not support type {1}", this->TheToken.LineNumber, array->Type());
    }

    return Env::NULLOBJ;
}

std::shared_ptr<IObject> FunctionLiteral::Evaluate(std::shared_ptr<Env> env) {
    auto ident = this->Ident->Evaluate(env);
    if (IsError(ident)) return ident;

    env->Set(this->Ident->Value, make_shared<Function>(this->Parameters, this->Body, env));
    return Env::NULLOBJ;
}

std::shared_ptr<IObject> StringLiteral::Evaluate(std::shared_ptr<Env> env) {
    return make_shared<StringObj>(this->Value);
}

std::shared_ptr<IObject> ArrayLiteral::Evaluate(std::shared_ptr<Env> env) {
    auto elements = EvalExpressions(this->Elements, env);
    if (elements.size() == 1 && IsError(elements[0])) return elements[0];

    return make_shared<ArrayObject>(elements);
}

std::shared_ptr<IObject> HashLiteral::Evaluate(std::shared_ptr<Env> env) {
    map<HashKey, HashPair> pairs;
    for (const auto& [key, value] : this->Pairs) {
        auto keyObj = key->Evaluate(env);
        if (IsError(keyObj)) return keyObj;

        auto hash = dynamic_pointer_cast<IHashable>(keyObj);
        if (hash == nullptr) {
            return NewerError("at {0}, unusable key {1}", keyObj->Type());
        }

        auto valueObj = value->Evaluate(env);
        if (IsError(valueObj)) return valueObj;

        pairs[hash->GetHashKey()] = HashPair(keyObj, valueObj);
    }
    return make_shared<Hash>(pairs);
}

