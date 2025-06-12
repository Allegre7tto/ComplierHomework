#include "compliers/lexer.h"
#include <cctype>

Lexer::Lexer(std::istream& is) : input(is), current_char(' '), line(1), column(0) {
    readChar();
    initializeTables();
}

Token Lexer::getNextToken() {
        skipWhitespace();
        
        if (isEOF()) {
            return Token(0, "", line, column); // EOF
        }

        int token_line = line;
        int token_column = column;

        Token token = tryRecognizeToken(token_line, token_column);
        
        return token;
}

void Lexer::initializeTables() {
        keywords["int"] = 5;
        keywords["else"] = 15;
        keywords["if"] = 17;
        keywords["while"] = 20;

        delimiters['('] = 81;  delimiters[')'] = 82;  delimiters[';'] = 84;
        delimiters['{'] = 86;  delimiters['}'] = 87;  delimiters['['] = 88;
        delimiters[']'] = 89;  delimiters[','] = 90;
}

void Lexer::readChar() {
        current_char = input.get();
        if (current_char == '\n') {
            line++;
            column = 0;
        } else if (current_char != EOF) {
            column++;
        }
}

char Lexer::peekChar() {
        return input.peek();
}

bool Lexer::isEOF() {
        return input.eof() || current_char == EOF;
}

void Lexer::skipWhitespace() {
        while (std::isspace(current_char) && !isEOF()) {
            readChar();
        }
}

Token Lexer::tryRecognizeToken(int token_line, int token_column) {
        if (delimiters.count(current_char)) {
            return recognizeDelimiter(token_line, token_column);
        }

        if (std::isalpha(current_char) || current_char == '_') {
            return recognizeIdentifierOrKeyword(token_line, token_column);
        }

        if (std::isdigit(current_char)) {
            return recognizeNumber(token_line, token_column);
        }

        if (isOperatorStart(current_char)) {
            return recognizeOperatorOrComment(token_line, token_column);
        }

        return handleUnrecognizedChar(token_line, token_column);
}

Token Lexer::recognizeDelimiter(int token_line, int token_column) {
        char c = current_char;
        readChar();
        return Token(delimiters[c], "", token_line, token_column);
}

Token Lexer::recognizeIdentifierOrKeyword(int token_line, int token_column) {
        std::string value;
        
        while ((std::isalnum(current_char) || current_char == '_') && !isEOF()) {
            value += current_char;
            readChar();
        }

        if (keywords.count(value)) {
            return Token(keywords[value], "", token_line, token_column);
        } else {
            return Token(111, value, token_line, token_column); // 标识符
        }
}

Token Lexer::recognizeNumber(int token_line, int token_column) {
        std::string value;
        
        while (std::isdigit(current_char) && !isEOF()) {
            value += current_char;
            readChar();
        }
        
        return Token(100, value, token_line, token_column); // 整型常数
}

bool Lexer::isOperatorStart(char c) {
        return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
               c == '=' || c == '>' || c == '<' || c == '!' ||
               c == '&' || c == '|' || c == '#';
}

Token Lexer::recognizeOperatorOrComment(int token_line, int token_column) {
        char first_char = current_char;
        readChar();

        switch (first_char) {
            case '=':
                if (current_char == '=') {
                    readChar();
                    return Token(51, "", token_line, token_column); // ==
                }
                return Token(46, "", token_line, token_column); // =

            case '>':
                if (current_char == '=') {
                    readChar();
                    return Token(48, "", token_line, token_column); // >=
                }
                return Token(47, "", token_line, token_column); // >

            case '<':
                if (current_char == '=') {
                    readChar();
                    return Token(50, "", token_line, token_column); // <=
                }
                return Token(49, "", token_line, token_column); // <

            case '!':
                if (current_char == '=') {
                    readChar();
                    return Token(52, "", token_line, token_column); // !=
                }
                return Token(55, "", token_line, token_column); // !

            case '&':
                if (current_char == '&') {
                    readChar();
                    return Token(53, "", token_line, token_column); // &&
                }
                return handleUnrecognizedChar(token_line, token_column);

            case '|':
                if (current_char == '|') {
                    readChar();
                    return Token(54, "", token_line, token_column); // ||
                }
                return handleUnrecognizedChar(token_line, token_column);

            case '+':
                if (current_char == '+') {
                    readChar();
                    return Token(56, "", token_line, token_column); // ++
                }
                return Token(41, "", token_line, token_column); // +

            case '-':
                if (current_char == '-') {
                    readChar();
                    return Token(57, "", token_line, token_column); // --
                }
                return Token(42, "", token_line, token_column); // -

            case '/':
                return Token(44, "", token_line, token_column); // /

            case '*':
                return Token(43, "", token_line, token_column); // *

            case '%':
                return Token(45, "", token_line, token_column); // %

            case '#':
                skipSingleLineComment();
                return getNextToken();

            default:
                return handleUnrecognizedChar(token_line, token_column);
        }
}

void Lexer::skipSingleLineComment() {
        while (current_char != '\n' && !isEOF()) {
            readChar();
        }
}

Token Lexer::handleUnrecognizedChar(int token_line, int token_column) {
        readChar();
        return Token(-1, "", token_line, token_column);
}
