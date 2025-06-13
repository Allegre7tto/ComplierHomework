mod lexer;

use lexer::Lexer;

fn main() {
    // 测试代码
    let test_input = r#"
        int x = 10;
        if (x >= 5) {
            x++;
        } else {
            x--;
        }
        # 这是一个注释
        while (x != 0) {
            x = x - 1;
        }
    "#;

    println!("输入代码:");
    println!("{}", test_input);
    println!("\n词法分析结果:");
    println!("================");

    let mut lexer = Lexer::new(test_input);
    let (tokens, errors) = lexer.tokenize();

    for token in &tokens {
        if token.token_type != 0 { // 不显示EOF
            println!("{} (行: {}, 列: {})", token, token.line, token.column);
        }
    }

    // 显示错误信息
    if !errors.is_empty() {
        println!("\n错误信息:");
        println!("================");
        for error in &errors {
            println!("错误: {}", error);
        }
    }
}
