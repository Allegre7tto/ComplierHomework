#include "compliers/semantic.h"
#include <iostream>
#include <sstream>
#include <string>

void analyzeInput(const std::string& input) {
    try {
        std::istringstream iss(input);
        Lexer lexer(iss);
        SemanticAnalyzer analyzer(lexer);
        
        if (analyzer.analyze()) {
            analyzer.printQuadruples();
        } else {
            std::cout << "Error: " << analyzer.getErrorMessage() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // 命令行输入模式
    if (argc > 1) {
        std::string input;
        for (int i = 1; i < argc; ++i) {
            if (i > 1) input += " ";
            input += argv[i];
        }
        analyzeInput(input);
        return 0;
    }
    
    // 交互式模式
    std::string input;
    while (true) {
        std::cout << "> ";
        
        if (!std::getline(std::cin, input)) {
            break;
        }
        
        if (input == "quit" || input == "exit" || input == "q") {
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        analyzeInput(input);
    }
    
    return 0;
} 