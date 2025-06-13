use std::collections::HashMap;

// 错误类型枚举
#[derive(Debug, Clone, PartialEq)]
pub enum LexError {
    UnterminatedString { line: i32, column: i32 },
    InvalidNumber { value: String, line: i32, column: i32 },
    UnknownCharacter { char: char, line: i32, column: i32 },
}

impl std::fmt::Display for LexError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            LexError::UnterminatedString { line, column } => {
                write!(f, "未终止的字符串字面量 (行: {}, 列: {})", line, column)
            }
            LexError::InvalidNumber { value, line, column } => {
                write!(f, "无效的数字 '{}' (行: {}, 列: {})", value, line, column)
            }
            LexError::UnknownCharacter { char, line, column } => {
                write!(f, "未知字符 '{}' (行: {}, 列: {})", char, line, column)
            }
        }
    }
}

// Token 类型常量
const EOF: i32 = 0;
const INTEGER: i32 = 100;
const FLOAT: i32 = 101;
const STRING: i32 = 102;
const IDENTIFIER: i32 = 111;

#[derive(Debug, Clone, PartialEq)]
pub struct Token {
    pub token_type: i32,    // 种别编码
    pub value: String,      // 属性值 (用于标识符、常数等)
    pub line: i32,          // 行号
    pub column: i32,        // 列号
}

impl Token {
    pub fn new(token_type: i32, value: String, line: i32, column: i32) -> Self {
        Token {
            token_type,
            value,
            line,
            column,
        }
    }
}

impl std::fmt::Display for Token {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let value_display = if self.value.is_empty() { "-" } else { &self.value };
        write!(f, "< {}, {} >", self.token_type, value_display)
    }
}

pub struct Lexer {
    input: Vec<char>,
    position: usize,
    current_char: char,
    line: i32,
    column: i32,
    keywords: HashMap<String, i32>,
    delimiters: HashMap<char, i32>,
}

impl Lexer {
    pub fn new(input: &str) -> Self {
        let chars: Vec<char> = input.chars().collect();
        let mut lexer = Lexer {
            input: chars,
            position: 0,
            current_char: '\0',
            line: 1,
            column: 0,
            keywords: HashMap::new(),
            delimiters: HashMap::new(),
        };
        
        lexer.read_char();
        lexer.initialize_tables();
        lexer
    }

    pub fn get_next_token(&mut self) -> Result<Token, LexError> {
        self.skip_whitespace_and_comments();
        
        if self.is_eof() {
            return Ok(Token::new(EOF, String::new(), self.line, self.column));
        }

        let token_line = self.line;
        let token_column = self.column;

        // 分隔符
        if let Some(&token_type) = self.delimiters.get(&self.current_char) {
            self.read_char();
            return Ok(Token::new(token_type, String::new(), token_line, token_column));
        }

        // 标识符和关键字
        if self.current_char.is_alphabetic() || self.current_char == '_' {
            return Ok(self.read_identifier_or_keyword(token_line, token_column));
        }

        // 数字
        if self.current_char.is_ascii_digit() {
            return self.read_number(token_line, token_column);
        }

        // 字符串
        if self.current_char == '"' {
            return self.read_string_literal(token_line, token_column);
        }

        // 操作符
        if self.is_operator_char(self.current_char) {
            return Ok(self.read_operator(token_line, token_column));
        }

        Err(LexError::UnknownCharacter {
            char: self.current_char,
            line: token_line,
            column: token_column,
        })
    }



    fn initialize_tables(&mut self) {
        // 关键字表
        let keywords = [
            ("int", 5), ("float", 6), ("char", 7), ("void", 8), ("return", 9),
            ("else", 15), ("if", 17), ("while", 20), ("for", 21), ("do", 22),
            ("break", 23), ("continue", 24),
        ];
        for (word, code) in keywords {
            self.keywords.insert(word.to_string(), code);
        }

        // 分隔符表
        let delimiters = [
            ('(', 81), (')', 82), (';', 84), ('{', 86), ('}', 87),
            ('[', 88), (']', 89), (',', 90),
        ];
        for (char, code) in delimiters {
            self.delimiters.insert(char, code);
        }
    }

    fn read_char(&mut self) {
        if self.position >= self.input.len() {
            self.current_char = '\0';
        } else {
            self.current_char = self.input[self.position];
            self.position += 1;
            
            if self.current_char == '\n' {
                self.line += 1;
                self.column = 0;
            } else {
                self.column += 1;
            }
        }
    }

    fn peek_char(&self) -> char {
        if self.position >= self.input.len() {
            '\0'
        } else {
            self.input[self.position]
        }
    }



    fn is_eof(&self) -> bool {
        self.current_char == '\0'
    }

    fn skip_whitespace_and_comments(&mut self) {
        loop {
            // 跳过空白字符
            while self.current_char.is_whitespace() && !self.is_eof() {
                self.read_char();
            }

            // 处理注释
            if self.current_char == '/' {
                match self.peek_char() {
                    '/' => {
                        // 单行注释
                        while self.current_char != '\n' && !self.is_eof() {
                            self.read_char();
                        }
                        continue;
                    }
                    '*' => {
                        // 多行注释
                        self.read_char(); // 跳过 '/'
                        self.read_char(); // 跳过 '*'
                        
                        while !self.is_eof() {
                            if self.current_char == '*' && self.peek_char() == '/' {
                                self.read_char(); // 跳过 '*'
                                self.read_char(); // 跳过 '/'
                                break;
                            }
                            self.read_char();
                        }
                        continue;
                    }
                    _ => break,
                }
            } else if self.current_char == '#' {
                // # 开头的注释
                while self.current_char != '\n' && !self.is_eof() {
                    self.read_char();
                }
                continue;
            } else {
                break;
            }
        }
    }

    fn read_identifier_or_keyword(&mut self, line: i32, column: i32) -> Token {
        let mut value = String::new();
        
        while (self.current_char.is_alphanumeric() || self.current_char == '_') && !self.is_eof() {
            value.push(self.current_char);
            self.read_char();
        }

        let token_type = self.keywords.get(&value).copied().unwrap_or(IDENTIFIER);
        let token_value = if token_type == IDENTIFIER { value } else { String::new() };
        
        Token::new(token_type, token_value, line, column)
    }

    fn read_number(&mut self, line: i32, column: i32) -> Result<Token, LexError> {
        let mut value = String::new();
        let mut is_float = false;
        
        // 读取整数部分
        while self.current_char.is_ascii_digit() {
            value.push(self.current_char);
            self.read_char();
        }
        
        // 检查小数点
        if self.current_char == '.' && self.peek_char().is_ascii_digit() {
            is_float = true;
            value.push(self.current_char);
            self.read_char();
            
            while self.current_char.is_ascii_digit() {
                value.push(self.current_char);
                self.read_char();
            }
        }
        
        // 检查科学计数法
        if matches!(self.current_char, 'e' | 'E') {
            is_float = true;
            value.push(self.current_char);
            self.read_char();
            
            if matches!(self.current_char, '+' | '-') {
                value.push(self.current_char);
                self.read_char();
            }
            
            if !self.current_char.is_ascii_digit() {
                return Err(LexError::InvalidNumber { value, line, column });
            }
            
            while self.current_char.is_ascii_digit() {
                value.push(self.current_char);
                self.read_char();
            }
        }
        
        let token_type = if is_float { FLOAT } else { INTEGER };
        Ok(Token::new(token_type, value, line, column))
    }

    fn read_string_literal(&mut self, line: i32, column: i32) -> Result<Token, LexError> {
        let mut value = String::new();
        self.read_char(); // 跳过开始引号
        
        while self.current_char != '"' && !self.is_eof() {
            if self.current_char == '\\' {
                self.read_char();
                match self.current_char {
                    'n' => value.push('\n'),
                    't' => value.push('\t'),
                    'r' => value.push('\r'),
                    '\\' => value.push('\\'),
                    '"' => value.push('"'),
                    _ => {
                        value.push('\\');
                        value.push(self.current_char);
                    }
                }
            } else {
                value.push(self.current_char);
            }
            self.read_char();
        }
        
        if self.is_eof() {
            return Err(LexError::UnterminatedString { line, column });
        }
        
        self.read_char(); // 跳过结束引号
        Ok(Token::new(STRING, value, line, column))
    }

    fn is_operator_char(&self, c: char) -> bool {
        matches!(c, '+' | '-' | '*' | '/' | '%' | '=' | '>' | '<' | '!' | '&' | '|')
    }

    fn read_operator(&mut self, line: i32, column: i32) -> Token {
        let first_char = self.current_char;
        self.read_char();

        let token_type = match first_char {
            '=' => if self.current_char == '=' { self.read_char(); 51 } else { 46 },
            '>' => if self.current_char == '=' { self.read_char(); 48 } else { 47 },
            '<' => if self.current_char == '=' { self.read_char(); 50 } else { 49 },
            '!' => if self.current_char == '=' { self.read_char(); 52 } else { 55 },
            '&' => if self.current_char == '&' { self.read_char(); 53 } else { 58 },
            '|' => if self.current_char == '|' { self.read_char(); 54 } else { 59 },
            '+' => match self.current_char {
                '+' => { self.read_char(); 56 },
                '=' => { self.read_char(); 60 },
                _ => 41,
            },
            '-' => match self.current_char {
                '-' => { self.read_char(); 57 },
                '=' => { self.read_char(); 61 },
                _ => 42,
            },
            '*' => if self.current_char == '=' { self.read_char(); 62 } else { 43 },
            '/' => 44, // 注释已在 skip_whitespace_and_comments 中处理
            '%' => 45,
            _ => -1,
        };

        Token::new(token_type, String::new(), line, column)
    }

    pub fn tokenize(&mut self) -> (Vec<Token>, Vec<LexError>) {
        let mut tokens = Vec::new();
        let mut errors = Vec::new();
        
        loop {
            match self.get_next_token() {
                Ok(token) => {
                    let is_eof = token.token_type == EOF;
                    tokens.push(token);
                    if is_eof { break; }
                }
                Err(error) => {
                    errors.push(error);
                    if !self.is_eof() {
                        self.read_char(); // 跳过错误字符继续
                    } else {
                        break;
                    }
                }
            }
        }
        
        (tokens, errors)
    }
} 