#include "defs.h"
#include "data.h"
#include "decl.h"
//将生成的树生成汇编码
int genlabel(){//调用 genlabel() 函数生成一个唯一的数字标签，这个标签将用于标记 if 条件为假时应该跳转到的位置。这可能对应 else 分支的开头，或者如果没有 else 分支，则对应 if 语句的结束。
  static int id=1;
  return (id++);
}
static int genIF(struct ASTnode *n){
  int Lfalse,Lend;
  Lfalse=genlabel();//Lfalse标记了else的开始 或者if的结尾
  if(n->right){
    Lend=genlabel();//标记if-else的最终结束点
  }
  genAST(n->left,Lfalse,n->op);//判断条件是否正确//如果是真 直接执行下面的 如果为假 直接跳到else的开始
  genfreeregs();//释放计算condition的寄存器
  genAST(n->mid,NOLABEL,n->op);//NOREG语句块 不用返回计算任何值 此处无需关心寄存器结果
  genfreeregs();//释放true statement计算的
  if(n->right){//如果条件判断正确并且有else语句 直接跳到if-else结束
    cgjump(Lend);
  }
  cglabel(Lfalse);//在汇编代码中当前位置放置 Lfalse 标签 在 genAST(n->left, Lfalse, n->op); 内部，当条件为假时会生成类似 JLE Lfalse_ASM_Label 这样的条件跳转指令。
  if(n->right){//条件为假并且有else跳转到此
    genAST(n->right,NOLABEL,n->op);
    genfreeregs();//释放寄存器
    cglabel(Lend);//在这里放置一个叫做 Lend 的汇编标签，它标志着当前的 if-else 语句块的逻辑结束点。
  }
  return NOREG;
}
static int genWHILE(struct ASTnode *n){
  int Lstart,Lend;
  Lstart=genlabel();
  Lend=genlabel();
  cglabel(Lstart);//在当前汇编输出中生成一个实际的汇编标签,这个标签标记了 WHILE 循环的起始点，即每次循环迭代时都会跳回这里来检查条件。
  genAST(n->left,Lend,n->op);//判断条件是否正确//如果是真 直接执行下面的 如果为假 直接跳到else的开始
  genfreeregs();
  genAST(n->right,NOLABEL,n->op);//如果循环条件为真，程序会顺序执行这里的循环体代码。
  genfreeregs();
  cgjump(Lstart);//返回开始处 进行新的判断
  cglabel(Lend);//这意味着当条件为假时跳转到 Lend 标签后，程序会从这里继续执行，这也就是循环结束后的代码。
  return NOREG;
}
static int gen_funccall(struct ASTnode* n){
  struct ASTnode* gluetree=n->left;//消耗 A_FUNCCALL 节点
  int reg;
  int numargs=0;//函数参数的个数
  while(gluetree){
    reg=genAST(gluetree->right,NOLABEL,gluetree->op);//解析expr 返回寄存器编号
    cgcopyarg(reg, gluetree->v.size);
    if(numargs==0)
    numargs=gluetree->v.size;//函数参数的个数
    genfreeregs();
    gluetree=gluetree->left;
  }
  return cgcall(n->v.id,numargs);
} 
 int genAST(struct ASTnode *n,int label,int parentASTop){  //将高级语言代码一次性地翻译成目标机器的汇编代码或机器代码，然后生成一个可执行文件
    //代码不是传递值，而是传递 注册标识符。例如，将值加载到寄存器中，然后 返回具有加载值的寄存器的标识。
    int leftreg,rightreg;
    if (n==NULL) return NOREG;
    switch (n->op)
    {
      case A_IF:
      return genIF(n);
      case A_WHILE:
      return genWHILE(n);
      case A_GLUE:
      genAST(n->left,NOLABEL,n->op);//分析每一个孩子 然后释放寄存器
      genfreeregs();
      genAST(n->right,NOLABEL,n->op);
      genfreeregs();
      return NOREG;
      case A_FUNCTION:
      cgfuncpreamble(n->v.id);//导出函数名 将基准指针 压入栈中 将 sp 赋给基准指针 开辟空间
      genAST(n->left,NOLABEL,n->op);
      cgfuncpostamble(n->v.id);//清理栈帧
      return NOREG;
    }
    if(n->left)
    leftreg=genAST(n->left,NOLABEL,n->op);//存放子表达式结果的寄存器编号
    if(n->right)
    rightreg=genAST(n->right,NOLABEL,n->op);
    switch (n->op)
    {
    case A_ADD:
    return cgadd(leftreg,rightreg);
    case A_SUBTRACT:
    return cgsub(leftreg,rightreg);
    case A_MULTIPLY:
    return cgmul(leftreg,rightreg);
    case A_DIVIDE:
    return cgdiv(leftreg,rightreg);
    case A_AND:
    return (cgand(leftreg, rightreg));
    case A_OR:
    return (cgor(leftreg, rightreg));
    case A_XOR:
    return (cgxor(leftreg, rightreg));
    case A_LSHIFT:
    return (cgshl(leftreg, rightreg));
    case A_RSHIFT:
    return (cgshr(leftreg, rightreg));
    case A_EQ://比较运算符
    case A_NE:
    case A_LT:
    case A_GT:
    case A_LE:
    case A_GE:
    if (parentASTop==A_IF||parentASTop==A_WHILE){//如果父亲节点是 if while 需要跳转
      return (cgcompare_and_jump(n->op,leftreg,rightreg,label));//	根据比较结果，直接控制程序跳转。
    }
    else{//将比较结果（真/假）表示为数值（1/0），存入寄存器。
      return (cgcompare_and_set(n->op, leftreg, rightreg));//赋值语句、函数参数、算术表达式等需要布尔值参与计算的地方
    }
    case A_INTLIT:
    return (cgloadint(n->v.intvalue,n->type));
    case A_STRLIT:
    return (cgloadglobstr(n->v.id));
    case A_IDENT://处理 已知变量a的值
    if (n->rvalue||parentASTop==A_DEREF)//如果需要加载值 或者 正在被解引用
    if(Symtable[n->v.id].class == C_LOCAL||Symtable[n->v.id].class == C_PARAM){//局部变量或参数
      return cgloadlocal(n->v.id, n->op);
    }else{
    return (cgloadglob(n->v.id,n->op));//则是为从全局变量中读取值生成汇编代码 例如在 y = x + 5; 中读取 x 的值
    }
    else 
    return (NOREG);
    case A_ASSIGN://处理 类似 a=3+5;
    switch(n->right->op){
      case A_IDENT: 
      if (Symtable[n->right->v.id].class == C_GLOBAL)
	    return (cgstorglob(leftreg, n->right->v.id));
	  else
	    return (cgstorlocal(leftreg, n->right->v.id));//leftreg是 等号右边表达式算出来的“值”所在的寄存器编号
      //因为 expr 左右树互换了  n->right->v.id是左边变量
      case A_DEREF: return (cgstorderef(leftreg, rightreg, n->right->type));//处理 *p=y 写入 leftreg是右边的值 rightreg是p内存地址 assign号左右子树交换了
      default: fatald("Can't A_ASSIGN in genAST(), op", n->op);
    }
    case A_WIDEN:
    return (cgwiden(leftreg,n->left->type,n->type));
    case A_RETURN:
    cgreturn(leftreg,Functionid);
    return NOREG;//不需要返回值
    case A_FUNCCALL:
    return gen_funccall(n);
    case A_ADDR:
    return (cgaddress(n->v.id));
    case A_DEREF:
    if(n->rvalue){//如果是右值 类似y=*p 这样的
      return cgderef(leftreg,n->left->type);//leftreg 是p内存地址  type 指向传入的指针的数据是多大
    }else{//如果是左值 类似 *p=10 这样的
      return leftreg;//leftreg保留 p的内存地址
    }
    case A_SCALE://没有右子树
    switch (n->v.size)//收缩的size total=index * size
    {
    case 2://移位的效率比乘法高
      return cgshlconst(leftreg,1);//左移一位 相当于x2 leftreg存储了偏移变量index
    case 4:
    return cgshlconst(leftreg,2);//左移两位
    case 8:
    return cgshlconst(leftreg,3);//左移3位
    default://处理不是2的幂的情况 类似结构体 不能移位了 只能乘
    rightreg= cgloadint(n->v.size, P_INT);//偏移变量   P_INT请分配一个标准的、全尺寸的通用寄存器（比如 64 位的），把这个数字（size）放进去。因为我马上要拿它跟另一个全尺寸的寄存器做乘法了，别给我搞个 8 位的小寄存器来添乱
     return (cgmul(leftreg, rightreg));
    }
    case A_POSTINC:
      return (cgloadglob(n->v.id, n->op));
    case A_POSTDEC:
      return (cgloadglob(n->v.id, n->op));
    case A_PREINC://前缀 在left里
      return (cgloadglob(n->left->v.id, n->op));
    case A_PREDEC:
      return (cgloadglob(n->left->v.id, n->op));
    case A_NEGATE:
      return (cgnegate(leftreg));
    case A_INVERT:
      return (cginvert(leftreg));
    case A_LOGNOT:
      return (cglognot(leftreg));
    case A_TOBOOL:
      return (cgboolean(leftreg, parentASTop, label));
    default:
    fatald("Unknown AST operator", n->op);    
    }
}
void genpreamble() {
  cgpreamble();
}
void genpostamble() {
  cgpostamble();
}
void genfreeregs() {
  freeall_registers();
}
void genglobsym(int id){ 
  cgglobsym(id);//汇编层面声明全局变量，并在数据段中为其预留空间。这通常在编译器的声明分析阶段完成。
}
int genglobstr(char *strvalue){
  int l=genlabel();//生成标签
  cgglobstr(l,strvalue);
  return l;
}
int genprimsize(int type) {
  return (cgprimsize(type));
}
