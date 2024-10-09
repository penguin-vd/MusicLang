#pragma once
#include <map>
#include <memory>

#include "Ast.hpp"
#include "Enviroment.hpp"
#include "Object.hpp"
#include "Builtins.hpp"

using namespace std;
class Evaluator {
   public:
    shared_ptr<IObject> Eval(shared_ptr<Node> node, shared_ptr<Env> env);

    map<string, shared_ptr<IObject>> Builtins;
    Evaluator() {
        Builtins["exit"] = make_shared<BuiltinObj>(ExitCall);
    }

   private:
    // Eval Functions
    shared_ptr<IObject> EvalProgram(shared_ptr<AstProgram> program, shared_ptr<Env> env);
    shared_ptr<IObject> EvalBlockStatement(shared_ptr<BlockStatement> block, shared_ptr<Env> env);
    shared_ptr<IObject> EvalAssignStatement(shared_ptr<AssignStatement> stmt, shared_ptr<IObject> value, shared_ptr<Env> env);
    shared_ptr<IObject> EvalIfExpression(shared_ptr<IfExpression> ie, shared_ptr<Env> env);
    shared_ptr<IObject> EvalIdentifier(shared_ptr<Identifier> ident, shared_ptr<Env> env);
    shared_ptr<IObject> EvalForExpression(shared_ptr<ForExpression> exp, shared_ptr<Env> env);
    shared_ptr<IObject> EvalHashLiteral(shared_ptr<HashLiteral> node, shared_ptr<Env> env);
    shared_ptr<IObject> ApplyFunction(shared_ptr<IObject> fn, vector<shared_ptr<IObject>> args, shared_ptr<Env> env, int line);
    vector<shared_ptr<IObject>> EvalExpressions(vector<shared_ptr<Expression>> exps, shared_ptr<Env> env);

    shared_ptr<IObject> EvalAssignOperator(shared_ptr<IObject> oldVal, shared_ptr<IObject> newVal, string op, int line);
    shared_ptr<IObject> EvalPrefixExpression(string op, shared_ptr<IObject> obj, int line);
    shared_ptr<IObject> EvalBangOperatorExpression(shared_ptr<IObject> obj);
    shared_ptr<IObject> EvalMinusOperatorExpression(shared_ptr<IObject> obj, int line);
    shared_ptr<IObject> EvalInfixExpression(string op, shared_ptr<IObject> left, shared_ptr<IObject> right, int line);
    shared_ptr<IObject> EvalIntegerInfixExpression(string op, shared_ptr<Integer> left, shared_ptr<Integer> right, int line);
    shared_ptr<IObject> EvalStringInfixExpression(string op, shared_ptr<StringObj> left, shared_ptr<StringObj> right, int line);
    shared_ptr<IObject> EvalIndexExpression(shared_ptr<IObject> left, shared_ptr<IObject> index, int line);
    shared_ptr<IObject> EvalArrayIndexExpression(shared_ptr<ArrayObject> array, shared_ptr<Integer> index);
    shared_ptr<IObject> EvalHashIndexExpression(shared_ptr<Hash> hash, shared_ptr<IObject> index, int line);

    shared_ptr<BooleanObj> NativeBoolToBooleanObj(bool input);
    shared_ptr<Env> ExtendFunctionEnv(shared_ptr<Function> fn, vector<shared_ptr<IObject>> args);
    shared_ptr<IObject> UnwarpReturnValue(shared_ptr<IObject> obj);
    bool IsTruthy(shared_ptr<IObject> obj);
    bool IsError(shared_ptr<IObject> obj);

    template <typename... Args>
    shared_ptr<Error> NewError(string format, Args... args);
    shared_ptr<Error> NewError(string format);
};
