#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>
#include <map>

struct Token {
    int type;           // 种别编码
    std::string value;  // 属性值 (用于标识符、常数等)
    int line;          // 行号
    int column;        // 列号

    Token(int t, std::string v = "", int l = 0, int c = 0) 
        : type(t), value(v), line(l), column(c) {}

    friend std::ostream& operator<<(std::ostream& os, const Token& token) {
        os << "< " << token.type << ", " << (token.value.empty() ? "-" : token.value) << " >";
        return os;
    }
};

class Lexer {
public:
    Lexer(std::istream& is);

    Token getNextToken();

private:
    std::istream& input;
    char current_char;
    int line;
    int column;

    std::map<std::string, int> keywords;
    std::map<char, int> delimiters;

    void initializeTables();

    void readChar();
    char peekChar();
    bool isEOF();
    void skipWhitespace();

    Token tryRecognizeToken(int token_line, int token_column);

    Token recognizeDelimiter(int token_line, int token_column);
    Token recognizeIdentifierOrKeyword(int token_line, int token_column);
    Token recognizeNumber(int token_line, int token_column);
    bool isOperatorStart(char c);
    Token recognizeOperatorOrComment(int token_line, int token_column);

    void skipSingleLineComment();

    Token handleUnrecognizedChar(int token_line, int token_column);
};

#endif // LEXER_H 