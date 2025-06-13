# Rust 词法分析器

一个用 Rust 实现的 C 语言词法分析器，支持完整的词法分析功能。

## 项目结构

```
compiler-rust/
├── src/
│   ├── main.rs          # 主程序入口，包含测试代码
│   ├── lexer.rs         # 词法分析器核心实现
│   └── lib.rs           # 库文件
├── tests/
│   └── lexer_test.rs    # 命令行测试工具
├── Cargo.toml           # 项目配置文件
├── Cargo.lock           # 依赖锁定文件
└── README.md            # 项目说明文档
```

## 功能特性

- ✅ **关键字识别**: `int`, `float`, `char`, `void`, `if`, `else`, `while`, `for`, `return` 等
- ✅ **标识符和常量**: 变量名、整数、浮点数（支持科学计数法）
- ✅ **字符串字面量**: 支持转义字符处理
- ✅ **操作符**: 算术、比较、逻辑、赋值操作符
- ✅ **分隔符**: 括号、分号、逗号等
- ✅ **注释处理**: 单行注释 (`//`, `#`) 和多行注释 (`/* */`)
- ✅ **错误处理**: 详细的错误信息和位置定位

## 编译指令

### 环境要求
- Rust 1.70+ 
- Cargo 包管理器

### 编译项目
```bash
# 检查代码
cargo check

# 编译项目
cargo build

# 编译发布版本
cargo build --release
```

### 运行程序

#### 1. 命令行词法分析
```bash
# 基本用法
cargo run --bin lexer_test "int x = 42;"

# 复杂示例
cargo run --bin lexer_test "int main() { float x = 3.14e-2; return 0; }"

# 包含注释的代码
cargo run --bin lexer_test "int x = 10; /* 这是注释 */ x++;"
```




