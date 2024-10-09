#include <cstdio>
#include <iostream>
#include <memory>

#include "Enviroment.hpp"
#include "Lexer.hpp"
#include "Object.hpp"
#include "Parser.hpp"
#include "Evaluator.hpp"

int Repl() {
    std::shared_ptr<Env> env = std::make_shared<Env>();
    Evaluator eval;
    std::string line;
    int code = 0;
    while(true) {
        line = "";
        std::cout << ">>> ";
        getline(cin, line);
        if (line != "") {
            Lexer l(line);
            Parser p(l);
            auto program = p.ParseProgram();
            if (!p.Errors.empty()) {
                for (const auto& err : p.Errors) {
                    std::cout << err << std::endl;
                }
                continue;
            }
            auto evaluated = eval.Eval(program, env);
            if (evaluated->Type() == ObjectType::EXIT) {
                code = static_pointer_cast<ExitObject>(evaluated)->Value;
                std::cout << "The program exited with code " << code << std::endl;
                break;
            }
            if (evaluated->Type() != ObjectType::NULL_OBJ) {
                std::cout << evaluated->Inspect() << std::endl;
            }
        }
    }
    return code;
}

int main() {
    return Repl();
}
