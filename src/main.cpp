#include "compliers/parser.h"
#include "compliers/semantic.h"
#include <iostream>
#include <string>
#include <sstream>

// 检查是否是终端输入的头文件
#ifdef _WIN32
    #include <io.h>
    #define isatty _isatty
    #define STDIN_FILENO 0
#else
    #include <unistd.h>
#endif

// 测试语法分析
void testSyntaxAnalysis(const std::string& input) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "语法分析测试 - 输入: " << input << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    std::istringstream iss(input);
    Lexer lexer(iss);
    Parser parser(lexer);
    
    ParseResult result = parser.parse();
    
    if (result != ParseResult::SUCCESS) {
        std::cout << "语法分析结果: " << parseResultToString(result) << std::endl;
        if (!parser.getErrorMessage().empty()) {
            std::cout << "错误信息: " << parser.getErrorMessage() << std::endl;
        }
    }
}

// 测试语义分析和中间代码生成
void testSemanticAnalysis(const std::string& input) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "语义分析与中间代码生成测试 - 输入: " << input << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    std::istringstream iss(input);
    Lexer lexer(iss);
    SemanticAnalyzer analyzer(lexer);
    
    if (analyzer.analyze()) {
        analyzer.printQuadruples();
    } else {
        std::cout << "✗ " << analyzer.getErrorMessage() << std::endl;
    }
}

// 测试函数，同时进行语法分析和语义分析
void testExpression(const std::string& input) {
    // 先进行语法分析
    testSyntaxAnalysis(input);
    
    // 再进行语义分析
    testSemanticAnalysis(input);
}

void runInteractiveMode() {
    std::cout << "交互模式：请输入赋值表达式（输入 'quit' 退出）:" << std::endl;
    std::cout << "支持的表达式格式: 变量 = 算术表达式;" << std::endl;
    std::cout << "例如: a = 6 / b + 5 * c - d;" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    std::string input;
    while (true) {
        std::cout << "\n请输入表达式: ";
        if (!std::getline(std::cin, input)) {
            break; // EOF
        }
        
        if (input == "quit" || input == "exit") {
            break;
        }
        
        if (!input.empty()) {
            testExpression(input);
        }
    }
}

void runBatchMode() {
    std::string input;
    if (!std::getline(std::cin, input)) {
        std::cerr << "错误：无法读取输入" << std::endl;
        return;
    }
    
    if (!input.empty()) {
        testExpression(input);
    }
}

void runTestCases() {
    std::cout << "运行实验文档中的测试用例:" << std::endl;
    
    // 测试用例1：实验文档中的示例
    std::string test1 = "a=6/b+5*c-d;";
    std::cout << "\n测试用例1: " << test1 << std::endl;
    testSemanticAnalysis(test1);
    
    // 测试用例2：更复杂的表达式
    std::string test2 = "x=(a+b)*(c-d);";
    std::cout << "\n测试用例2: " << test2 << std::endl;
    testSemanticAnalysis(test2);
    
    // 测试用例3：多个语句
    std::string test3 = "a=1; b=a+2; c=a*b;";
    std::cout << "\n测试用例3: " << test3 << std::endl;
    testSemanticAnalysis(test3);
}

int main(int argc, char* argv[]) {
    
    // 检查命令行参数
    bool interactive = false;
    bool run_tests = false;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-i" || arg == "--interactive") {
            interactive = true;
        } else if (arg == "-t" || arg == "--test") {
            run_tests = true;
        }
    }
    
    // 显示程序信息
    std::cout << "实验4: 语义分析和中间代码生成器" << std::endl;
    std::cout << "支持赋值表达式的递归下降翻译器" << std::endl;
    
    // 根据模式运行
    if (run_tests) {
        runTestCases();
    } else if (interactive) {
        runInteractiveMode();
    } else {
        // 检查是否有来自管道的输入
        if (!isatty(STDIN_FILENO)) {
            runBatchMode();
        } else {
            // 如果没有指定模式且没有管道输入，显示帮助
            std::cout << "\n使用方法:" << std::endl;
            std::cout << "  " << argv[0] << " -i       # 交互模式" << std::endl;
            std::cout << "  " << argv[0] << " -t       # 运行测试用例" << std::endl;
            std::cout << "  echo 'a=1+2;' | " << argv[0] << "  # 管道输入" << std::endl;
            
            // 默认运行测试用例
            std::cout << "\n默认运行测试用例:" << std::endl;
            runTestCases();
        }
    }
    
    std::cout << "\n程序结束。" << std::endl;
    return 0;
} 