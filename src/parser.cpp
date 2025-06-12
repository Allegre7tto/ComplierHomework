#include "compliers/parser.h"

static const int EPSILON_TOKEN = -1;

Parser::Parser(Lexer &lexer)
        : lexer(lexer), current_token(0), step_counter(1), verbose(true) {

    current_token = lexer.getNextToken();

    initializeGrammar();
    computeFirstSets();
    computeFollowSets();
    buildParseTable();
}

ParseResult Parser::parse() {
    return parseInternal(false);
}

ParseResult Parser::parseWithSemantics(SemanticActionCallback callback) {
    semantic_callback = callback;
    return parseInternal(true);
}

ParseResult Parser::parseInternal(bool with_semantics) {
    error_message.clear();
    step_counter = 1;
    matched_tokens.clear();

    while (!parse_stack.empty()) parse_stack.pop();
    parse_stack.push(Symbol(0)); // EOF 终结符
    parse_stack.push(Symbol(NonTerminal::STMTS)); // 开始符

    while (!parse_stack.empty()) {
        Symbol top = parse_stack.top();

        if (top.type == SymbolType::EPSILON) {
            parse_stack.pop();
            continue;
        }

        // 终结符
        if (top.type == SymbolType::TERMINAL) {
            if (top.token_type == current_token.type) {
                if (with_semantics) {
                    matched_tokens.push_back(current_token);
                }
                parse_stack.pop();
                advance();
            } else {
                setError("期望终结符 " + tokenTypeToString(top.token_type) + "，但得到 " +
                         tokenTypeToString(current_token.type));
                return ParseResult::SYNTAX_ERROR;
            }
        } else { // 非终结符
            auto key = std::make_pair(top.non_terminal, current_token.type);
            if (parse_table.count(key) && parse_table[key].has_production) {
                int prod_idx = parse_table[key].production_index;
                const Production &prod = productions[prod_idx];

                printProduction(prod_idx);

                parse_stack.pop();

                // 执行语义动作
                if (with_semantics && semantic_callback) {
                    semantic_callback(prod_idx, matched_tokens);
                }

                const std::vector<Symbol> &right = prod.right;
                for (auto it = right.rbegin(); it != right.rend(); ++it) {
                    if (it->type == SymbolType::EPSILON) continue;
                    parse_stack.push(*it);
                }
            } else {
                setError("在符号 " + nonTerminalToString(top.non_terminal) + " 和输入 token " +
                         tokenTypeToString(current_token.type) + " 上没有可用产生式");
                return ParseResult::SYNTAX_ERROR;
            }
        }
    }

    if (!isEOF()) {
        setError("输入未完全消费，剩余 token 类型: " + tokenTypeToString(current_token.type));
        return ParseResult::INCOMPLETE_INPUT;
    }

    return ParseResult::SUCCESS;
}

void Parser::initializeGrammar() {
    using S = Symbol;

    auto T = [](int t) { return S(t); };
    auto NT = [](NonTerminal nt) { return S(nt); };

    // 1. stmts → stmt rest0
    productions.emplace_back(NonTerminal::STMTS, std::vector<S>{NT(NonTerminal::STMT), NT(NonTerminal::REST0)},
                             "stmts → stmt rest0");

    // 2. rest0 → stmt rest0
    productions.emplace_back(NonTerminal::REST0, std::vector<S>{NT(NonTerminal::STMT), NT(NonTerminal::REST0)},
                             "rest0 → stmt rest0");
    // 3. rest0 → ε
    productions.emplace_back(NonTerminal::REST0, std::vector<S>{}, "rest0 → ε");

    // 4. stmt → loc = expr ;
    productions.emplace_back(NonTerminal::STMT,
                             std::vector<S>{NT(NonTerminal::LOC), T(46), NT(NonTerminal::EXPR), T(84)},
                             "stmt → loc = expr ;");

    // 5. stmt → if ( bool ) stmt else stmt
    productions.emplace_back(NonTerminal::STMT,
                             std::vector<S>{T(17), T(81), NT(NonTerminal::BOOL_EXPR), T(82),
                                            NT(NonTerminal::STMT), T(15), NT(NonTerminal::STMT)},
                             "stmt → if ( bool ) stmt else stmt");

    // 6. stmt → while ( bool ) stmt
    productions.emplace_back(NonTerminal::STMT,
                             std::vector<S>{T(20), T(81), NT(NonTerminal::BOOL_EXPR), T(82), NT(NonTerminal::STMT)},
                             "stmt → while ( bool ) stmt");

    // 7. loc → id resta
    productions.emplace_back(NonTerminal::LOC,
                             std::vector<S>{T(111), NT(NonTerminal::RESTA)},
                             "loc → id resta");

    // 8. resta → [ elist ]
    productions.emplace_back(NonTerminal::RESTA,
                             std::vector<S>{T(88), NT(NonTerminal::ELIST), T(89)},
                             "resta → [ elist ]");
    // 9. resta → ε
    productions.emplace_back(NonTerminal::RESTA, std::vector<S>{}, "resta → ε");

    // 10. elist → expr rest1
    productions.emplace_back(NonTerminal::ELIST,
                             std::vector<S>{NT(NonTerminal::EXPR), NT(NonTerminal::REST1)},
                             "elist → expr rest1");

    // 11. rest1 → , expr rest1
    productions.emplace_back(NonTerminal::REST1,
                             std::vector<S>{T(90), NT(NonTerminal::EXPR), NT(NonTerminal::REST1)},
                             "rest1 → , expr rest1");
    // 12. rest1 → ε
    productions.emplace_back(NonTerminal::REST1, std::vector<S>{}, "rest1 → ε");

    // 13. bool → equality (这里命名为 BOOL_EXPR)
    productions.emplace_back(NonTerminal::BOOL_EXPR,
                             std::vector<S>{NT(NonTerminal::EQUALITY)},
                             "bool → equality");

    // 14. equality → rel rest4
    productions.emplace_back(NonTerminal::EQUALITY,
                             std::vector<S>{NT(NonTerminal::REL), NT(NonTerminal::REST4)},
                             "equality → rel rest4");

    // 15. rest4 → == rel rest4
    productions.emplace_back(NonTerminal::REST4,
                             std::vector<S>{T(51), NT(NonTerminal::REL), NT(NonTerminal::REST4)},
                             "rest4 → == rel rest4");
    // 16. rest4 → != rel rest4
    productions.emplace_back(NonTerminal::REST4,
                             std::vector<S>{T(52), NT(NonTerminal::REL), NT(NonTerminal::REST4)},
                             "rest4 → != rel rest4");
    // 17. rest4 → ε
    productions.emplace_back(NonTerminal::REST4, std::vector<S>{}, "rest4 → ε");

    // 18. rel → expr rop_expr
    productions.emplace_back(NonTerminal::REL,
                             std::vector<S>{NT(NonTerminal::EXPR), NT(NonTerminal::ROP_EXPR)},
                             "rel → expr rop_expr");

    // 19. rop_expr → < expr
    productions.emplace_back(NonTerminal::ROP_EXPR, std::vector<S>{T(49), NT(NonTerminal::EXPR)},
                             "rop_expr → < expr");
    // 20. rop_expr → <= expr
    productions.emplace_back(NonTerminal::ROP_EXPR, std::vector<S>{T(50), NT(NonTerminal::EXPR)},
                             "rop_expr → <= expr");
    // 21. rop_expr → > expr
    productions.emplace_back(NonTerminal::ROP_EXPR, std::vector<S>{T(47), NT(NonTerminal::EXPR)},
                             "rop_expr → > expr");
    // 22. rop_expr → >= expr
    productions.emplace_back(NonTerminal::ROP_EXPR, std::vector<S>{T(48), NT(NonTerminal::EXPR)},
                             "rop_expr → >= expr");
    // 23. rop_expr → ε
    productions.emplace_back(NonTerminal::ROP_EXPR, std::vector<S>{}, "rop_expr → ε");

    // 24. expr → term rest5
    productions.emplace_back(NonTerminal::EXPR,
                             std::vector<S>{NT(NonTerminal::TERM), NT(NonTerminal::REST5)},
                             "expr → term rest5");

    // 25. rest5 → + term rest5
    productions.emplace_back(NonTerminal::REST5, std::vector<S>{T(41), NT(NonTerminal::TERM), NT(NonTerminal::REST5)},
                             "rest5 → + term rest5");
    // 26. rest5 → - term rest5
    productions.emplace_back(NonTerminal::REST5, std::vector<S>{T(42), NT(NonTerminal::TERM), NT(NonTerminal::REST5)},
                             "rest5 → - term rest5");
    // 27. rest5 → ε
    productions.emplace_back(NonTerminal::REST5, std::vector<S>{}, "rest5 → ε");

    // 28. term → unary rest6
    productions.emplace_back(NonTerminal::TERM,
                             std::vector<S>{NT(NonTerminal::UNARY), NT(NonTerminal::REST6)},
                             "term → unary rest6");

    // 29. rest6 → * unary rest6
    productions.emplace_back(NonTerminal::REST6, std::vector<S>{T(43), NT(NonTerminal::UNARY), NT(NonTerminal::REST6)},
                             "rest6 → * unary rest6");
    // 30. rest6 → / unary rest6
    productions.emplace_back(NonTerminal::REST6, std::vector<S>{T(44), NT(NonTerminal::UNARY), NT(NonTerminal::REST6)},
                             "rest6 → / unary rest6");
    // 31. rest6 → ε
    productions.emplace_back(NonTerminal::REST6, std::vector<S>{}, "rest6 → ε");

    // 32. unary → factor
    productions.emplace_back(NonTerminal::UNARY, std::vector<S>{NT(NonTerminal::FACTOR)}, "unary → factor");

    // 33. factor → ( expr )
    productions.emplace_back(NonTerminal::FACTOR, std::vector<S>{T(81), NT(NonTerminal::EXPR), T(82)},
                             "factor → ( expr )");
    // 34. factor → loc
    productions.emplace_back(NonTerminal::FACTOR, std::vector<S>{NT(NonTerminal::LOC)}, "factor → loc");
    // 35. factor → num
    productions.emplace_back(NonTerminal::FACTOR, std::vector<S>{T(100)}, "factor → num");
}

std::set<int> Parser::computeFirstOfString(const std::vector<Symbol> &symbols) {
    std::set<int> result;

    if (symbols.empty()) {
        result.insert(EPSILON_TOKEN);
        return result;
    }

    for (const Symbol &sym : symbols) {
        if (sym.type == SymbolType::TERMINAL) {
            result.insert(sym.token_type);
            return result;
        } else if (sym.type == SymbolType::NON_TERMINAL) {
            const auto &first_nt = first_sets[sym.non_terminal];
            result.insert(first_nt.begin(), first_nt.end());
            if (first_nt.count(EPSILON_TOKEN) == 0) {
                // 不能推出 epsilon
                result.erase(EPSILON_TOKEN);
                return result;
            } else {
                // 可以推出 epsilon，继续看下一个符号
                result.erase(EPSILON_TOKEN);
            }
        } else { // EPSILON
            result.insert(EPSILON_TOKEN);
            return result;
        }
    }
    // 所有符号都能推出 epsilon
    result.insert(EPSILON_TOKEN);
    return result;
}

void Parser::computeFirstSets() {
    for (int nt = (int) NonTerminal::STMTS; nt <= (int) NonTerminal::FACTOR; ++nt) {
        first_sets[(NonTerminal) nt] = {};
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto &prod : productions) {
            auto &first_A = first_sets[prod.left];
            std::set<int> first_alpha = computeFirstOfString(prod.right);
            size_t before = first_A.size();
            first_A.insert(first_alpha.begin(), first_alpha.end());
            if (first_A.size() != before) changed = true;
        }
    }
}

void Parser::computeFollowSets() {
    for (int nt = (int) NonTerminal::STMTS; nt <= (int) NonTerminal::FACTOR; ++nt) {
        follow_sets[(NonTerminal) nt] = {};
    }

    follow_sets[NonTerminal::STMTS].insert(0);

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto &prod : productions) {
            const NonTerminal A = prod.left;
            const auto &alpha = prod.right;
            for (size_t i = 0; i < alpha.size(); ++i) {
                const Symbol &sym = alpha[i];
                if (sym.type != SymbolType::NON_TERMINAL) continue;
                NonTerminal B = sym.non_terminal;

                std::vector<Symbol> beta(alpha.begin() + i + 1, alpha.end());
                std::set<int> first_beta = computeFirstOfString(beta);

                size_t before = follow_sets[B].size();

                for (int t : first_beta) {
                    if (t != EPSILON_TOKEN) follow_sets[B].insert(t);
                }
                if (follow_sets[B].size() != before) changed = true;

                if (first_beta.count(EPSILON_TOKEN) || beta.empty()) {
                    size_t bef = follow_sets[B].size();
                    follow_sets[B].insert(follow_sets[A].begin(), follow_sets[A].end());
                    if (follow_sets[B].size() != bef) changed = true;
                }
            }
        }
    }
}

void Parser::buildParseTable() {
    parse_table.clear();

    for (size_t idx = 0; idx < productions.size(); ++idx) {
        const Production &prod = productions[idx];
        std::set<int> first_alpha = computeFirstOfString(prod.right);

        for (int t : first_alpha) {
            if (t != EPSILON_TOKEN) {
                parse_table[{prod.left, t}] = TableEntry((int) idx);
            }
        }

        if (first_alpha.count(EPSILON_TOKEN)) {
            for (int b : follow_sets[prod.left]) {
                parse_table[{prod.left, b}] = TableEntry((int) idx);
            }
        }
    }
}

void Parser::advance() {
    current_token = lexer.getNextToken();
}

bool Parser::isEOF() const {
    return current_token.type == 0;
}

void Parser::setError(const std::string &message) {
    error_message = message;
}

void Parser::setVerbose(bool v) { verbose = v; }

const std::string &Parser::getErrorMessage() const { return error_message; }

void Parser::printProduction(int production_index) {
    if (production_index >= 0 && (size_t) production_index < productions.size()) {
        std::cout << "(" << step_counter++ << ") " << productions[production_index].description << std::endl;
    }
}

std::string Parser::symbolToString(const Symbol &symbol) const {
    if (symbol.type == SymbolType::TERMINAL) {
        return tokenTypeToString(symbol.token_type);
    } else if (symbol.type == SymbolType::NON_TERMINAL) {
        return nonTerminalToString(symbol.non_terminal);
    } else {
        return "ε";
    }
}

std::string Parser::nonTerminalToString(NonTerminal nt) const {
    switch (nt) {
        case NonTerminal::STMTS: return "stmts";
        case NonTerminal::REST0: return "rest0";
        case NonTerminal::STMT: return "stmt";
        case NonTerminal::LOC: return "loc";
        case NonTerminal::RESTA: return "resta";
        case NonTerminal::ELIST: return "elist";
        case NonTerminal::REST1: return "rest1";
        case NonTerminal::BOOL_EXPR: return "bool";
        case NonTerminal::EQUALITY: return "equality";
        case NonTerminal::REST4: return "rest4";
        case NonTerminal::REL: return "rel";
        case NonTerminal::ROP_EXPR: return "rop_expr";
        case NonTerminal::EXPR: return "expr";
        case NonTerminal::REST5: return "rest5";
        case NonTerminal::TERM: return "term";
        case NonTerminal::REST6: return "rest6";
        case NonTerminal::UNARY: return "unary";
        case NonTerminal::FACTOR: return "factor";
        default: return "?";
    }
}

std::string Parser::tokenTypeToString(int token_type) const {
    if (token_type == EPSILON_TOKEN) return "ε";
    if (token_type == 0) return "$"; // EOF
    return std::to_string(token_type);
}

std::string parseResultToString(ParseResult result) {
    switch (result) {
        case ParseResult::SUCCESS:
            return "成功";
        case ParseResult::SYNTAX_ERROR:
            return "语法错误";
        case ParseResult::INCOMPLETE_INPUT:
            return "输入不完整";
        default:
            return "未知错误";
    }
}

 