#pragma once
#include <map>
#include <string>

#include "Ast.hpp"
#include "Object.hpp"

using namespace std;
class Env {
   public:
    static shared_ptr<BooleanObj> TRUE;
    static shared_ptr<BooleanObj> FALSE;
    static shared_ptr<Null> NULLOBJ;
    static shared_ptr<BreakObj> BREAK;

    map<string, shared_ptr<IObject>> Store;
    shared_ptr<Env> Outer;

    shared_ptr<IObject> Get(string name);
    shared_ptr<IObject> Set(string name, shared_ptr<IObject> val);
    shared_ptr<IObject> Set(string name, shared_ptr<IObject> index, shared_ptr<IObject> assignVal);

    shared_ptr<IObject> GetHashObject(string name, shared_ptr<IHashable> index);
    shared_ptr<IObject> GetArrayObject(string name, shared_ptr<Integer> index);

    shared_ptr<IObject> AddEnv(shared_ptr<Env> env);
};

shared_ptr<Env> NewEnclosedEnviroment(shared_ptr<Env> outer);

struct Function : public IObject {
    vector<shared_ptr<Identifier>> Parameters;
    shared_ptr<BlockStatement> Body;
    shared_ptr<Env> Enviroment;

    Function(vector<shared_ptr<Identifier>> p, shared_ptr<BlockStatement> b, shared_ptr<Env> e) : Parameters(p), Body(b), Enviroment(e) {}

    ObjectType Type() override { return ObjectType::FUNCTION; }
    string Inspect() override {
        string temp = "function (";
        for (const auto& param : Parameters) {
            temp += param->ToString();
            temp += ", ";
        }

        if (!Parameters.empty()) {
            temp.resize(temp.size() - 2);
        }
        temp += ") {\n" + Body->ToString() + "\n}";
        return temp;
    }
};
