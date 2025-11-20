// src/main.cpp
#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include "../include/ast.hpp"
#include "../include/tac.hpp"

// Оголошення, які надає Flex/Bison
extern int yyparse(void);
extern int yylex_destroy(void);
extern FILE* yyin;

// Глобальний корінь AST, встановлюється в parser.y
extern AST::Block* g_root;

int main(int argc, char* argv[]) {
    bool emitDot = false;
    bool emitTac = false;
    std::string inputFile;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--ast") emitDot = true;
        else if (a == "--tac") emitTac = true;
        else inputFile = a;
    }

    if (!inputFile.empty()) {
        FILE* f = nullptr;
#ifdef _MSC_VER
        if (fopen_s(&f, inputFile.c_str(), "rb") != 0 || !f) {
            std::cerr << "Cannot open input file: " << inputFile << "\n";
            return 1;
        }
#else
        f = std::fopen(inputFile.c_str(), "rb");
        if (!f) {
            std::cerr << "Cannot open input file: " << inputFile << "\n";
            return 1;
        }
#endif
        yyin = f;
        if (!yyin) {
            std::cerr << "Cannot open input file: " << inputFile << "\n";
            return 1;
        }
    }
    else {
        yyin = stdin;
    }

    int res = yyparse();
    yylex_destroy();

    if (res != 0 || g_root == nullptr) {
        std::cerr << "Parsing failed.\n";
        return 2;
    }

    std::unique_ptr<AST::Block> program(g_root); // керуємо життям

    if (emitDot) {
        std::ofstream out("ast.dot");
        if (!out) {
            std::cerr << "Cannot open ast.dot for writing\n";
            return 3;
        }
        AST::writeDOT(*program, out);
        std::cout << "AST written to ast.dot\n";
        return 0;
    }
    else if (emitTac) {                                // ДОДАЙ
        std::ofstream out("tac.txt");
        if (!out) { std::cerr << "Cannot open tac.txt\n"; return 3; }
        TAC::Emitter em;
        em.gen(program.get());
        em.write(out);
        std::cout << "TAC written to tac.txt\n";
        return 0;
    }
    else {
        AST::Context ctx;
        try {
            program->exec(ctx);
        }
        catch (const std::exception& ex) {
            std::cerr << "Runtime error: " << ex.what() << "\n";
            return 4;
        }
    }

    return 0;
}
