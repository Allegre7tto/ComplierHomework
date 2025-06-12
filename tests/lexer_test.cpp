#include "compliers/lexer.h"
#include <iostream>

// 词法分析器测试程序
int main() {
    std::cout << "词法分析器启动" << std::endl;
    
    Lexer lexer(std::cin);

    Token token(0);
    do {
        token = lexer.getNextToken();
        if (token.type != 0 && token.type != -1) {
            std::cout << token << std::endl;
        } else if (token.type == -1) {
            // 遇到错误，但继续分析以发现更多错误
            std::cerr << "继续分析..." << std::endl;
        }
    } while (token.type != 0); // 0 表示文件结束

    std::cout << "词法分析完成。" << std::endl;
    return 0;
} 