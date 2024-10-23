#include "Benchmark.hpp"
#include <array>
#include <chrono>
#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Enviroment.hpp"
#include "fmt/core.h"

const size_t NUMBER_OF_RUNS = 25;

void Benchmark(int count) {
	// NOTE: This is a bad way to benchmark, but I like it :)
	// Fib the destroyer of worlds
	std::string fib = "function fib(x) { let a = 0; let b = 1; for (i in range(0, x)) { let c = b; b = a + b; a = c; } return a; } fib("+ std::to_string(count)  + "); ";
	Run(fib, fmt::format("{0}x fib", count));
}

void Run(std::string code, std::string name) {
	double sum = 0;
	for (size_t i = 0; i < NUMBER_OF_RUNS; ++i) {
		//std::cout << fmt::format("starting {0}", name) << std::endl;
		auto start = std::chrono::system_clock::now();
		RunString(code);
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed = end - start;
		//std::cout << fmt::format("{0} at run {1} took: {2} seconds", name, i + 1, elapsed.count()) << std::endl;
		sum += elapsed.count();
	}
	std::cout << fmt::format("average time for {0} = {1}", name, sum / (double)NUMBER_OF_RUNS) << std::endl;
}

void RunString(std::string code) {
	auto pstart = std::chrono::system_clock::now();
	Lexer l(code);
	Parser p(l);
	auto program = p.ParseProgram();
	if (!p.Errors.empty()) {
		for (const auto& err : p.Errors) {
			std::cerr << err << std::endl;
		}
		return;
	}
	auto pend = std::chrono::system_clock::now();
	std::chrono::duration<double> pelapsed = pend - pstart;
	//std::cout << "parsing took: " << pelapsed.count() << " seconds" << std::endl;

	auto estart = std::chrono::system_clock::now();
	std::shared_ptr<Env> env = std::make_shared<Env>();
	env->Set("NOTES", make_shared<NoteObj>());
	env->Set("TIME", make_shared<TimeObj>());
	auto fin = program->Evaluate(env);
	if (fin->Type() == ObjectType::ERROR) {
		std::cerr << fin->Inspect() << std::endl;
	}

	auto eend = std::chrono::system_clock::now();
	std::chrono::duration<double> eelapsed = eend - estart;
	//std::cout << "evaluating took: " << eelapsed.count() << " seconds" << std::endl;
}