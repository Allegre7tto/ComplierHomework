# 编译器实验

词法分析器、语法分析器和语义分析器实现。

## 项目结构

```
.
├── CMakeLists.txt          # CMake构建配置
├── README.md               # 项目说明
├── tests.txt               # 测试脚本
├── include/compliers/      # 头文件
│   ├── lexer.h            # 词法分析器
│   ├── parser.h           # 语法分析器
│   └── semantic.h         # 语义分析器
├── src/                   # 源文件
│   ├── lexer.cpp          
│   ├── parser.cpp         
│   ├── semantic.cpp       
│   └── main.cpp           
├── tests/                 # 测试程序
│   ├── lexer_test.cpp     
│   ├── ll1_parser_test.cpp 
│   └── semantic_test.cpp  
└── build/                 # 构建目录
```

## 编译

```bash
mkdir build && cd build
cmake ..
make
```

## 运行

### 词法分析器
```bash
./bin/lexer_test                    # 交互式
echo "a=b+c;" | ./bin/lexer_test    # 管道输入
```

### 语法分析器
```bash
./bin/ll1_parser_test "x=5*2/3;"    # 命令行
./bin/ll1_parser_test               # 交互式
```

### 语义分析器
```bash
./bin/semantic_test "a=b+c;"        # 命令行
./bin/semantic_test                 # 交互式
```