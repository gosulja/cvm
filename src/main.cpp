#include "lexer.hpp"
#include "compiler.hpp"
#include "cvm.hpp"

int main() {
    std::string source;
    std::cout << "Expression: ";
    std::cin >> source;
    std::cout << "\n";

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Compiler compiler;
    auto bytecode = compiler.compile(tokens);

    CVM vm;

    vm.execute(bytecode);

    std::cout << "Result: " << std::to_string(vm.result()) <<
        std::endl;

    return 0;
}
