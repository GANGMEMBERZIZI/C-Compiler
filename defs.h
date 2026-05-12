#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  //字符测试和转换函数
enum{//这个是词法分析识别input的字符 扫描器 不分析语法 所以 ++ -- 不分前后
    T_E0F,
  // Binary operators
  T_ASSIGN, T_LOGOR, T_LOGAND, 
  T_OR, T_XOR, T_AMPER, 
  T_EQ, T_NE,
  T_LT, T_GT, T_LE, T_GE,
  T_LSHIFT, T_RSHIFT,
  T_PLUS, T_MINUS, T_STAR, T_SLASH,
  // Other operators
  T_INC, T_DEC, T_INVERT, T_LOGNOT,
  // Type keywords
  T_VOID, T_CHAR, T_INT, T_LONG,
  // Other keywords
  T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN,
  // Structural tokens
  T_INTLIT, T_STRLIT, T_SEMI, T_IDENT,
  T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN,
  T_LBRACKET, T_RBRACKET
};
struct token            
{
    int token;
    int intvalue; //表示整数值
};
struct ASTnode{
    int op;
    int type;//记录树的类型
    int rvalue;//如果是右值则为真
    struct ASTnode *right;
    struct ASTnode *mid;
    struct ASTnode *left;
    union 
    {
         int intvalue;
         int id;
         int size;
    }v;  
};
enum{//记录已经被识别的变量(ast)的数据类型 像glue只是粘合作用的就是none
    P_NONE,P_VOID,P_CHAR,P_INT,P_LONG,P_VOIDPTR, P_CHARPTR, P_INTPTR, P_LONGPTR
};
//AST node types 解析器的枚举值  A_TOBOOL隐式不等于零比较器
enum{
     A_ASSIGN= 1, A_LOGOR, A_LOGAND, A_OR, A_XOR, A_AND,
  A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE, A_LSHIFT, A_RSHIFT,
  A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
  A_INTLIT, A_STRLIT, A_IDENT, A_GLUE,
  A_IF, A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN,
  A_FUNCCALL, A_DEREF, A_ADDR, A_SCALE,
  A_PREINC, A_PREDEC, A_POSTINC, A_POSTDEC,
  A_NEGATE, A_INVERT, A_LOGNOT, A_TOBOOL
};
enum{//记录定义的是函数还是变量
    S_VARIABLE,S_FUNCTION,S_ARRAY
};
struct symtable{//符号表
    char *name;//变量的名字
    int type;//记录变量类型
    int stype;//记录是函数还是变量
    int endlabel; //函数结束标签
    int size;//个数
};
