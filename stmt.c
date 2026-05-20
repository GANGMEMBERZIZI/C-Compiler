#include "defs.h"
#include "data.h"
#include "decl.h"
//构建语句的ast
/* BNF表达式 statements: statement   //statements语句块 statement 是单个语句块  statements: statement statements 代换 得出 遍历 可以连续输出
     | statement statements
     ;

statement: 'print' expression ';'
     ; */ 
      /*statements: statement
      |      statement statements
      ;

 statement: 'print' expression ';'
      |     'int'   identifier ';'
      |     identifier '=' expression ';'
      ;

 identifier: T_IDENT
 while_statement: 'while' '(' true_false_expression ')' compound_statement  ;
      ; */
static struct ASTnode *single_statement(void);
struct ASTnode *if_statement(){
     struct ASTnode *condAST ,*trueAST ,*falseAST=NULL;
     match(T_IF,"if");
     lparen();//if里的左小括号
     condAST=binexpr(0);//给cond里的数字表达式构成树
     if(condAST->op<A_EQ||condAST->op>A_GE){//确保里面是逻辑符号
          condAST = mkastunary(A_TOBOOL, condAST->type, condAST, 0);//自动套一个bool
     }
     rparen();
     trueAST=compound_statement();
     if(Token.token==T_ELSE){
          scan(&Token);
          falseAST=compound_statement();
     }
     return (mkastnode(A_IF,P_NONE,condAST, trueAST, falseAST, 0));
}
struct ASTnode *while_statement(){//while语句
     struct ASTnode *condAST , *bodyAST;
     match(T_WHILE,"while");
     lparen();
     condAST=binexpr(0);
     if(condAST->op<A_EQ||condAST->op>A_GE)
     condAST = mkastunary(A_TOBOOL, condAST->type, condAST, 0);
     rparen();
     bodyAST=compound_statement();
     return (mkastnode(A_WHILE,P_NONE,condAST,NULL,bodyAST,0));
}
// for_statement: 'for' '(' preop_statement ';'
//                          true_false_expression ';'
//                          postop_statement ')' compound_statement  ;
//
// preop_statement:  statement          (for now)
// postop_statement: statement          (for now)
//
// Parse a FOR statement
// and return its AST
static struct ASTnode *for_statement(){
     struct ASTnode *condAST,*bodyAST;
     struct ASTnode *preopAST,*postopAST;
     struct ASTnode *tree;
     match(T_FOR,"for");
     lparen();
     preopAST=single_statement();
     semi();
     condAST=binexpr(0);
     if (condAST->op < A_EQ || condAST->op > A_GE)
     condAST = mkastunary(A_TOBOOL, condAST->type, condAST, 0);
     semi();
     postopAST=single_statement();
     rparen();
     bodyAST=compound_statement();
     tree=mkastnode(A_GLUE,P_NONE,bodyAST,NULL,postopAST,0);
     tree=mkastnode(A_WHILE,P_NONE,condAST,NULL,tree,0);
     return (mkastnode(A_GLUE,P_NONE,preopAST,NULL,tree,0));
}
static struct ASTnode *return_statement(){
     struct ASTnode * tree;
     if (Symtable[Functionid].type==P_VOID)
     fatal("Can't return from a void function");
     match(T_RETURN,"return");
     lparen();
     tree=binexpr(0);//此时已经知道return的type是什么了
     tree=modify_type(tree,Symtable[Functionid].type,0);
     if(tree==NULL)
     fatal("Incompatible type to print");
     tree = mkastunary(A_RETURN, P_NONE, tree, 0);
     rparen();
     return tree;
}
static struct ASTnode *single_statement(void) {//这个是 for括号里的第三个
     int type;
  switch (Token.token) {
    case T_CHAR:  
    case T_INT:
    case T_LONG:
    type=parse_type();
    ident();
      var_declaration(type,1,0);//局部变量
      semi();
      return (NULL);		
    case T_IF:
      return (if_statement());
    case T_WHILE:
      return (while_statement());
    case T_FOR:
      return (for_statement());
    case T_RETURN:
      return (return_statement()); 
    default:
      return binexpr(0);
  }
  return (NULL);
}
struct ASTnode * compound_statement(){
     struct ASTnode *left=NULL;
     struct ASTnode *tree;
     lbrace();
  while (1)
  {
    tree=single_statement();
    if (tree != NULL && (tree->op == A_ASSIGN||tree->op ==A_RETURN||tree->op==A_FUNCCALL))//需要分号的特殊情况
      semi();
    if(tree!=NULL){
     if(left==NULL){
          left=tree;
     }
     else{
          left=mkastnode(A_GLUE,P_NONE,left,NULL,tree,0);
     }
    }
    if(Token.token==T_RBRACE){
     rbrace();
     return left;
    }
  }
}     
/*{
    print("Hello");      // 第一个子语句
    int x = 10;          // 第二个子语句 (变量声明)
    x = x + 5;           // 第三个子语句 (赋值)
} 用这个来当作例子解释
 首先解析第一个子句 先tree等于print构建子树 然后left一开始为null，赋值到了left
 解释第二个子句定义，tree=null
 第三个 tree=ident left不为null 用glue粘合*/