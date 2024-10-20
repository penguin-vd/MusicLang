#pragma once
#include <map>
#include <string>

#include "Object.hpp"

using namespace std;
class Env {
   public:
    static shared_ptr<BooleanObj> TRUE;
    static shared_ptr<BooleanObj> FALSE;
    static shared_ptr<Null> NULLOBJ;
    static shared_ptr<BreakObj> BREAK;

    Env() {}
    Env(map<string, shared_ptr<IObject>> fields) : Store(fields) {}

    map<string, shared_ptr<IObject>> Store;
    shared_ptr<Env> Outer;

    shared_ptr<IObject> Get(string name);
    shared_ptr<IObject> Set(string name, shared_ptr<IObject> val);
    shared_ptr<IObject> Set(string name, shared_ptr<IObject> index, shared_ptr<IObject> assignVal);

    shared_ptr<IObject> GetHashObject(string name, shared_ptr<IHashable> index);
    shared_ptr<IObject> GetArrayObject(string name, shared_ptr<Integer> index);

    shared_ptr<IObject> AddEnv(shared_ptr<Env> env);
    std::shared_ptr<IObject> ExtendEnv(std::map<std::string, std::shared_ptr<IObject>> fields);
};

shared_ptr<Env> NewEnclosedEnviroment(shared_ptr<Env> outer);
