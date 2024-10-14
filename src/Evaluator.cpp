#include "Evaluator.hpp"

#include <fmt/core.h>

#include <cstdio>
#include <map>
#include <memory>

#include "Ast.hpp"
#include "Enviroment.hpp"
#include "Object.hpp"

template <typename... Args>
shared_ptr<Error> NewError(string format, Args... args) {
    return make_shared<Error>(fmt::format(format, args...));
}

shared_ptr<Error> NewError(string format) {
    return make_shared<Error>(format);
}

shared_ptr<IObject> ApplyFunction(shared_ptr<IObject> fn, vector<shared_ptr<IObject>> args, shared_ptr<Env> env, int line) {
    if (auto func = dynamic_pointer_cast<Function>(fn)) {
        auto extEnv = ExtendFunctionEnv(func, args);
        auto evaluated = func->Body->Evaluate(extEnv);
        return UnwarpReturnValue(evaluated);
    } else if (auto builtin = dynamic_pointer_cast<BuiltinObj>(fn)) {
        return builtin->Function(args);
    } else if (auto access = dynamic_pointer_cast<AccessFuncObj>(fn)) {
        return access->Function(env->Get("_this"), args);
    }
    return NewError("at {0}, not a function: {1}", line, fn->Type());
}

vector<shared_ptr<IObject>> EvalExpressions(vector<shared_ptr<Expression>> exps, shared_ptr<Env> env) {
    vector<shared_ptr<IObject>> results;
    for (const auto& exp : exps) {
        auto evaluated = exp->Evaluate(env);
        if (IsError(evaluated)) return {evaluated};
        results.push_back(evaluated);
    }

    return results;
}

shared_ptr<IObject> EvalAssignOperator(shared_ptr<IObject> oldVal, shared_ptr<IObject> newVal, string op, int line) {
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

shared_ptr<IObject> EvalPrefixExpression(string op, shared_ptr<IObject> obj, int line) {
    if (op == "!") {
        return EvalBangOperatorExpression(obj);
    } else if (op == "-") {
        return EvalMinusOperatorExpression(obj, line);
    }
    return NewError("at {0}, unkown operator: {1}{2}", line, op, obj->Type());
}

shared_ptr<IObject> EvalBangOperatorExpression(shared_ptr<IObject> obj) {
    if (auto boolean = dynamic_pointer_cast<BooleanObj>(obj)) {
        return boolean == Env::TRUE ? Env::FALSE : Env::TRUE;
    }
    if (obj->Type() == ObjectType::NULL_OBJ) return Env::TRUE;

    return Env::FALSE;
}

shared_ptr<IObject> EvalMinusOperatorExpression(shared_ptr<IObject> obj, int line) {
    if (obj->Type() != ObjectType::INTEGER) {
        return NewError("at {0}, unknown operaitor: -{1}", line, obj->Type());
        ;
    }
    return make_shared<Integer>(-(static_pointer_cast<Integer>(obj)->Value));
}

shared_ptr<IObject> EvalInfixExpression(string op, shared_ptr<IObject> left, shared_ptr<IObject> right, int line) {
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

shared_ptr<IObject> EvalIntegerInfixExpression(string op, shared_ptr<Integer> left, shared_ptr<Integer> right, int line) {
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

shared_ptr<IObject> EvalStringInfixExpression(string op, shared_ptr<StringObj> left, shared_ptr<StringObj> right, int line) {
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

shared_ptr<IObject> EvalIndexExpression(shared_ptr<IObject> left, shared_ptr<IObject> index, int line) {
    if (left->Type() == ObjectType::ARRAY && index->Type() == ObjectType::INTEGER) {
        return EvalArrayIndexExpression(static_pointer_cast<ArrayObject>(left), static_pointer_cast<Integer>(index));
    }
    if (left->Type() == ObjectType::HASH) {
        return EvalHashIndexExpression(static_pointer_cast<Hash>(left), index, line);
    }
    return NewError("at {0}, index operator not supported: {1}", line, left->Type());
}


shared_ptr<IObject> EvalArrayIndexExpression(shared_ptr<ArrayObject> array, shared_ptr<Integer> index) {
    int idx = index->Value;
    if (0 > idx || idx > array->Elements.size()) {
        return Env::NULLOBJ;
    }
    return array->Elements[idx];
}

shared_ptr<IObject> EvalHashIndexExpression(shared_ptr<Hash> hash, shared_ptr<IObject> index, int line) {
    auto hashAble = dynamic_pointer_cast<IHashable>(index);
    if (!hashAble) {
        return NewError("at {0}, unusable as hash key: {1}", line, index->Type());
    }

    if (hash->Pairs.find(hashAble->GetHashKey()) != hash->Pairs.end()) {
        return hash->Pairs[hashAble->GetHashKey()].Value;
    }

    return Env::NULLOBJ;
}

shared_ptr<BooleanObj> NativeBoolToBooleanObj(bool input) {
    return input ? Env::TRUE : Env::FALSE;
}

shared_ptr<Env> ExtendFunctionEnv(shared_ptr<Function> fn, vector<shared_ptr<IObject>> args) {
    auto extEnv = NewEnclosedEnviroment(fn->Enviroment);
    for (size_t i = 0; i < fn->Parameters.size(); ++i) {
        extEnv->Set(fn->Parameters[i]->Value, args[i]);
    }
    return extEnv;
}

shared_ptr<IObject> UnwarpReturnValue(shared_ptr<IObject> obj) {
    if (obj->Type() == ObjectType::RETURN_VALUE) {
        return static_pointer_cast<ReturnValue>(obj)->Value;
    }
    return obj;
}

bool IsTruthy(shared_ptr<IObject> obj) {
    if (obj->Type() != ObjectType::BOOLEAN && obj->Type() != ObjectType::NULL_OBJ) {
        return true;
    }
    if (obj->Type() == ObjectType::BOOLEAN) {
        return static_pointer_cast<BooleanObj>(obj)->Value;
    }
    return false;
}

bool IsError(shared_ptr<IObject> obj) {
    if (obj != nullptr) {
        return obj->Type() == ObjectType::ERROR;
    }
    return false;
}
