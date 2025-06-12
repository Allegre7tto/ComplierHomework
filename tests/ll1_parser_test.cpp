#include "compliers/parser.h"
#include <iostream>
#include <sstream>
#include <string>

void analyzeInput(const std::string& input) {
    std::istringstream iss(input);
    Lexer lexer(iss);
    Parser parser(lexer);
    
    // 设置详细输出，显示LL(1)分析过程
    parser.setVerbose(true);
    
    // 执行语法分析
    ParseResult result = parser.parse();
    
    if (result != ParseResult::SUCCESS) {
        std::cout << "Error: " << parser.getErrorMessage() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // 命令行模式
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