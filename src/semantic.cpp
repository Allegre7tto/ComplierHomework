#include "compliers/semantic.h"

// 注意：这个语义分析器实现了递归下降翻译器
// 虽然看起来与Parser类似，但这是编译器设计的标准做法：
// 1. Parser负责纯语法分析，验证输入是否符合语法
// 2. SemanticAnalyzer在语法分析的同时执行语义动作，生成中间代码
// 3. 两者的语法规则相同，但目标不同：Parser验证语法，SemanticAnalyzer生成代码
// 4. 这种分离符合编译器前端的模块化设计原则

SemanticAnalyzer::SemanticAnalyzer(Lexer& lexer) 
    : lexer(lexer), current_token(0), temp_count(0) {
    advance(); // 读取第一个token
}

bool SemanticAnalyzer::analyze() {
    error_message.clear();
    quadruples.clear();
    temp_count = 0;
    
    try {
        Attribute stmts_attr = parseStmts();
        
        // 回填最后的nextlist
        backpatch(stmts_attr.nextlist, nextQuad());
        
        // 检查是否有剩余输入
        if (current_token.type != 0) { // 0为EOF
            setError("输入未完全解析完成");
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        setError(e.what());
        return false;
    }
}

void SemanticAnalyzer::printQuadruples() const {
    for (size_t i = 0; i < quadruples.size(); ++i) {
        std::cout << i << ": " << quadruples[i].toString() << std::endl;
    }
}

std::string SemanticAnalyzer::newTemp() {
    return "t" + std::to_string(++temp_count);
}

void SemanticAnalyzer::emit(const std::string& op, const std::string& arg1, 
                           const std::string& arg2, const std::string& result) {
    quadruples.emplace_back(op, arg1, arg2, result);
}

int SemanticAnalyzer::nextQuad() const {
    return static_cast<int>(quadruples.size());
}

void SemanticAnalyzer::advance() {
    current_token = lexer.getNextToken();
}

bool SemanticAnalyzer::match(int expected_type) {
    if (current_token.type == expected_type) {
        advance();
        return true;
    }
    return false;
}

void SemanticAnalyzer::setError(const std::string& message) {
    error_message = "语义分析错误: " + message + 
                   " (第" + std::to_string(current_token.line) + "行)";
}

// 链表操作函数实现
std::vector<int> SemanticAnalyzer::makeList(int quad_addr) {
    return std::vector<int>{quad_addr};
}

std::vector<int> SemanticAnalyzer::merge(const std::vector<int>& list1, const std::vector<int>& list2) {
    std::vector<int> result = list1;
    result.insert(result.end(), list2.begin(), list2.end());
    return result;
}

std::vector<int> SemanticAnalyzer::merge(const std::vector<int>& list1, const std::vector<int>& list2, 
                                        const std::vector<int>& list3) {
    std::vector<int> result = merge(list1, list2);
    result.insert(result.end(), list3.begin(), list3.end());
    return result;
}

void SemanticAnalyzer::backpatch(const std::vector<int>& list, int quad_addr) {
    for (int addr : list) {
        if (addr >= 0 && addr < static_cast<int>(quadruples.size())) {
            quadruples[addr].result = std::to_string(quad_addr);
        }
    }
}

// 按照文档翻译模式实现递归下降翻译器

Attribute SemanticAnalyzer::parseStmts() {
    // stmts → stmt rest0
    Attribute stmt_attr = parseStmt();
    
    // rest0 的处理
    Attribute stmts_attr;
    stmts_attr.nextlist = stmt_attr.nextlist;
    
    while (current_token.type != 0 && current_token.type != 87) { // 不是EOF和}
        // m 标号
        int m_quad = nextQuad();
        
        // 回填前一个语句的nextlist
        backpatch(stmts_attr.nextlist, m_quad);
        
        Attribute next_stmt = parseStmt();
        stmts_attr.nextlist = next_stmt.nextlist;
    }
    
    return stmts_attr;
}

Attribute SemanticAnalyzer::parseStmt() {
    Attribute stmt_attr;
    
    // stmt → if(bool) m1 stmt1 n else m2 stmt2
    if (current_token.type == 17) { // if
        advance();
        
        if (!match(81)) { // (
            throw std::runtime_error("期望 '('");
        }
        
        Attribute bool_attr = parseBool();
        
        if (!match(82)) { // )
            throw std::runtime_error("期望 ')'");
        }
        
        // m1 标号
        int m1_quad = nextQuad();
        
        Attribute stmt1 = parseStmt();
        
        // n 标号 - 生成无条件跳转
        Attribute n_attr;
        n_attr.nextlist = makeList(nextQuad());
        emit("j", "-", "-", "-");
        
        if (!match(15)) { // else
            throw std::runtime_error("期望 'else'");
        }
        
        // m2 标号
        int m2_quad = nextQuad();
        
        Attribute stmt2 = parseStmt();
        
        // 语义动作
        backpatch(bool_attr.truelist, m1_quad);
        backpatch(bool_attr.falselist, m2_quad);
        stmt_attr.nextlist = merge(stmt1.nextlist, n_attr.nextlist, stmt2.nextlist);
        
        return stmt_attr;
    }
    
    // stmt → while(m1 bool) m2 stmt1
    if (current_token.type == 20) { // while
        advance();
        
        if (!match(81)) { // (
            throw std::runtime_error("期望 '('");
        }
        
        // m1 标号
        int m1_quad = nextQuad();
        
        Attribute bool_attr = parseBool();
        
        if (!match(82)) { // )
            throw std::runtime_error("期望 ')'");
        }
        
        // m2 标号
        int m2_quad = nextQuad();
        
        Attribute stmt1 = parseStmt();
        
        // 语义动作
        backpatch(stmt1.nextlist, m1_quad);
        backpatch(bool_attr.truelist, m2_quad);
        stmt_attr.nextlist = bool_attr.falselist;
        emit("j", "-", "-", std::to_string(m1_quad));
        
        return stmt_attr;
    }
    
    // stmt → loc = expr ;
    if (current_token.type == 111) { // 标识符
        Attribute loc = parseLoc();
        if (loc.place.empty()) {
            throw std::runtime_error("解析位置表达式失败");
        }
        
        if (!match(46)) { // =
            throw std::runtime_error("期望 '='");
        }
        
        Attribute expr = parseExpr();
        if (expr.place.empty()) {
            throw std::runtime_error("解析表达式失败");
        }
        
        if (!match(84)) { // ;
            throw std::runtime_error("期望 ';'");
        }
        
        // 语义动作：生成赋值四元式
        handleAssignment(loc, expr);
        
        // stmt.nextlist = makelist() (空链表)
        stmt_attr.nextlist = std::vector<int>();
        
        return stmt_attr;
    }
    
    // 处理复合语句 { stmts }
    if (current_token.type == 86) { // {
        advance();
        Attribute stmts = parseStmts();
        if (!match(87)) { // }
            throw std::runtime_error("期望 '}'");
        }
        return stmts;
    }
    
    throw std::runtime_error("不支持的语句类型");
}

// 布尔表达式解析
Attribute SemanticAnalyzer::parseBool() {
    // bool → equality
    return parseEquality();
}

Attribute SemanticAnalyzer::parseEquality() {
    // equality → rel rest4
    Attribute rel = parseRel();
    
    Attribute equality_attr = rel;
    
    // rest4 → ==rel rest4 | !=rel rest4 | ε
    while (current_token.type == 51 || current_token.type == 52) { // == or !=
        std::string op = (current_token.type == 51) ? "j==" : "j!=";
        advance();
        
        Attribute next_rel = parseRel();
        
        // 生成比较跳转四元式
        Attribute new_attr;
        new_attr.truelist = makeList(nextQuad());
        new_attr.falselist = makeList(nextQuad() + 1);
        emit(op, equality_attr.place, next_rel.place, "-");
        emit("j", "-", "-", "-");
        
        equality_attr = new_attr;
    }
    
    return equality_attr;
}

Attribute SemanticAnalyzer::parseRel() {
    // rel → expr rop_expr
    Attribute expr = parseExpr();
    
    // rop_expr → <expr | <=expr | >expr | >=expr | ε
    if (current_token.type >= 47 && current_token.type <= 50) { // >, <, >=, <=
        std::string op;
        switch (current_token.type) {
            case 47: op = "j>"; break;  // >
            case 49: op = "j<"; break;  // <
            case 48: op = "j>="; break; // >=
            case 50: op = "j<="; break; // <=
        }
        advance();
        
        Attribute next_expr = parseExpr();
        
        // 生成关系跳转四元式
        Attribute rel_attr;
        rel_attr.truelist = makeList(nextQuad());
        rel_attr.falselist = makeList(nextQuad() + 1);
        emit(op, expr.place, next_expr.place, "-");
        emit("j", "-", "-", "-");
        
        return rel_attr;
    } else {
        // rop_expr → ε (布尔变量)
        Attribute rel_attr;
        rel_attr.place = expr.place;
        rel_attr.truelist = makeList(nextQuad());
        rel_attr.falselist = makeList(nextQuad() + 1);
        emit("jnz", expr.place, "-", "-");
        emit("j", "-", "-", "-");
        
        return rel_attr;
    }
}

Attribute SemanticAnalyzer::parseExpr() {
    // expr → term rest5
    Attribute term = parseTerm();
    if (term.place.empty()) {
        throw std::runtime_error("解析term失败");
    }
    
    Attribute result = term;
    
    // rest5 → +term rest5 | -term rest5 | ε
    while (current_token.type == 41 || current_token.type == 42) { // + or -
        std::string op = (current_token.type == 41) ? "+" : "-";
        advance();
        
        Attribute next_term = parseTerm();
        if (next_term.place.empty()) {
            throw std::runtime_error("解析term失败");
        }
        
        // 语义动作：生成二元运算四元式
        handleBinaryOp(result, result, op, next_term);
    }
    
    return result;
}

Attribute SemanticAnalyzer::parseTerm() {
    // term → unary rest6
    Attribute unary = parseUnary();
    if (unary.place.empty()) {
        throw std::runtime_error("解析unary失败");
    }
    
    Attribute result = unary;
    
    // rest6 → *unary rest6 | /unary rest6 | ε
    while (current_token.type == 43 || current_token.type == 44) { // * or /
        std::string op = (current_token.type == 43) ? "*" : "/";
        advance();
        
        Attribute next_unary = parseUnary();
        if (next_unary.place.empty()) {
            throw std::runtime_error("解析unary失败");
        }
        
        // 语义动作：生成二元运算四元式
        handleBinaryOp(result, result, op, next_unary);
    }
    
    return result;
}

Attribute SemanticAnalyzer::parseUnary() {
    // unary → factor
    return parseFactor();
}

Attribute SemanticAnalyzer::parseFactor() {
    // factor → (expr) | loc | num
    
    if (current_token.type == 81) { // (
        advance();
        Attribute expr = parseExpr();
        if (expr.place.empty()) {
            throw std::runtime_error("解析表达式失败");
        }
        
        if (!match(82)) { // )
            throw std::runtime_error("期望 ')'");
        }
        
        return expr;
    }
    else if (current_token.type == 111) { // 标识符
        Attribute loc = parseLoc();
        
        // 如果是数组访问，需要生成数组读取四元式
        if (!loc.offset.empty()) {
            std::string temp = newTemp();
            emit("=[]", loc.place + "[" + loc.offset + "]", "-", temp);
            loc.place = temp;
            loc.offset = "";
        }
        
        return loc;
    }
    else if (current_token.type == 100) { // 数字常量
        std::string value = current_token.value;
        advance();
        return Attribute(value);
    }
    
    throw std::runtime_error("期望表达式");
}

Attribute SemanticAnalyzer::parseLoc() {
    // loc → id resta
    if (current_token.type != 111) {
        throw std::runtime_error("期望标识符");
    }
    
    std::string id = current_token.value;
    advance();
    
    Attribute loc(id);
    
    // resta → [elist] | ε
    if (current_token.type == 88) { // [
        advance();
        
        // 解析elist (支持多维数组)
        Attribute elist = parseElist(id);
        
        if (!match(89)) { // ]
            throw std::runtime_error("期望 ']'");
        }
        
        // 按照文档翻译模式处理数组访问
        std::string temp1 = newTemp();
        emit("-", elist.place, "C", temp1);  // 数组基址减去常量C
        
        std::string temp2 = newTemp();
        emit("*", elist.offset, "w", temp2);  // 偏移量乘以元素大小w
        
        loc.place = temp1;
        loc.offset = temp2;
    }
    
    return loc;
}

// 新增：解析数组下标列表
Attribute SemanticAnalyzer::parseElist(const std::string& array_name) {
    // elist → expr rest1
    Attribute expr = parseExpr();
    
    Attribute elist;
    elist.place = array_name;
    
    // rest1 → , expr rest1 | ε
    if (current_token.type == 90) { // ,
        // 多维数组处理
        std::string temp = newTemp();
        emit("*", expr.place, "n2", temp);  // 第一维乘以第二维大小
        
        advance(); // 消耗 ,
        
        Attribute next_expr = parseExpr();
        emit("+", temp, next_expr.place, temp);
        
        elist.offset = temp;
        
        // 继续处理更多维度
        while (current_token.type == 90) { // ,
            advance();
            std::string new_temp = newTemp();
            Attribute dim_expr = parseExpr();
            emit("*", elist.offset, "n" + std::to_string(3), new_temp); // 假设下一维大小
            emit("+", new_temp, dim_expr.place, new_temp);
            elist.offset = new_temp;
        }
    } else {
        // 一维数组
        elist.offset = expr.place;
    }
    
    return elist;
}

// 语义动作函数实现

void SemanticAnalyzer::handleAssignment(const Attribute& loc, const Attribute& expr) {
    if (loc.offset.empty()) {
        // 简单变量赋值: loc.place = expr.place
        emit("=", expr.place, "-", loc.place);
    } else {
        // 数组元素赋值: loc.place[loc.offset] = expr.place
        emit("[]=", expr.place, "-", loc.place + "[" + loc.offset + "]");
    }
}

void SemanticAnalyzer::handleArrayAccess(Attribute& loc, const Attribute& index) {
    // 按照文档中的翻译模式处理数组访问
    loc.offset = index.place;
}

void SemanticAnalyzer::handleBinaryOp(Attribute& result, const Attribute& left, 
                                     const std::string& op, const Attribute& right) {
    std::string temp = newTemp();
    emit(op, left.place, right.place, temp);
    result.place = temp;
} 