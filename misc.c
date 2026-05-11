#include "defs.h"
#include "data.h"
#include "decl.h"
void match(int t,char *what){ //t:例如 T_INT, T_PLUS, T_SEMI what是你想输出的 如果你期望匹配一个分号，what 值会是 ";" 验证当前词法单元 (Token) 是否符合预期
    if(Token.token==t){//// 如果当前令牌符合预期
        scan(&Token);//消耗token 获取下一个
    }else{
        fatals("Expected", what);//不符合预期报错
    }
}
void semi(){ //遇到;可以直接调用 semi()，而不需要每次都写 match(T_SEMI, ";")
    match(T_SEMI,";");
}
void lbrace(){
   match(T_LBRACE,"{");
}
void rbrace(){
  match(T_RBRACE,"}");
}
void lparen(){
  match(T_LPAREN,"(");
}
void rparen(){
  match(T_RPAREN,")");
}
void ident(){
    match(T_IDENT,"identifier");
}
void fatal(char *s){
    fprintf(stderr,"%s on line %d\n",s,Line);//stderr 是一个预定义好的 FILE* 类型的指针，它指向标准错误流
    //打印一个字符串错误信息
    exit(1);
}
void fatals(char *s1, char *s2) {//s1 是错误的类型 s2是通常描述与错误相关的特定实体（例如，变量名、函数名
  fprintf(stderr, "%s:%s on line %d\n", s1, s2, Line);
  exit(1);
}
void fatald(char *s, int d) {//s是错误消息 d是与错误相关的数字
  fprintf(stderr, "%s:%d on line %d\n", s, d, Line);
  exit(1);
}
void fatalc(char *s, int c) {//s是错误消息 c是错误的字符 ascII码形式
  fprintf(stderr, "%s:%c on line %d\n", s, c, Line);
  exit(1);
}