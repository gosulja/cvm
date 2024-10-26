#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "compiler.hpp"
#include "cvm.hpp"

void execute_code(const std::string& code, bool debug, bool show_last) {
    try {
        Lexer lexer(code);
        auto tokens = lexer.generate();
        Compiler compiler(tokens);
        auto bytecode = compiler.compile();
        CVM vm(bytecode, debug);
        vm.execute();
        
        if (show_last) print("result: " + vm.getResultAsString());
    } catch (const std::exception& e) {
        print("error: " + std::string(e.what()));
    }
}

void repl_mode(bool debug, bool show_last) {
    print("CVM REPL v0.1 (type 'exit();' to stop, 'help();' for commands)");
    
    while (true) {
        std::string input;
        std::cout << ">>> ";
        std::getline(std::cin, input);

        if (input == "exit();") {
            break;
        } else if (input == "help();") {
            print("Available commands:");
            print("  exit();  - Exit the REPL");
            print("  help();  - Show this help message");
            print("  clear(); - Clear the screen");
            continue;
        } else if (input == "clear();") {
            #ifdef _WIN32
                system("cls");
            #else
                system("clear");
            #endif
            continue;
        } else if (input.empty()) {
            continue;
        }

        execute_code(input, debug, show_last);
    }
}

void file_mode(const std::string& filename, bool debug, bool show_last) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        print("error: could not open file '" + filename + "'");
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    print("executing file: " + filename);
    execute_code(content, debug, show_last);
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [-d] [filename]\n";
    std::cout << "  If no filename is provided, starts in REPL mode\n";
}

int main(int argc, char* argv[]) {
    bool debug_mode = false;
    bool show_last = false;

    if (argc > 3) {
        print_usage(argv[0]);
        return 1;
    }

    try {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-h" || arg == "--help") {
                print_usage(argv[0]);
                return 0;
            }
            else if (arg == "-d") {
                debug_mode = true;
            }
            else if (arg == "-s") {
                show_last = true;
            }
            else {
                if (i != argc - 1) {
                    print_usage(argv[0]);
                    return 1;
                }
                file_mode(arg, debug_mode, show_last);
                return 0;
            }
        }

        repl_mode(debug_mode, show_last);
    } catch (const std::exception& e) {
        print("fatal error: " + std::string(e.what()));
        return 1;
    }

    return 0;
}
