#include "defs.h"
#include "data.h"
#include "decl.h"
int parse_type(){
    int type;
    switch (Token.token) {
    case T_VOID:
      type = P_VOID;
      break;
    case T_CHAR:
      type = P_CHAR;
      break;
    case T_INT:
      type = P_INT;
      break;
    case T_LONG:
      type = P_LONG;
      break;
    default:
      fatald("Illegal type, token", Token.token);
  }
  while(1){
    scan(&Token);
    if (Token.token!=T_STAR)
    break;
    type =pointer_to(type);
  }
  return type;
}
void var_declaration(int type){
  int id;
  if(Token.token==T_LBRACKET){
    scan(&Token);
    if (Token.token==T_INTLIT){
      id=addglob(Text,pointer_to(type),S_ARRAY,0,Token.intvalue);
      genglobsym(id);
    }
    scan(&Token);
    match(T_RBRACKET,"]");
  }else{
    id=addglob(Text,type,S_VARIABLE,0,1);
    genglobsym(id);
  }
  semi();
}
struct ASTnode *function_declaration(int type){
    struct ASTnode *tree,*finalstmt;
    int nameslot,endlabel;
    endlabel=genlabel();//记录的是函数的终点出口
    nameslot=addglob(Text,type,S_FUNCTION,endlabel,0);//添加函数名字为变量,nameslot是变量数组索引
    Functionid=nameslot;//告诉编译器 我在解析那个函数 这个包括了 函数名和类型
    lparen();
    rparen();
    tree=compound_statement();
    if(type!=P_VOID){
      if (tree == NULL)
      fatal("No statements in function with non-void type");
        finalstmt = (tree->op == A_GLUE) ? tree->right : tree;//如果用了 glue 那右子树就是最后的语句 用 glue将它和return 粘合起来
        if (finalstmt == NULL || finalstmt->op != A_RETURN)
      fatal("No return for function with non-void type");
    }
    return mkastunary(A_FUNCTION,type,tree,nameslot);
}
void global_declarations(){//全局变量
  struct ASTnode *tree;
  int type;
  while(1){
    type=parse_type();
    ident();
    if(Token.token==T_LPAREN){//如果有括号 是函数
      tree=function_declaration(type);
      if (O_dumpAST){//当其为 1（真）时：编译器会在解析完每一个函数或全局定义后，立即调用 dumpAST() 函数，将树的结构以缩进的形式输出到屏幕上。
        dumpAST(tree, NOLABEL, 0); //将内存的ast 输出
        fprintf(stdout, "\n\n");//在屏幕（标准输出）上打印两个换行符
      }
      genAST(tree,NOLABEL,0);
    }else{
      var_declaration(type);//是变量
    }
    if(Token.token==T_E0F)
    break;
  }
}