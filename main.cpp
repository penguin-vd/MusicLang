#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "Enviroment.hpp"
#include "Lexer.hpp"
#include "Object.hpp"
#include "Parser.hpp"
#include "Benchmark.hpp"

const int FILE_ERROR = 144;

int Repl() {
    std::shared_ptr<Env> env = std::make_shared<Env>();
    env->Set("NOTES", make_shared<NoteObj>());
    env->Set("TIME", make_shared<TimeObj>());
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
        return FILE_ERROR;
    }

    std::string fileContent;
    fileContent.assign((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    if (file.fail() && !file.eof()) {
        std::cerr << "Error: Failed to read file '" << fileName << "'" << std::endl;
        return FILE_ERROR;
    }
    file.close();
    
    if (file.fail()) {
        std::cerr << "Error: Failed to close file '" << fileName << "'" << std::endl;
        return FILE_ERROR;
    }
    
    Lexer l(fileContent);
    Parser p(l);
    auto program = p.ParseProgram();
    for (const auto& err : p.Errors) {
        std::cerr << err << std::endl;
        return 1;
    }

    std::shared_ptr<Env> env = std::make_shared<Env>();
    env->Set("NOTES", make_shared<NoteObj>());
    env->Set("TIME", make_shared<TimeObj>());
    auto fin = program->Evaluate(env);

    if (fin->Type() == ObjectType::EXIT) {
        return static_pointer_cast<ExitObject>(fin)->Value;
    }

    if (fin->Type() == ObjectType::ERROR) {
        std::cout << fin->Inspect() << std::endl;
    }
    
    return 0;
}

void PrintHelp() {
    std::cout << "Usage: mlang [options] [file]\n";
    std::cout << "Options:\n";
    std::cout << "  --repl           Start the REPL\n";
    std::cout << "  --help           Show this help message\n";
    std::cout << "If no options are provided, the program will attempt to run the specified file.\n";
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    std::srand(std::time(0));
    
    if (args.empty() || args[0] == "--help") {
        return 0;
    }

    if (args[0] == "--repl") {
        return Repl();
    }

    if (args[0] == "--benchmark") {
        Benchmark(1000000);
        return 0;
    }

    int code = RunFile(args[0]);
    if (code == FILE_ERROR) {
        PrintHelp();
    }

    return code;
}
