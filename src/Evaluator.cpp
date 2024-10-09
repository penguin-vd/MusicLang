#include "Evaluator.hpp"

#include <fmt/core.h>

#include <cstdio>
#include <memory>

#include "Ast.hpp"
#include "Enviroment.hpp"
#include "Object.hpp"

shared_ptr<IObject> Evaluator::Eval(shared_ptr<Node> node, shared_ptr<Env> env) {
    if (auto exp = dynamic_pointer_cast<AstProgram>(node)) {
        return EvalProgram(exp, env);
    } else if (auto exp = dynamic_pointer_cast<BlockStatement>(node)) {
        return EvalBlockStatement(exp, env);
    } else if (auto exp = dynamic_pointer_cast<IfExpression>(node)) {
        return EvalIfExpression(exp, env);
    } else if (auto exp = dynamic_pointer_cast<ExpressionStatement>(node)) {
        return Eval(exp->TheExpression, env);
    } else if (auto exp = dynamic_pointer_cast<IntegerLiteral>(node)) {
        return std::make_shared<Integer>(exp->Value);
    } else if (auto exp = dynamic_pointer_cast<BooleanExpression>(node)) {
        return NativeBoolToBooleanObj(exp->Value);
    } else if (auto exp = dynamic_pointer_cast<StringLiteral>(node)) {
        return make_shared<StringObj>(exp->Value);
    } else if (auto exp = dynamic_pointer_cast<PrefixExpression>(node)) {
        auto preRight = Eval(exp->Right, env);
        if (IsError(preRight)) return preRight;

        return EvalPrefixExpression(exp->Operator, preRight, exp->TheToken.LineNumber);
    } else if (auto exp = dynamic_pointer_cast<InfixExpression>(node)) {
        auto inLeft = Eval(exp->Left, env);
        if (IsError(inLeft)) return inLeft;

        auto inRight = Eval(exp->Right, env);
        if (IsError(inRight)) return inRight;

        return EvalInfixExpression(exp->Operator, inLeft, inRight, exp->TheToken.LineNumber);
    } else if (auto exp = dynamic_pointer_cast<ReturnStatement>(node)) {
        auto retVal = Eval(exp->ReturnValue, env);
        if (IsError(retVal)) return retVal;

        return make_shared<ReturnValue>(retVal);
    } else if (auto exp = dynamic_pointer_cast<LetStatement>(node)) {
        auto letVal = Eval(exp->Value, env);
        if (IsError(letVal)) return letVal;
        if (Builtins.find(exp->Name->Value) != Builtins.end()) {
            return NewError("at line: {0}, {1} is a builtin function", exp->TheToken.LineNumber, exp->Name->Value);
        }

        env->Set(exp->Name->Value, letVal);
    } else if (auto exp = dynamic_pointer_cast<AssignStatement>(node)) {
        auto assignVal = Eval(exp->Value, env);
        if (IsError(assignVal)) return assignVal;

        auto assignRes = EvalAssignStatement(exp, assignVal, env);
        if (IsError(assignRes)) return assignRes;
    } else if (auto exp = dynamic_pointer_cast<Identifier>(node)) {
        return EvalIdentifier(exp, env);
    } else if (auto exp = dynamic_pointer_cast<FunctionLiteral>(node)) {
        auto ident = EvalIdentifier(exp->Ident, env);
        if (IsError(ident)) return ident;

        auto fnParams = exp->Parameters;
        auto body = exp->Body;
        env->Set(exp->Ident->Value, make_shared<Function>(fnParams, body, env));
    } else if (auto exp = dynamic_pointer_cast<CallExpression>(node)) {
        auto function = Eval(exp->Function, env);
        if (IsError(function)) return function;

        auto callArgs = EvalExpressions(exp->Arguments, env);
        if (callArgs.size() == 1 && IsError(callArgs[0])) return callArgs[0];

        return ApplyFunction(function, callArgs, env, exp->TheToken.LineNumber);
    } else if (auto exp = dynamic_pointer_cast<ArrayLiteral>(node)) {
        auto elements = EvalExpressions(exp->Elements, env);
        if (elements.size() == 1 && IsError(elements[0])) return elements[0];

        return make_shared<ArrayObject>(elements);
    } else if (auto exp = dynamic_pointer_cast<IndexExpression>(node)) {
        auto left = Eval(exp->Left, env);
        if (IsError(left)) return left;

        auto index = Eval(exp->Index, env);
        if (IsError(index)) return index;

        return EvalIndexExpression(left, index, exp->TheToken.LineNumber);
    } else if (auto exp = dynamic_pointer_cast<HashLiteral>(node)) {
        return EvalHashLiteral(exp, env);
    } else if (auto exp = dynamic_pointer_cast<ForExpression>(node)) {
        auto forRes = EvalForExpression(exp, env);
    } else if (auto exp = dynamic_pointer_cast<BreakStatement>(node)) {
        return Env::BREAK;
    } else if (auto exp = dynamic_pointer_cast<AccessStatement>(node)) {
        return NewError("NOT IMPLEMENTED");
    }

    return Env::NULLOBJ;
}

shared_ptr<IObject> Evaluator::EvalProgram(shared_ptr<AstProgram> program, shared_ptr<Env> env) {
    shared_ptr<IObject> result = Env::NULLOBJ;
    for (auto& stmt : program->Statements) {
        result = Eval(stmt, env);
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

shared_ptr<IObject> Evaluator::EvalBlockStatement(shared_ptr<BlockStatement> block, shared_ptr<Env> env) {
    shared_ptr<IObject> result = Env::NULLOBJ;
    for (auto& stmt : block->Statements) {
        result = Eval(stmt, env);
        if (result->Type() == ObjectType::RETURN_VALUE || IsError(result) || result->Type() == ObjectType::BREAK) {
            return result;
        }
    }

    return result;
}

shared_ptr<IObject> Evaluator::EvalAssignStatement(shared_ptr<AssignStatement> stmt, shared_ptr<IObject> value, shared_ptr<Env> env) {
    if (auto exp = dynamic_pointer_cast<Identifier>(stmt->Name)) {
        auto obj = env->Get(exp->Value);
        if (obj == nullptr) {
            return NewError("at {0}, variable with name {1} has not been found", stmt->TheToken.LineNumber, stmt->Name->ToString());
        }
        value = EvalAssignOperator(obj, value, stmt->Operator, stmt->TheToken.LineNumber);
        if (IsError(value)) return value;

        env->Set(exp->Value, value);
    } else if (auto exp = dynamic_pointer_cast<IndexExpression>(stmt->Name)) {
        auto left = Eval(exp->Left, env);
        if (IsError(left)) return left;

        auto index = Eval(exp->Index, env);
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
            value = EvalAssignOperator(res, value, stmt->Operator, stmt->TheToken.LineNumber);
            if (IsError(value)) return value;

            env->Set(exp->Left->TokenLiteral(), index, value);
        }
    }

    return Env::NULLOBJ;
}

shared_ptr<IObject> Evaluator::EvalIfExpression(shared_ptr<IfExpression> ie, shared_ptr<Env> env) {
    auto condition = Eval(ie->Condition, env);
    if (IsError(condition)) return condition;

    if (IsTruthy(condition)) {
        return Eval(ie->Consequence, env);
    } else if (ie->Alternative != nullptr) {
        return Eval(ie->Alternative, env);
    }

    return Env::NULLOBJ;
}

shared_ptr<IObject> Evaluator::EvalIdentifier(shared_ptr<Identifier> ident, shared_ptr<Env> env) {
    auto result = env->Get(ident->Value);
    if (result != nullptr) {
        return result;
    }
    if (Builtins.find(ident->Value) != Builtins.end()) {
        return Builtins[ident->Value];
    }
    return NewError("at {}, identifier not found", ident->TheToken.LineNumber);
}

shared_ptr<IObject> Evaluator::EvalForExpression(shared_ptr<ForExpression> exp, shared_ptr<Env> env) {
    auto array = Eval(exp->Iterative->Array, env);
    if (auto arrayObj = dynamic_pointer_cast<ArrayObject>(array)) {
        auto arrEnv = NewEnclosedEnviroment(env);
        for (size_t i = 0; i < arrayObj->Elements.size(); ++i) {
            arrEnv->Set(exp->Iterative->Index->Value, arrayObj->Elements[i]);
            auto res = Eval(exp->Body, arrEnv);
            if (res->Type() == ObjectType::BREAK) break;
        }
    } else if (auto iter = dynamic_pointer_cast<IterObj>(array)) {
        auto iterEnv = NewEnclosedEnviroment(env);
        for (int i = iter->Low; i < iter->High; i += iter->Steps) {
            iterEnv->Set(exp->Iterative->Index->Value, make_shared<Integer>(i));
            auto res = Eval(exp->Body, iterEnv);
            if (res->Type() == ObjectType::BREAK) break;
        }
    } else {
        return NewError("at {0}, for does not support type {1}", exp->TheToken.LineNumber, array->Type());
    }

    return Env::NULLOBJ;
}

shared_ptr<IObject> Evaluator::EvalHashLiteral(shared_ptr<HashLiteral> node, shared_ptr<Env> env) {
    map<HashKey, HashPair> pairs;
    for (const auto& [key, value] : node->Pairs) {
        auto keyObj = Eval(key, env);
        if (IsError(keyObj)) return keyObj;

        auto hash = dynamic_pointer_cast<IHashable>(keyObj);
        if (hash == nullptr) {
            return NewError("at {0}, unusable key {1}", keyObj->Type());
        }

        auto valueObj = Eval(value, env);
        if (IsError(valueObj)) return valueObj;

        pairs[hash->GetHashKey()] = HashPair(keyObj, valueObj);
    }
    return make_shared<Hash>(pairs);
}

shared_ptr<IObject> Evaluator::ApplyFunction(shared_ptr<IObject> fn, vector<shared_ptr<IObject>> args, shared_ptr<Env> env, int line) {
    if (auto func = dynamic_pointer_cast<Function>(fn)) {
        auto extEnv = ExtendFunctionEnv(func, args);
        auto evaluated = Eval(func->Body, extEnv);
        return UnwarpReturnValue(evaluated);
    } else if (auto builtin = dynamic_pointer_cast<BuiltinObj>(fn)) {
        return builtin->Function(args);
    }
    return NewError("at {0}, not a function: {1}", line, fn->Type());
}

vector<shared_ptr<IObject>> Evaluator::EvalExpressions(vector<shared_ptr<Expression>> exps, shared_ptr<Env> env) {
    vector<shared_ptr<IObject>> results;
    for (const auto& exp : exps) {
        auto evaluated = Eval(exp, env);
        if (IsError(evaluated)) return {evaluated};
        results.push_back(evaluated);
    }

    return results;
}

shared_ptr<IObject> Evaluator::EvalAssignOperator(shared_ptr<IObject> oldVal, shared_ptr<IObject> newVal, string op, int line) {
    if (op == "=") {
        return newVal;
    } else if (op == "+=") {
        if (oldVal->Type() != ObjectType::INTEGER || newVal->Type() != ObjectType::INTEGER) {
            return NewError("at {0}, type mismatch: {1} {2} {3}", line, oldVal->Type(), op, newVal->Type());
        }
        return make_shared<Integer>(static_pointer_cast<Integer>(oldVal)->Value + static_pointer_cast<Integer>(newVal)->Value);
    } else if (op == "-=") {
        if (oldVal->Type() != ObjectType::INTEGER || newVal->Type() != ObjectType::INTEGER) {
            return NewError("at {0}, type mismatch: {1} {2} {3}", line, oldVal->Type(), op, newVal->Type());
        }
        return make_shared<Integer>(static_pointer_cast<Integer>(oldVal)->Value - static_pointer_cast<Integer>(newVal)->Value);
    } else if (op == "*=") {
        if (oldVal->Type() != ObjectType::INTEGER || newVal->Type() != ObjectType::INTEGER) {
            return NewError("at {0}, type mismatch: {1} {2} {3}", line, oldVal->Type(), op, newVal->Type());
        }
        return make_shared<Integer>(static_pointer_cast<Integer>(oldVal)->Value * static_pointer_cast<Integer>(newVal)->Value);
    } else {
        return NewError("at {0}, operator '{1}' not recognized", line, op);
    }
}

shared_ptr<IObject> Evaluator::EvalPrefixExpression(string op, shared_ptr<IObject> obj, int line) {
    if (op == "!") {
        return EvalBangOperatorExpression(obj);
    } else if (op == "-") {
        return EvalMinusOperatorExpression(obj, line);
    }
    return NewError("at {0}, unkown operator: {1}{2}", line, op, obj->Type());
}

shared_ptr<IObject> Evaluator::EvalBangOperatorExpression(shared_ptr<IObject> obj) {
    if (auto boolean = dynamic_pointer_cast<BooleanObj>(obj)) {
        return boolean == Env::TRUE ? Env::FALSE : Env::TRUE;
    }
    if (obj->Type() == ObjectType::NULL_OBJ) return Env::TRUE;

    return Env::FALSE;
}

shared_ptr<IObject> Evaluator::EvalMinusOperatorExpression(shared_ptr<IObject> obj, int line) {
    if (obj->Type() != ObjectType::INTEGER) {
        return NewError("at {0}, unknown operaitor: -{1}", line, obj->Type());
        ;
    }
    return make_shared<Integer>(-(static_pointer_cast<Integer>(obj)->Value));
}

shared_ptr<IObject> Evaluator::EvalInfixExpression(string op, shared_ptr<IObject> left, shared_ptr<IObject> right, int line) {
    if (left->Type() == ObjectType::INTEGER && left->Type() == ObjectType::INTEGER) {
        return EvalIntegerInfixExpression(op, static_pointer_cast<Integer>(left), static_pointer_cast<Integer>(right), line);
    }

    if (left->Type() != right->Type()) {
        return NewError("at {0}, type mismatch: {1} {2} {3}", line, left->Type(), op, right->Type());
    }

    if (left->Type() == ObjectType::STRING) {
        return EvalStringInfixExpression(op, static_pointer_cast<StringObj>(left), static_pointer_cast<StringObj>(right), line);
    }

    if (op == "==") {
        return NativeBoolToBooleanObj(left->Inspect() == right->Inspect());
    } else if (op == "!=") {
        return NativeBoolToBooleanObj(left->Inspect() != right->Inspect());
    }

    return NewError("at {0}, unknown operator: {1} {2} {3}", line, left->Type(), op, right->Type());
}

shared_ptr<IObject> Evaluator::EvalIntegerInfixExpression(string op, shared_ptr<Integer> left, shared_ptr<Integer> right, int line) {
    int leftVal = left->Value;
    int rightVal = right->Value;
    if (op == "+") {
        return make_shared<Integer>(leftVal + rightVal);
    } else if (op == "-") {
        return make_shared<Integer>(leftVal - rightVal);
    } else if (op == "*") {
        return make_shared<Integer>(leftVal * rightVal);
    } else if (op == "/") {
        return make_shared<Integer>(leftVal / rightVal);
    } else if (op == "<") {
        return NativeBoolToBooleanObj(leftVal < rightVal);
    } else if (op == ">") {
        return NativeBoolToBooleanObj(leftVal > rightVal);
    } else if (op == "==") {
        return NativeBoolToBooleanObj(leftVal == rightVal);
    } else if (op == "!=") {
        return NativeBoolToBooleanObj(leftVal != rightVal);
    }
    return NewError("at {0}, unknown operator: {1} {2} {3}", line, left->Type(), op, right->Type());
}

shared_ptr<IObject> Evaluator::EvalStringInfixExpression(string op, shared_ptr<StringObj> left, shared_ptr<StringObj> right, int line) {
    string leftVal = left->Value;
    string rightVal = right->Value;
    if (op == "+") {
        return make_shared<StringObj>(leftVal + rightVal);
    } else if (op == "==") {
        return NativeBoolToBooleanObj(leftVal == rightVal);
    } else if (op == "!=") {
        return NativeBoolToBooleanObj(leftVal != rightVal);
    }
    return NewError("at {0}, unknown operator: {1} {2} {3}", line, left->Type(), op, right->Type());
}

shared_ptr<IObject> Evaluator::EvalIndexExpression(shared_ptr<IObject> left, shared_ptr<IObject> index, int line) {
    if (left->Type() == ObjectType::ARRAY && index->Type() == ObjectType::INTEGER) {
        return EvalArrayIndexExpression(static_pointer_cast<ArrayObject>(left), static_pointer_cast<Integer>(index));
    }
    if (left->Type() == ObjectType::HASH) {
        return EvalHashIndexExpression(static_pointer_cast<Hash>(left), index, line);
    }
    return NewError("at {0}, index operator not supported: {1}", line, left->Type());
}


shared_ptr<IObject> Evaluator::EvalArrayIndexExpression(shared_ptr<ArrayObject> array, shared_ptr<Integer> index) {
    int idx = index->Value;
    if (0 > idx || idx > array->Elements.size()) {
        return Env::NULLOBJ;
    }
    return array->Elements[idx];
}

shared_ptr<IObject> Evaluator::EvalHashIndexExpression(shared_ptr<Hash> hash, shared_ptr<IObject> index, int line) {
    auto hashAble = dynamic_pointer_cast<IHashable>(index);
    if (!hashAble) {
        return NewError("at {0}, unusable as hash key: {1}", line, index->Type());
    }

    if (hash->Pairs.find(hashAble->GetHashKey()) != hash->Pairs.end()) {
        return hash->Pairs[hashAble->GetHashKey()].Value;
    }

    return Env::NULLOBJ;
}

shared_ptr<BooleanObj> Evaluator::NativeBoolToBooleanObj(bool input) {
    return input ? Env::TRUE : Env::FALSE;
}

shared_ptr<Env> Evaluator::ExtendFunctionEnv(shared_ptr<Function> fn, vector<shared_ptr<IObject>> args) {
    auto extEnv = NewEnclosedEnviroment(fn->Enviroment);
    for (size_t i = 0; i < fn->Parameters.size(); ++i) {
        extEnv->Set(fn->Parameters[i]->Value, args[i]);
    }
    return extEnv;
}

shared_ptr<IObject> Evaluator::UnwarpReturnValue(shared_ptr<IObject> obj) {
    if (obj->Type() == ObjectType::RETURN_VALUE) {
        return static_pointer_cast<ReturnValue>(obj)->Value;
    }
    return obj;
}

bool Evaluator::IsTruthy(shared_ptr<IObject> obj) {
    if (obj->Type() != ObjectType::BOOLEAN && obj->Type() != ObjectType::NULL_OBJ) {
        return true;
    }
    if (obj->Type() == ObjectType::BOOLEAN) {
        return static_pointer_cast<BooleanObj>(obj)->Value;
    }
    return false;
}

bool Evaluator::IsError(shared_ptr<IObject> obj) {
    if (obj != nullptr) {
        return obj->Type() == ObjectType::ERROR;
    }
    return false;
}

template <typename... Args>
shared_ptr<Error> Evaluator::NewError(string format, Args... args) {
    return make_shared<Error>(fmt::format(format, args...));
}

shared_ptr<Error> Evaluator::NewError(string format) {
    return make_shared<Error>(format);
}
