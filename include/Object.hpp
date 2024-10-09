#pragma once

#include <fmt/core.h>
#include <fmt/format.h>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

enum class ObjectType {
    INTEGER,
    FLOAT,
    BOOLEAN,
    STRING,
    NULL_OBJ,
    RETURN_VALUE,
    ERROR,
    FUNCTION,
    BUILTIN,
    EXIT,
    INCLUDE,
    ARRAY,
    HASH,
    BREAK,
    ITER,
};

class IObject {
   public:
    virtual ObjectType Type() = 0;
    virtual std::string Inspect() = 0;
    virtual ~IObject() = default;
};

template <>
struct fmt::formatter<IObject> {
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(IObject& obj, FormatContext& ctx) -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", obj.Inspect());
    }
};

template <>
struct fmt::formatter<ObjectType> {
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(ObjectType type, FormatContext& ctx) -> decltype(ctx.out()) {
        std::string typeStr = "";
        switch (type) {
            case ObjectType::INTEGER:
                typeStr = "INTEGER";
                break;
            case ObjectType::FLOAT:
                typeStr = "FLOAT";
                break;
            case ObjectType::BOOLEAN:
                typeStr = "BOOLEAN";
                break;
            case ObjectType::STRING:
                typeStr = "STRING";
                break;
            case ObjectType::NULL_OBJ:
                typeStr = "NULL_OBJ";
                break;
            case ObjectType::RETURN_VALUE:
                typeStr = "RETURN_VALUE";
                break;
            case ObjectType::ERROR:
                typeStr = "ERROR";
                break;
            case ObjectType::FUNCTION:
                typeStr = "FUNCTION";
                break;
            case ObjectType::BUILTIN:
                typeStr = "BUILTIN";
                break;
            case ObjectType::EXIT:
                typeStr = "EXIT";
                break;
            case ObjectType::INCLUDE:
                typeStr = "INCLUDE";
                break;
            case ObjectType::ARRAY:
                typeStr = "ARRAY";
                break;
            case ObjectType::HASH:
                typeStr = "HASH";
                break;
            case ObjectType::BREAK:
                typeStr = "BREAK";
                break;
            case ObjectType::ITER:
                typeStr = "ITER";
                break;
        }
        return fmt::format_to(ctx.out(), "{}", typeStr);
    }
};
// Forward declare HashKey
struct HashKey;

class IHashable {
   public:
    virtual ~IHashable() = default;
    virtual HashKey GetHashKey() = 0;
};

struct ExitObject : public IObject {
    int Value;

    ExitObject(int val) : Value(val) {}
    ObjectType Type() override { return ObjectType::EXIT; }
    std::string Inspect() override { return std::to_string(Value); }
};

struct Integer : public IObject, public IHashable {
    int Value;

    Integer(int val) : Value(val) {}
    ObjectType Type() override { return ObjectType::INTEGER; }
    std::string Inspect() override { return std::to_string(Value); }
    HashKey GetHashKey() override;
};

struct BooleanObj : public IObject, public IHashable {
    bool Value;
    BooleanObj() {}
    BooleanObj(bool val) : Value(val) {}
    ObjectType Type() override { return ObjectType::BOOLEAN; }
    std::string Inspect() override { return std::to_string(Value); }
    HashKey GetHashKey() override;
};

struct Null : public IObject {
    ObjectType Type() override { return ObjectType::NULL_OBJ; }
    std::string Inspect() override { return "null"; }
};

struct ReturnValue : public IObject {
    std::shared_ptr<IObject> Value;

    ReturnValue(std::shared_ptr<IObject> val) : Value(val) {}
    ObjectType Type() override { return ObjectType::RETURN_VALUE; }
    std::string Inspect() override { return Value->Inspect(); }
};

struct BreakObj : public IObject {
    ObjectType Type() override { return ObjectType::BREAK; }
    std::string Inspect() override { return "break"; }
};

struct Error : public IObject {
    std::string Message;
    Error(std::string msg) : Message(msg) {}
    ObjectType Type() override { return ObjectType::BREAK; }
    std::string Inspect() override { return "ERROR: " + Message; }
};

struct StringObj : public IObject, public IHashable {
    std::string Value;
    StringObj(std::string val) : Value(val) {}
    ObjectType Type() override { return ObjectType::STRING; }
    std::string Inspect() override { return Value; }
    HashKey GetHashKey() override;
};

using BuiltinFunction = std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>> params)>;
struct BuiltinObj : public IObject {
    BuiltinFunction Function;

    BuiltinObj(BuiltinFunction func) : Function(func) {}
    ObjectType Type() override { return ObjectType::FUNCTION; }
    std::string Inspect() override { return "builtin obj"; }
};

struct ArrayObject : public IObject {
    std::vector<std::shared_ptr<IObject>> Elements;

    ArrayObject(std::vector<std::shared_ptr<IObject>> e) : Elements(e) {}
    ObjectType Type() override { return ObjectType::ARRAY; }
    std::string Inspect() override {
        std::string temp = "[";

        for (const auto& e : Elements) {
            temp += e->Inspect();
            temp += ", ";
        }

        if (!Elements.empty()) {
            temp.resize(temp.size() - 2);
        }
        temp += "]";
        return temp;
    }
};

// HashKey implementation
struct HashKey {
    ObjectType Type;
    uint64_t Value;

    HashKey(Integer& integer) {
        Value = (uint64_t)integer.Value;
        Type = integer.Type();
    }

    HashKey(BooleanObj& boolean) {
        Value = boolean.Value;
        Type = boolean.Type();
    }

    HashKey(StringObj& string) {
        Value = 0;
        for (const auto& ch : string.Value) {
            Value += ch;
        }
        Type = string.Type();
    }

    bool operator<(const HashKey& other) const { return Value < other.Value; }
};

struct HashPair {
    std::shared_ptr<IObject> Key;
    std::shared_ptr<IObject> Value;
    HashPair(){};
    HashPair(std::shared_ptr<IObject> key, std::shared_ptr<IObject> val)
        : Key(key), Value(val) {}
};

struct Hash : public IObject {
    std::map<HashKey, HashPair> Pairs;

    Hash(std::map<HashKey, HashPair> p) : Pairs(p) {}
    ObjectType Type() override { return ObjectType::HASH; }
    std::string Inspect() override {
        std::string temp = "{";
        for (const auto& [_, kvp] : Pairs) {
            temp += kvp.Key->Inspect();
            temp += ":";
            temp += kvp.Value->Inspect();
            temp += ", ";
        }
        if (!Pairs.empty()) temp.resize(temp.size() - 2);
        temp += "}";
        return temp;
    }
};

struct IterObj : public IObject {
    int Low;
    int High;
    int Steps;
    ObjectType Type() override { return ObjectType::ITER; }
    std::string Inspect() override {
        return "range(" + std::to_string(Low) + ", " + std::to_string(High) +
               ", " + std::to_string(Steps) + ")";
    }
};
