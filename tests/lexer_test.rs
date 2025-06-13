use std::env;
use std::process;

// 引入词法分析器模块
use compiler_rust::lexer::Lexer;

fn main() {
    let args: Vec<String> = env::args().collect();
    
    if args.len() != 2 {
        print_usage();
        process::exit(1);
    }
    
    let input = &args[1];
    
    // 创建词法分析器并进行分析
    let mut lexer = Lexer::new(input);
    let (tokens, errors) = lexer.tokenize();
    
    // 输出令牌（不显示EOF）
    for token in &tokens {
        if token.token_type != 0 {
            println!("{}", token);
        }
    }
    
    // 显示错误信息
    if !errors.is_empty() {
        for error in &errors {
            eprintln!("错误: {}", error);
        }
    }
    
    // 如果有错误，以非零状态码退出
    if !errors.is_empty() {
        process::exit(1);
    }
}

fn print_usage() {
    println!("用法: lexer_test <代码>");
    println!("示例: lexer_test \"int x = 42;\"");
} 