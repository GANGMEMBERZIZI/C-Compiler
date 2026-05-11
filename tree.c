#include "defs.h"
#include "data.h"
#include "decl.h"
//构建AST
struct ASTnode *mkastnode (int op,int type,struct ASTnode *left,struct ASTnode *mid,struct ASTnode *right,int intvalue){ //return 一个结构体指针
    struct ASTnode *n;
    n=(struct ASTnode *)malloc(sizeof(struct ASTnode));//开辟空间
    if (n==NULL){
        fatal("Unable to malloc in mkastnode()");
    }
    n->op=op; //将传递的op left等等 给到新树n
    n->type=type;
    n->left=left;
    n->mid=mid;
    n->right=right;
    n->v.intvalue=intvalue;
    return n;
}
//叶子节点，只有左孩子地节点，增加可读性，不用赋值NULL
struct ASTnode *mkastleaf(int op,int type,int intvalue){
    return (mkastnode(op,type,NULL,NULL,NULL,intvalue));
}
struct ASTnode *mkastunary(int op,int type,struct ASTnode *left,int intvalue){
    return (mkastnode(op,type,left,NULL,NULL,intvalue));
}
static int gendumplabel(){
    static int id =1;//存在静态数据区 仅在程序加载时初始化一次。所以他会记住id的值 第二次调用id 就变成2了
    return id++;//先return id id+1
}
// 内存的分布 栈 堆 静态数据区 代码区(用来存放你写的代码的汇编指令)
void dumpAST(struct ASTnode *n,int label,int level){//打印一个树的结构
    int Lfalse,Lstart,Lend;
    switch(n->op){//特殊控制流节点
        case A_IF:
        Lfalse=gendumplabel();//如果left条件不成立 跳转到这个位置
        for (int i=0;i<level;i++){
            fprintf(stdout," ");//输出空格 以便分清谁是谁的孩子
        }
        fprintf(stdout, "A_IF");
        if (n->right){
            Lend=gendumplabel();//Lend 是else 结束的地方 整个if-else结束的地方 如果有else 条件成立 的话 直接跳转到 Lend 不用看 else的地方
            fprintf(stdout,", end L%d",Lend);
        }
        fprintf(stdout,"\n");
        dumpAST(n->left,Lfalse,level+2);//条件为假需要跳转
        dumpAST(n->mid,NOLABEL,level+2);
        if (n->right){//如果有else的话
            dumpAST(n->right,NOLABEL,level+2);
        }
        return ;
        case A_WHILE:
        Lstart=gendumplabel();
        for (int i=0;i<level;i++){
            fprintf(stdout," ");
        }
        fprintf(stdout,"A_WHILE, start L%d\n",Lstart);
        Lend=gendumplabel();
        dumpAST(n->left,Lend,level+2);//判断条件 是否跳转
        dumpAST(n->right,NOLABEL,level+2);
        return;
    }
    if (n->op==A_GLUE){
        level=-2;
    }
    //生成ast
    if (n->left) dumpAST(n->left, NOLABEL, level+2);
    if (n->right) dumpAST(n->left, NOLABEL, level+2);
    for (int i=0;i<level;i++){
            fprintf(stdout," ");
        }
    switch (n->op) {
    case A_GLUE:
      fprintf(stdout, "\n\n"); return;
    case A_FUNCTION:
      fprintf(stdout, "A_FUNCTION %s\n", Gsym[n->v.id].name); return;
    case A_ADD:
      fprintf(stdout, "A_ADD\n"); return;
    case A_SUBTRACT:
      fprintf(stdout, "A_SUBTRACT\n"); return;
    case A_MULTIPLY:
      fprintf(stdout, "A_MULTIPLY\n"); return;
    case A_DIVIDE:
      fprintf(stdout, "A_DIVIDE\n"); return;
    case A_EQ:
      fprintf(stdout, "A_EQ\n"); return;
    case A_NE:
      fprintf(stdout, "A_NE\n"); return;
    case A_LT:
      fprintf(stdout, "A_LE\n"); return;
    case A_GT:
      fprintf(stdout, "A_GT\n"); return;
    case A_LE:
      fprintf(stdout, "A_LE\n"); return;
    case A_GE:
      fprintf(stdout, "A_GE\n"); return;
    case A_INTLIT:
      fprintf(stdout, "A_INTLIT %d\n", n->v.intvalue); return;
    case A_STRLIT:
      fprintf(stdout, "A_STRLIT L%d\n", n->v.id); return;
    case A_IDENT:
      if (n->rvalue)
        fprintf(stdout, "A_IDENT rval %s\n", Gsym[n->v.id].name);
      else
        fprintf(stdout, "A_IDENT %s\n", Gsym[n->v.id].name);
      return;
    case A_ASSIGN:
      fprintf(stdout, "A_ASSIGN\n"); return;
    case A_WIDEN:
      fprintf(stdout, "A_WIDEN\n"); return;
    case A_RETURN:
      fprintf(stdout, "A_RETURN\n"); return;
    case A_FUNCCALL:
      fprintf(stdout, "A_FUNCCALL %s\n", Gsym[n->v.id].name); return;
    case A_ADDR:
      fprintf(stdout, "A_ADDR %s\n", Gsym[n->v.id].name); return;
    case A_DEREF:
      if (n->rvalue)
        fprintf(stdout, "A_DEREF rval\n");
      else
        fprintf(stdout, "A_DEREF\n");
      return;
    case A_SCALE:
      fprintf(stdout, "A_SCALE %d\n", n->v.size); return;
    default:
      fatald("Unknown dumpAST operator", n->op);
  }
}
