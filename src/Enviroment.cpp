#include "Enviroment.hpp"

#include <memory>

#include "Object.hpp"

HashKey Integer::GetHashKey() { return HashKey(*this); }
HashKey BooleanObj::GetHashKey() { return HashKey(*this); }
HashKey StringObj::GetHashKey() { return HashKey(*this); }

std::shared_ptr<BooleanObj> Env::TRUE = std::make_shared<BooleanObj>(true);
std::shared_ptr<BooleanObj> Env::FALSE = std::make_shared<BooleanObj>(false);
std::shared_ptr<Null> Env::NULLOBJ = std::make_shared<Null>();
std::shared_ptr<BreakObj> Env::BREAK = std::make_shared<BreakObj>();

std::shared_ptr<IObject> Env::Set(std::string name,
                                  std::shared_ptr<IObject> val) {
    if (Outer != nullptr && Outer->Get(name) != nullptr) {
        Outer->Set(name, val);
        return val;
    }
    Store[name] = val;
    return val;
}

std::shared_ptr<IObject> Env::Set(std::string name,
                                  std::shared_ptr<IObject> index,
                                  std::shared_ptr<IObject> assignVal) {
    std::shared_ptr<IObject> obj = Get(name);
    if (!obj) return nullptr;

    if (auto hash = std::dynamic_pointer_cast<Hash>(obj)) {
        if (auto indexHash = std::dynamic_pointer_cast<IHashable>(index)) {
            hash->Pairs[indexHash->GetHashKey()] = HashPair(index, assignVal);
            return assignVal;
        }
    } else if (auto array = std::dynamic_pointer_cast<ArrayObject>(obj)) {
        if (auto indexInt = std::dynamic_pointer_cast<Integer>(index)) {
            if (0 < indexInt->Value &&
                indexInt->Value < array->Elements.size()) {
                array->Elements[indexInt->Value] = assignVal;
                return assignVal;
            }
        }
    }
    return Env::NULLOBJ;
}

std::shared_ptr<IObject> Env::Get(std::string name) {
    if (Store.find(name) == Store.end()) {
        if (Outer != nullptr) {
            return Outer->Get(name);
        } else
            return nullptr;
    }

    return Store[name];
}
std::shared_ptr<IObject> Env::GetHashObject(std::string name,
                                            std::shared_ptr<IHashable> index) {
    if (Store.find(name) == Store.end()) {
        if (Outer != nullptr) {
            return Outer->GetHashObject(name, index);
        } else
            return nullptr;
    }

    auto hash = std::dynamic_pointer_cast<Hash>(Store[name]);
    if (!hash) return nullptr;

    if (hash->Pairs.find(index->GetHashKey()) != hash->Pairs.end()) {
        return hash->Pairs[index->GetHashKey()].Value;
    }
    return nullptr;
}

std::shared_ptr<IObject> Env::GetArrayObject(std::string name,
                                             std::shared_ptr<Integer> index) {
    if (Store.find(name) == Store.end()) {
        if (Outer != nullptr) {
            return Outer->GetArrayObject(name, index);
        } else
            return nullptr;
    }

    auto array = std::dynamic_pointer_cast<ArrayObject>(Store[name]);
    if (!array) return nullptr;

    if (0 < index->Value && index->Value < array->Elements.size()) {
        return array->Elements[index->Value];
    }
    return nullptr;
}

std::shared_ptr<IObject> Env::AddEnv(std::shared_ptr<Env> env) {
    for (const auto &[key, value] : env->Store) {
        if (value->Type() == ObjectType::ERROR) {
            return value;
        }
        Store[key] = value;
    }
    return Env::NULLOBJ;
}

std::shared_ptr<Env> NewEnclosedEnviroment(std::shared_ptr<Env> outer) {
    std::shared_ptr<Env> env = std::make_shared<Env>();
    env->Outer = outer;
    return env;
}
