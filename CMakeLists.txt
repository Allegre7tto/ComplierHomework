# 最低CMake版本要求
cmake_minimum_required(VERSION 3.15)

# 项目信息
project(Compiler
    VERSION 1.0.0
    DESCRIPTION "A simple compiler with lexer and parser"
    LANGUAGES CXX
)

# 只有在作为主项目时才构建测试和示例
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # 确保使用-std=c++xx而不是-std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)
    
    # 支持文件夹在IDE中
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    
    # 测试只在主项目中构建
    include(CTest)
endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 创建词法分析器库
add_library(lexer
    src/lexer.cpp
)

# 设置lexer库的包含目录
target_include_directories(lexer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# 创建语法分析器库
add_library(parser
    src/parser.cpp
)

# 设置parser库的包含目录和依赖
target_include_directories(parser
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(parser
    PUBLIC lexer
)

# 创建语义分析器库
add_library(semantic
    src/semantic.cpp
)

# 设置semantic库的包含目录和依赖
target_include_directories(semantic
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(semantic
    PUBLIC lexer
)

# 创建词法分析器测试可执行文件
add_executable(lexer_test
    tests/lexer_test.cpp
)

target_link_libraries(lexer_test
    PRIVATE lexer
)

# 创建LL(1)语法分析器测试可执行文件
add_executable(ll1_parser_test
    tests/ll1_parser_test.cpp
)

target_link_libraries(ll1_parser_test
    PRIVATE parser lexer
)

# 创建语义分析器测试可执行文件
add_executable(semantic_test
    tests/semantic_test.cpp
)

target_link_libraries(semantic_test
    PRIVATE semantic lexer
)

# 创建语法分析器可执行文件
add_executable(compiler
    src/main.cpp
)

target_link_libraries(compiler
    PRIVATE parser semantic lexer
)

# 设置目标属性
set_target_properties(lexer parser semantic lexer_test ll1_parser_test semantic_test compiler
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 添加自定义目标用于测试
add_custom_target(test_compiler
    COMMAND echo "测试简单表达式:" && echo "5*2/3" | ${CMAKE_BINARY_DIR}/bin/compiler
    COMMAND echo "" && echo "测试复杂表达式:" && echo "9+5*2/3-6" | ${CMAKE_BINARY_DIR}/bin/compiler
    DEPENDS compiler
    COMMENT "运行语法分析器测试"
    VERBATIM
)

add_custom_target(test_lexer_target
    COMMAND echo "测试词法分析器:" && echo "9+5*2/3-6" | ${CMAKE_BINARY_DIR}/bin/lexer_test
    DEPENDS lexer_test
    COMMENT "运行词法分析器测试"
    VERBATIM
)

add_custom_target(test_parser_target
    COMMAND ${CMAKE_BINARY_DIR}/bin/ll1_parser_test
    DEPENDS ll1_parser_test
    COMMENT "运行LL(1)语法分析器测试"
    VERBATIM
)

add_custom_target(test_ll1_target
    COMMAND ${CMAKE_BINARY_DIR}/bin/ll1_parser_test
    DEPENDS ll1_parser_test
    COMMENT "运行LL(1)语法分析器测试"
    VERBATIM
)

add_custom_target(test_semantic_target
    COMMAND ${CMAKE_BINARY_DIR}/bin/semantic_test
    DEPENDS semantic_test
    COMMENT "运行语义分析器测试"
    VERBATIM
)

# 如果有测试输入文件，可以添加更多测试
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_input.txt)
    add_custom_target(test_from_file
        COMMAND ${CMAKE_BINARY_DIR}/bin/compiler < ${CMAKE_CURRENT_SOURCE_DIR}/test_input.txt
        DEPENDS compiler
        COMMENT "从文件运行测试"
        VERBATIM
    )
endif()

# 安装规则（可选）
include(GNUInstallDirs)

# 安装库
install(TARGETS lexer parser semantic
    EXPORT CompilerTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 安装可执行文件
install(TARGETS compiler lexer_test ll1_parser_test semantic_test
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 安装头文件
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 导出目标（可选，用于其他项目引用）
install(EXPORT CompilerTargets
    FILE CompilerTargets.cmake
    NAMESPACE Compiler::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Compiler
) 