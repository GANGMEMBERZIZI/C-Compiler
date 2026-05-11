#include "defs.h"
#include "data.h"
#include "decl.h"
//将gen.c 的汇编指令写下out.s gcc 调用cpu生成可执行文件
static int freereg[4];//这个表示空闲的寄存器 1表示空 freereg[0]代表%r8.。。。。
static char *reglist[4]={"%r8","%r9","%r10","%r11"};//提供了这些寄存器在实际汇编代码中的名称 64位 long类型
static char *breglist[4] = { "%r8b", "%r9b", "%r10b", "%r11b" };//最低一个字节寄存器 char类型
static char *dreglist[4] = { "%r8d", "%r9d", "%r10d", "%r11d" };//32位 int 
void freeall_registers(){//将寄存器都标记寄存器正在使用中。
    freereg[0]=freereg[1]=freereg[2]=freereg[3]=1;
}
static int alloc_register(){//可用的寄存器列表，寻找第一个空闲的寄存器。一旦找到，它就会将该寄存器标记为“已使用”，并返回其索引。如果所有寄存器都已被占用，它将打印错误消息并终止程序
    for(int i=0;i<4;i++){
        if(freereg[i]){
            freereg[i]=0;
            return i;
        }
    }
    fatal("Out of registers");
    exit(1);
}
static void free_register(int reg){  //释放非空闲的寄存器
    if(freereg[reg]!=0){
        fatald("Error trying to free register", reg);
    }
    freereg[reg]=1;
}
void cgpreamble() {//这是在预装驱动程序。它手动写死了一段汇编代码，让你的编译器生成的程序天生就拥有“打印整数”的能力，而不需要用户自己去实现复杂的 I/O 操作。
  freeall_registers();
  fputs("\t.text\n", Outfile);
}
void cgpostamble(){

}
void cgfuncpreamble(int id) {//搭建栈帧
  char *name = Gsym[id].name;
  fprintf(Outfile,
	  "\t.text\n"//告诉汇编器，接下来的内容是 代码（指令），请把它放在代码段（Text Segment）。
	  "\t.globl\t%s\n"
	  "\t.type\t%s, @function\n"//函数
	  "%s:\n" "\tpushq\t%%rbp\n"
	  "\tmovq\t%%rsp, %%rbp\n", name, name, name);//第一个name入口  pushq保存旧的栈底指针 movq 建立新的栈底
}
void cgfuncpostamble(int id) {
  if (Gsym[id].type == P_VOID) {
    fprintf(Outfile, "\tmovl\t$0, %%eax\n");  // 显式设置返回值为 0
  }
  cglabel(Gsym[id].endlabel);//生成结束标签
  fputs("\tpopq %rbp\n" "\tret\n", Outfile);//清理函数内存
}
int cgloadint(int value,int type){ //value 加载到寄存器的常量数值
    int r=alloc_register(); //找到空闲的寄存器
    fprintf(Outfile,"\tmovq\t$%d,%s\n",value,reglist[r]);//\t制表符是一个转义序列，表示一个制表符（tab）。在汇编代码中，通常用制表符来缩进指令，使其更易读 ，movq: 这是要生成的汇编指令的名称。movq 是 x86-64 架构下的一条指令，含义是 "move quadword"（移动一个四字），通常用于将一个64位的值从源操作数移动到目的操作数。
    //在 x86 汇编语法（通常是 AT&T 语法）中，$ 符号前缀表示紧随其后的数字是一个 立即数（immediate value）。这意味着这个数字是指令本身的一部分，而不是内存地址或寄存器内容。%d 就是传入的value值 把这个写入out.s文件里 like 10=>  movq 10 ,%r8
    return r; //返回存储寄存器的索引号
}
int cgloadglob(int id) {//把变量从内存读入寄存器（Read）。
  int r = alloc_register();
  switch (Gsym[id].type) {
    case P_CHAR:
      fprintf(Outfile, "\tmovzbq\t%s(\%%rip), %s\n", Gsym[id].name,
              reglist[r]);
      break;
    case P_INT:
      fprintf(Outfile, "\tmovslq\t%s(\%%rip), %s\n", Gsym[id].name,
              reglist[r]);
      break;
    case P_LONG:
    case P_CHARPTR:
    case P_INTPTR:
    case P_LONGPTR:
      fprintf(Outfile, "\tmovq\t%s(\%%rip), %s\n", Gsym[id].name, reglist[r]);
      break;
    default:
      fatald("Bad type in cgloadglob:", Gsym[id].type);
  }
  return r;
}
int cgloadglobstr(int id){//给定一个字符串标签号 将其地址加载到寄存器里
  int r=alloc_register();//分配空的寄存器
  fprintf(Outfile, "\tleaq\tL%d(\%%rip), %s\n", id, reglist[r]);//leaq加载有效地址 %rip 偏移量 相对寻址
  return r;
}
//movq移动值 leaq移动地址
int cgadd(int r1,int r2){//r1 r2 代表cgload 后存储数的寄存器的索引数
    fprintf(Outfile,"\taddq\t%s,%s\n",reglist[r1],reglist[r2]);
    free_register(r1);
    return r2;
}
int cgmul(int r1,int r2){
    fprintf(Outfile,"\timulq\t%s,%s\n",reglist[r1],reglist[r2]);
    free_register(r1);
    return r2;
}
int cgsub(int r1, int r2) {
  fprintf(Outfile, "\tsubq\t%s, %s\n", reglist[r2], reglist[r1]);
  free_register(r2);
  return(r1);
}
int cgdiv(int r1,int r2){//r1/r2
    fprintf(Outfile,"\tmovq\t%s,%%rax\n",reglist[r1]);
    //将被除数加载到 %rax 寄存器 在汇编指令中 被除数要移动到%rax寄存器
    fprintf(Outfile,"\tcqo\n");//使用idivq 指令是 需要将%rax的符号位拓展到%rdx 高64位rdx 低64位rax idivq 余数%rdx 商64%rax idivq 有符号 divq无符号
    fprintf(Outfile,"\tidivq\t%s\n",reglist[r2]);//知道了被除数，直接用idivq 除数
    fprintf(Outfile,"\tmovq\t%%rax,%s\n",reglist[r1]); //将rax商 移动到r1寄存器
    free_register(r2);//释放r2寄存器
    return r1;
}
void cgprintint(int r){
    fprintf(Outfile,"\tmovq\t%s,%%rdi\n",reglist[r]);//转移到rdi寄存器
    fprintf(Outfile,"\tcall\tprintint\n");//call 调用函数，调用printint函数 打印rdi寄存器
    free_register(r);//释放
}
int cgcall(int r,int id){
    int outr=alloc_register();//分配一个新的通用寄存器
  fprintf(Outfile, "\tmovq\t%s, %%rdi\n", reglist[r]);//rdi 输入寄存器
  fprintf(Outfile, "\tcall\t%s\n", Gsym[id].name);//把当前代码执行到的地址（返回地址）压入栈（Stack）中。CPU 跳转到 print 这个标签（Label）所在的内存地址去执行代码。 name是函数的名字
  fprintf(Outfile, "\tmovq\t%%rax, %s\n", reglist[outr]);//%rax 里的值搬运（movq）到我们自己分配的通用寄存器（outr）里保存起来
  free_register(r);
  return (outr);
}
int cgshlconst(int r, int val) {
  fprintf(Outfile, "\tsalq\t$%d, %s\n", val, reglist[r]);//sal 算数左移 q 64位
  return(r);//接着用这个寄存器 
}
int cgstorglob(int r, int id) {//把寄存器里的值写入内存（Write）。 写将内存中的变量值写入寄存器 再进行 add等操作 然后再写回内存
  switch (Gsym[id].type) {
    case P_CHAR:
      fprintf(Outfile, "\tmovb\t%s, %s(\%%rip)\n", breglist[r],
              Gsym[id].name); //%rip 下一条将要执行的指令在内存中的地址
      break;
    case P_INT:
      fprintf(Outfile, "\tmovl\t%s, %s(\%%rip)\n", dreglist[r],
              Gsym[id].name);
      break;
    case P_LONG:
    case P_CHARPTR:
    case P_INTPTR:
    case P_LONGPTR:
      fprintf(Outfile, "\tmovq\t%s, %s(\%%rip)\n", reglist[r], Gsym[id].name);
      break;
    default:
      fatald("Bad type in cgloadglob:", Gsym[id].type);
  }
  return (r);
}
//P_NONE, P_VOID, P_CHAR, P_INT, P_LONG
static int psize[] = { 0, 0, 1, 4, 8, 8, 8, 8, 8 };
int cgprimsize(int type){
    if(type<P_NONE||type>P_LONGPTR)
    fatal("Bad type in cgprimsize()");
    return psize[type];
}
void cgglobsym(int id) {
  int typesize;
  typesize = cgprimsize(Gsym[id].type);
  fprintf(Outfile, "\t.data\n" "\t.globl\t%s\n", Gsym[id].name);
  fprintf(Outfile, "%s:", Gsym[id].name);
  for(int i=0;i<Gsym[id].size;i++){//如果是 变量 size为1 数组循环输出长度  函数不会在data段
  switch(typesize) {
    case 1: fprintf(Outfile, "\t.byte\t0\n"); break;
    case 4: fprintf(Outfile, "\t.long\t0\n"); break;
    case 8: fprintf(Outfile, "\t.quad\t0\n"); break;
    default: fatald("Unknown typesize in cgglobsym: ", typesize);
  }
}
}
void cgglobstr(int l,char *strvalue){
  char *cptr;
  cglabel(l);
  for(cptr=strvalue;*cptr;cptr++){//判断结束条件 解引用*cptr 为空
    fprintf(Outfile, "\t.byte\t%d\n", *cptr);
  }
  fprintf(Outfile, "\t.byte\t0\n");//结束标签
}
static char *cmplist[]={ "sete", "setne", "setl", "setg", "setle", "setge" };
//用cmpq 比较两个寄存器所存的值 cmpq 指令本身不会将结果（%r9 - %r8 的值）存储到寄存器中。它只影响 CPU 的状态标志寄存器（FLAGS register）。
//然后再用setcc指令族 它们的作用是根据 CPU 之前的标志位状态，将目标寄存器的最低字节设置为 00000001（如果条件为真）或 00000000（如果条件为假）。
//但逻辑符号返回的是1或者0 例如1000 变成1001 是不符合要求的 要把前面的字节（8位)都置为0
//how 是传入的setcc 指令族类型 sete setnee等,breglist[r2]是cpu状态标志位的最低字节
 int cgcompare_and_set(int ASTop,int r1, int r2) {//计算布尔值
    if(ASTop<A_EQ||ASTop>A_GE){
        fatal("Bad ASTop in cgcompare_and_set()");
    }
  fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
  fprintf(Outfile, "\t%s\t%s\n",cmplist[ASTop-A_EQ], breglist[r2]);//获取对应的 SETcc 指令的助记符
  fprintf(Outfile, "\tmovzbq\t%s, %s\n", breglist[r2], reglist[r2]);//从 breglist[r2] 所指定的字节寄存器中读取 8 位数据。将这 8 位数据复制到 reglist[r2] 所指定的64 位寄存器的最低 8 位。将 reglist[r2] 的高 56 位（即除了最低字节以外的所有位）全部设置为零
  free_register(r1);
  return (r2);
}
void cglabel(int l){//生成汇编语言中的标签（Label）
    fprintf(Outfile,"L%d:\n",l);
}
void cgjump(int l){
    fprintf(Outfile, "\tjmp\tL%d\n", l);//jmp 跳转指令 ，L%d跳转标签
}
//in AST order: A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char *invcmplist[] = { "jne", "je", "jge", "jle", "jg", "jl" };
int cgcompare_and_jump(int ASTop,int r1,int r2,int label){
    if(ASTop<A_EQ||ASTop>A_GE){
        fatal("Bad ASTop in cgcompare_and_set()");
    }
    fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
    fprintf(Outfile, "\t%s\tL%d\n", invcmplist[ASTop - A_EQ], label);//整个表达式会生成一条汇编指令，例如 jne L10，意思是 R1 != R2 时跳转到 L10 (如果 ASTop 表示 ==)，或者 R1 >= R2 时跳转到 L10 (如果 ASTop 表示 <)。这正是实现高级语言中 if 语句逻辑所需要的。
    freeall_registers();
    return NOREG;
}
int cgwiden(int r,int oldtype,int newtype){
    return r;
}
void cgreturn (int reg,int id){//只在遇到 A_RETURN 节点时才调用
    switch (Gsym[id].type) {
      case P_VOID:
      fprintf(Outfile, "\tmovl\t$0, %%eax\n");
      break;
    case P_CHAR:
      fprintf(Outfile, "\tmovzbl\t%s, %%eax\n", breglist[reg]);
      break;
    case P_INT:
      fprintf(Outfile, "\tmovl\t%s, %%eax\n", dreglist[reg]);
      break;
    case P_LONG:
      fprintf(Outfile, "\tmovq\t%s, %%rax\n", reglist[reg]);
      break;
    default:
      fatald("Bad function type in cgreturn:", Gsym[id].type);
  }
  cgjump(Gsym[id].endlabel);
}
int cgaddress(int id) {
  int r = alloc_register();

  fprintf(Outfile, "\tleaq\t%s(%%rip), %s\n", Gsym[id].name, reglist[r]);//leap 计算源操作数的地址，并将其加载到目标寄存器中   它不读取内存中的值，只计算地址
  return (r);
}
int cgderef(int r, int type) {//读取 y=*p
  switch (type) {
    case P_CHARPTR:
      fprintf(Outfile, "\tmovzbq\t(%s), %s\n", reglist[r], reglist[r]);//去地址 (%s) 处读取 8 个字节 (64-bit) 读取值 然后覆盖原寄存器
      break;
    case P_INTPTR:
      fprintf(Outfile, "\tmovq\t(%s), %s\n", reglist[r], reglist[r]);
      break;
    case P_LONGPTR:
      fprintf(Outfile, "\tmovq\t(%s), %s\n", reglist[r], reglist[r]);
      break;
    default:
      fatald("Can't cgderef on type:", type);
  }
  return (r);
}
int cgstorderef(int r1 ,int r2 ,int type){
  switch(type){
    case P_CHAR://char 只有一个字节 数据用 bre就行
    fprintf(Outfile,"\tmovb\t%s ,(%s)\n",breglist[r1],reglist[r2]);//括号表示 间接寻址  把r1的值 存在 reglist[r2]所在的内存地址 r1 数据源
    break;
    case P_INT:
    fprintf(Outfile, "\tmovq\t%s, (%s)\n", reglist[r1], reglist[r2]);
    break;
    case P_LONG:
    fprintf(Outfile, "\tmovq\t%s, (%s)\n", reglist[r1], reglist[r2]);
    break;
    default:
    fatald("Can't cgstoderef on type:", type);
  }
  return r1;//返回值 r1存的是值 r2是地址
}

// char	movzbl	读1字节，扩展到4字节，高位自动清零
// int	movl	读4字节，写4字节，高位自动清零
// long	movq	读8字节，写8字节

// .bss	存放没初始值的全局变量（自动清零）	可读、可写
// Stack/Heap	存放局部变量和动态申请的内存	可读、可写
// .data	存放有初始值的全局变量	可读、可写
// .text	存放代码（函数指令）	只读、可执行