#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <string>
#include <stack>
#include <map>
#include <set>
#include <vector>
#include <functional>

enum class ParseResult {
    SUCCESS,        // 解析成功
    SYNTAX_ERROR,   // 语法错误
    INCOMPLETE_INPUT // 输入不完整
};

enum class NonTerminal {
    STMTS, REST0, STMT, LOC, RESTA, ELIST, REST1, 
    BOOL_EXPR, EQUALITY, REST4, REL, ROP_EXPR, 
    EXPR, REST5, TERM, REST6, UNARY, FACTOR
};

enum class SymbolType {
    TERMINAL,      // 终结符
    NON_TERMINAL,  // 非终结符
    EPSILON        // 空串
};

struct Symbol {
    SymbolType type;
    int token_type;
    NonTerminal non_terminal;
    
    Symbol(int tt) : type(SymbolType::TERMINAL), token_type(tt), non_terminal(NonTerminal::STMTS) {}
    Symbol(NonTerminal nt) : type(SymbolType::NON_TERMINAL), token_type(0), non_terminal(nt) {}
    Symbol() : type(SymbolType::EPSILON), token_type(0), non_terminal(NonTerminal::STMTS) {} // epsilon
    
    bool operator==(const Symbol& other) const {
        if (type != other.type) return false;
        if (type == SymbolType::TERMINAL) return token_type == other.token_type;
        if (type == SymbolType::NON_TERMINAL) return non_terminal == other.non_terminal;
        return true; // both epsilon
    }
    
    bool operator<(const Symbol& other) const {
        if (type != other.type) return type < other.type;
        if (type == SymbolType::TERMINAL) return token_type < other.token_type;
        if (type == SymbolType::NON_TERMINAL) return non_terminal < other.non_terminal;
        return false; // both epsilon
    }
};

struct Production {
    NonTerminal left;                
    std::vector<Symbol> right;      
    std::string description;         
    
    Production(NonTerminal l, std::vector<Symbol> r, std::string desc) 
        : left(l), right(r), description(desc) {}
};

struct TableEntry {
    bool has_production;
    int production_index;
    
    TableEntry() : has_production(false), production_index(-1) {}
    TableEntry(int idx) : has_production(true), production_index(idx) {}
};

// 语义动作回调函数类型
using SemanticActionCallback = std::function<void(int production_index, const std::vector<Token>& matched_tokens)>;

class Parser {
public:
    explicit Parser(Lexer& lexer);
    
    ~Parser() = default;
    
    ParseResult parse();
    
    // 带语义动作的解析
    ParseResult parseWithSemantics(SemanticActionCallback callback);
    
    void setVerbose(bool verbose);
    
    const std::string& getErrorMessage() const;
    
    // 获取当前token（供语义分析器使用）
    Token getCurrentToken() const { return current_token; }
    
    // 获取产生式信息（供语义分析器使用）
    const std::vector<Production>& getProductions() const { return productions; }

private:
    Lexer& lexer;
    
    Token current_token;
    
    int step_counter;
    
    bool verbose;
    
    std::string error_message;
    
    std::vector<Production> productions;              // 产生式集合
    std::map<std::pair<NonTerminal, int>, TableEntry> parse_table; // LL(1)预测分析表
    std::map<NonTerminal, std::set<int>> first_sets; // FIRST集合
    std::map<NonTerminal, std::set<int>> follow_sets; // FOLLOW集合
    std::stack<Symbol> parse_stack;                   // 分析栈
    
    // 语义动作相关
    SemanticActionCallback semantic_callback;
    std::vector<Token> matched_tokens;                // 匹配的token序列
    
    void initializeGrammar();
    void computeFirstSets(); 
    void computeFollowSets();
    void buildParseTable(); 
    
    std::set<int> computeFirstOfString(const std::vector<Symbol>& symbols); 
    void advance();                                    
    bool isEOF() const;                               
    void setError(const std::string& message);        
    
    void printProduction(int production_index);      
    void printStackAndInput();                       
    
    std::string symbolToString(const Symbol& symbol) const;
    std::string nonTerminalToString(NonTerminal nt) const;
    std::string tokenTypeToString(int token_type) const;
    
    // 内部解析函数
    ParseResult parseInternal(bool with_semantics = false);
};

std::string parseResultToString(ParseResult result);

#endif // PARSER_H 