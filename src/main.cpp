#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>

#include "Enviroment.hpp"
#include "Lexer.hpp"
#include "Object.hpp"
#include "Parser.hpp"

int Repl() {
    std::shared_ptr<Env> env = std::make_shared<Env>();
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
            auto evaluated = program->Evaluate(env);
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

int RunFile(std::string fileName) {
    std::ifstream file(fileName);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << fileName << "' - " << std::strerror(errno) << std::endl;
        return 1;
    }

    std::string fileContent;
    fileContent.assign((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    if (file.fail() && !file.eof()) {
        std::cerr << "Error: Failed to read file '" << fileName << "'" << std::endl;
        return 1;
    }
    file.close();
    
    if (file.fail()) {
        std::cerr << "Error: Failed to close file '" << fileName << "'" << std::endl;
        return 1;
    }
    
    Lexer l(fileContent);
    Parser p(l);
    auto program = p.ParseProgram();
    for (const auto& err : p.Errors) {
        std::cerr << err << std::endl;
        return 1;
    }

    std::shared_ptr<Env> env = std::make_shared<Env>();
    auto fin = program->Evaluate(env);

    if (fin->Type() == ObjectType::EXIT) {
        return static_pointer_cast<ExitObject>(fin)->Value;
    }

    if (fin->Type() == ObjectType::ERROR) {
        std:cout << fin->Inspect() << std::endl;
    }

    return 0;
}

int main() {
    // TODO: Add args parsing here
    
    return Repl();
}
