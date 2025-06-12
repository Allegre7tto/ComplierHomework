#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"
#include "lexer.h"
#include <string>
#include <vector>

// 四元式结构
struct Quadruple {
    std::string op;      // 操作符
    std::string arg1;    // 第一个操作数
    std::string arg2;    // 第二个操作数
    std::string result;  // 结果

    Quadruple(const std::string& operation, const std::string& operand1, 
              const std::string& operand2, const std::string& res)
        : op(operation), arg1(operand1), arg2(operand2), result(res) {}

    // 格式化输出四元式
    std::string toString() const {
        return op + ", " + arg1 + ", " + arg2 + ", " + result;
    }
};

// 属性结构，用于存储文法符号的属性
struct Attribute {
    std::string place;     // 存放值的变量名
    std::string offset;    // 数组偏移量（用于数组访问）
    std::vector<int> truelist;   // 真出口链表
    std::vector<int> falselist;  // 假出口链表
    std::vector<int> nextlist;   // 下一条指令链表
    int quad;              // 四元式地址（用于标号m）
    
    Attribute() : place(""), offset(""), quad(-1) {}
    Attribute(const std::string& p) : place(p), offset(""), quad(-1) {}
    Attribute(const std::string& p, const std::string& o) : place(p), offset(o), quad(-1) {}
};

// 语义分析器 - 扩展现有Parser功能，添加语义分析
// 注意：这个类重用了Parser的语法分析逻辑，但添加了语义动作
class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(Lexer& lexer);
    
    // 开始语义分析
    bool analyze();
    
    // 打印四元式序列
    void printQuadruples() const;
    
    // 获取错误信息
    const std::string& getErrorMessage() const { return error_message; }
    
    // 获取四元式序列
    const std::vector<Quadruple>& getQuadruples() const { return quadruples; }

private:
    Lexer& lexer;
    Token current_token;
    std::vector<Quadruple> quadruples;  // 四元式序列
    int temp_count;                     // 临时变量计数器
    std::string error_message;
    
    // 工具函数
    std::string newTemp();              // 生成新的临时变量
    void emit(const std::string& op, const std::string& arg1, 
              const std::string& arg2, const std::string& result);  // 生成四元式
    int nextQuad() const;               // 获取下一条四元式的地址
    void advance();                     // 读取下一个token
    bool match(int expected_type);      // 匹配并消耗token
    void setError(const std::string& message);
    
    // 链表操作函数
    std::vector<int> makeList(int quad_addr);
    std::vector<int> merge(const std::vector<int>& list1, const std::vector<int>& list2);
    std::vector<int> merge(const std::vector<int>& list1, const std::vector<int>& list2, 
                          const std::vector<int>& list3);
    void backpatch(const std::vector<int>& list, int quad_addr);
    
    // 递归下降翻译器函数
    // 注意：这些函数实现了与Parser相同的语法，但添加了语义动作
    // 这是为了在语法分析的同时生成中间代码，符合编译器设计的标准做法
    Attribute parseStmts();
    Attribute parseStmt();
    Attribute parseExpr();
    Attribute parseTerm();
    Attribute parseUnary();
    Attribute parseFactor();
    Attribute parseLoc();
    Attribute parseElist(const std::string& array_name);
    Attribute parseBool();
    Attribute parseEquality();
    Attribute parseRel();
    
    // 语义动作函数
    void handleAssignment(const Attribute& loc, const Attribute& expr);
    void handleArrayAccess(Attribute& loc, const Attribute& index);
    void handleBinaryOp(Attribute& result, const Attribute& left, 
                       const std::string& op, const Attribute& right);
};

#endif // SEMANTIC_H 