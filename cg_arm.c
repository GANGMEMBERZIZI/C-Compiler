#include "defs.h"
#include "data.h"
#include "decl.h"
static int freereg[4];
static char *reglist[4] = { "r4", "r5", "r6", "r7" };
void freeall_registers(void) {
  freereg[0] = freereg[1] = freereg[2] = freereg[3] = 1;
}
static int alloc_register(void){
  for (int i = 0; i < 4; i++) {
    if (freereg[i]) {
      freereg[i] = 0;
      return (i);
    }
  }
  fatal("Out of registers");
  return NOREG;
}
static void free_register(int reg) {
  if (freereg[reg] != 0)
    fatald("Error trying to free register", reg);
  freereg[reg] = 1;
}
#define MAXINTS 1024
int Intlist[MAXINTS];//存储具体数值的数组
static int Intslot=0;//计数器，记录当前仓库里已经存了多少个数
static void set_int_offset(int val){
    int offset=1;
    for(int i=0;i<Intslot;i++){//查重 
        if(Intlist[i]==val){
            offset=4*i;//在 32 位 ARM 中，每个整数地址占 4 字节。第 i 个数的地址偏移量自然就是 i 乘以 4。
            break;
        }
    }
    if (offset==1){
        offset=4*Intslot;//新的offset
        if (Intslot == MAXINTS)
      fatal("Out of int slots in set_int_offset()");
      Intlist[Intslot++] = val;//把新数字存进数组，并把计数器 +1
      fprintf(Outfile, "\tldr\tr3, .L3+%d\n", offset);//读取数据的值
    }
}
void cgpreamble() {
  freeall_registers();
  fputs("\t.text\n", Outfile);
}
void cgpostamble(){
    fprintf(Outfile,".L2:\n");//生成标签 下面用来存放变量地址
    for(int i=0;i<Globs;i++){
        if (Gsym[i].stype==S_VARIABLE || Gsym[i].stype==S_ARRAY)
            fprintf(Outfile,"\t.word %s\n",Gsym[i].name);
    }
    fprintf(Outfile, ".L3:\n");//生成标签 存变量值的内存
  for (int i = 0; i < Intslot; i++) {
    fprintf(Outfile, "\t.word %d\n", Intlist[i]);
  }
}
void cgfuncpreamble(int id) {
  char *name = Gsym[id].name;
  fprintf(Outfile,
	  "\t.text\n"///切换到代码段（存放指令的地方）
	  "\t.globl\t%s\n"//全局函数
	  "\t.type\t%s, \%%function\n"//是一个函数
	  "%s:\n" "\tpush\t{fp, lr}\n"//  lr 存储函数执行完后该回哪去 fp存着“上一级函数的栈底”。保存它是为了以后能还原上一级的环境。
	  "\tadd\tfp, sp, #4\n"//sp 栈指针 指向栈顶 越来越小 fp帧指针 固定不动的基准点 用offset来处理局部变量
	  "\tsub\tsp, sp, #8\n" "\tstr\tr0, [fp, #-8]\n", name, name, name);//1,预留8个字节好习惯 把r0寄存器的值 入内存
}
void cgfuncpostamble(int id) {//函数结语
  if (Gsym[id].type == P_VOID) {
    fprintf(Outfile, "\tmov\tr0, #0\n");
  }
  cglabel(Gsym[id].endlabel);//生成结束标签
  fputs("\tsub\tsp, fp, #4\n" "\tpop\t{fp, pc}\n" "\t.align\t2\n", Outfile);//借fp找回sp    pop出栈 align 对齐
  //函数运行中 fp不动 运行新函数要动  
}
int cgloadint(int value, int type) {
  int r = alloc_register();

  if (value <= 1000)//指令最多只能1023
    fprintf(Outfile, "\tmov\t%s, #%d\n", reglist[r], value);
  else {//如果太大 就存内存
    set_int_offset(value);
    fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);//将内存-> 寄存器
  }
  return (r);
}
static void set_var_offset(int id){
    int offset=0;
    for (int i=0;i<id;i++){
        if(Gsym[i].stype==S_VARIABLE){
            offset+=4;
        }  
    }
    fprintf(Outfile,"\tldr\tr3, .L2+%d\n",offset);
}
int cgloadglob(int id) {
  int r = alloc_register();
  set_var_offset(id);
  switch (Gsym[id].type) {
  case P_CHAR:
    fprintf(Outfile, "\tldrb\t%s, [r3]\n", reglist[r]);
    break;
  default:
    fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
    break;
  }
  return (r);
}

int cgadd(int r1, int r2) {
  fprintf(Outfile, "\tadd\t%s, %s, %s\n", reglist[r2], reglist[r1],
	  reglist[r2]);
  free_register(r1);
  return (r2);
}
int cgsub(int r1, int r2) {
  fprintf(Outfile, "\tsub\t%s, %s, %s\n", reglist[r1], reglist[r1],
	  reglist[r2]);
  free_register(r2);
  return (r1);
}

int cgmul(int r1, int r2) {
  fprintf(Outfile, "\tmul\t%s, %s, %s\n", reglist[r2], reglist[r1],
	  reglist[r2]);
  free_register(r1);
  return (r2);
}

int cgdiv(int r1, int r2) {

  fprintf(Outfile, "\tmov\tr0, %s\n", reglist[r1]);
  fprintf(Outfile, "\tmov\tr1, %s\n", reglist[r2]);
  fprintf(Outfile, "\tbl\t__aeabi_idiv\n");//aeabi这是一套标准，规定了ARM程序之间怎么传递参数、怎么返回值。
  fprintf(Outfile, "\tmov\t%s, r0\n", reglist[r1]);
  free_register(r2);
  return (r1);
}

void cgprintint(int r) {
  fprintf(Outfile, "\tmov\tr0, %s\n", reglist[r]);
  fprintf(Outfile, "\tbl\tprintint\n");
  fprintf(Outfile, "\tnop\n");
  free_register(r);
}

int cgcall(int r, int id) {
  fprintf(Outfile, "\tmov\tr0, %s\n", reglist[r]);
  fprintf(Outfile, "\tbl\t%s\n", Gsym[id].name);//连接跳转 函数跳过去，还得记路回来
  fprintf(Outfile, "\tmov\t%s, r0\n", reglist[r]);
  return (r);
}
int cgshlconst(int r, int val) {
  fprintf(Outfile, "\tlsl\t%s, %s, #%d\n", reglist[r], reglist[r], val);//lsl 左移  arm写常数要加# 写两个寄存器 是因为arm3 操作数指令 不会覆盖!
  return(r);//接着用这个寄存器  
}
int cgstorglob(int r, int id) {//把寄存器里的数据，写回到全局变量的内存地址中去

  set_var_offset(id);//调用辅助函数计算全局变量的地址

  switch (Gsym[id].type) {
  case P_CHAR:
    fprintf(Outfile, "\tstrb\t%s, [r3]\n", reglist[r]);//存一个字节
    break;
  case P_INT:
  case P_LONG:
  case P_CHARPTR:
  case P_INTPTR:
  case P_LONGPTR:
    fprintf(Outfile, "\tstr\t%s, [r3]\n", reglist[r]);//寄存器 --> 内存
    break;
  default:
    fatald("Bad type in cgloadglob:", Gsym[id].type);
  }
  return (r);
}

static int psize[] = { 0, 0, 1, 4, 4,4,4,4};

int cgprimsize(int type) {
  if (type < P_NONE || type > P_LONGPTR)
    fatal("Bad type in cgprimsize()");
  return (psize[type]);
}

int cgloadglobstr(int id) {
  int r = alloc_register();
  fprintf(Outfile, "\tldr\t%s, =L%d\n", reglist[r], id);
  return r;
}

void cglabel(int l);

void cgglobstr(int l, char *strvalue) {
  char *cptr;
  cglabel(l);
  for (cptr = strvalue; *cptr; cptr++) {
    fprintf(Outfile, "\t.byte\t%d\n", *cptr);
  }
  fprintf(Outfile, "\t.byte\t0\n");
}

void cgglobsym(int id) {
  int typesize;
  typesize = cgprimsize(Gsym[id].type);

  fprintf(Outfile, "\t.data\n" "\t.globl\t%s\n", Gsym[id].name);
  fprintf(Outfile, "%s:", Gsym[id].name);
  for(int i=0; i<Gsym[id].size; i++){//循环输出数组的每个元素空间
    switch(typesize) {
      case 1: fprintf(Outfile, "\t.byte\t0\n"); break;
      case 4: fprintf(Outfile, "\t.long\t0\n"); break;
      default: fatald("Unknown typesize in cgglobsym: ", typesize);
    }
  }
}

// A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char *cmplist[] =
  { "moveq", "movne", "movlt", "movgt", "movle", "movge" };

//  A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char *invcmplist[] =
  { "movne", "moveq", "movge", "movle", "movgt", "movlt" };

int cgcompare_and_set(int ASTop, int r1, int r2) {

  if (ASTop < A_EQ || ASTop > A_GE)
    fatal("Bad ASTop in cgcompare_and_set()");

  fprintf(Outfile, "\tcmp\t%s, %s\n", reglist[r1], reglist[r2]);
  fprintf(Outfile, "\t%s\t%s, #1\n", cmplist[ASTop - A_EQ], reglist[r2]);
  fprintf(Outfile, "\t%s\t%s, #0\n", invcmplist[ASTop - A_EQ], reglist[r2]);
  fprintf(Outfile, "\tuxtb\t%s, %s\n", reglist[r2], reglist[r2]);
  free_register(r1);
  return (r2);
}

void cglabel(int l) {//生成标签
  fprintf(Outfile, "L%d:\n", l);
}

void cgjump(int l) {
  fprintf(Outfile, "\tb\tL%d\n", l);//if-else 跳转，while 循环，goto 跳过去就不管了
}

// A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char *brlist[] = { "bne", "beq", "bge", "ble", "bgt", "blt" };

int cgcompare_and_jump(int ASTop, int r1, int r2, int label) {

  if (ASTop < A_EQ || ASTop > A_GE)
    fatal("Bad ASTop in cgcompare_and_set()");

  fprintf(Outfile, "\tcmp\t%s, %s\n", reglist[r1], reglist[r2]);
  fprintf(Outfile, "\t%s\tL%d\n", brlist[ASTop - A_EQ], label);
  freeall_registers();
  return (NOREG);
}
int cgwiden(int r, int oldtype, int newtype) {//拓宽
  return (r);
}
void cgreturn(int reg, int id) {
  fprintf(Outfile, "\tmov\tr0, %s\n", reglist[reg]);
  cgjump(Gsym[id].endlabel);
}
int cgaddress(int id) {
  int r = alloc_register();
  set_var_offset(id);
  fprintf(Outfile, "\tmov\t%s, r3\n", reglist[r]);
  return (r);
}
int cgderef(int r, int type) {
  switch (type) {
  case P_CHARPTR:
    fprintf(Outfile, "\tldrb\t%s, [%s]\n", reglist[r], reglist[r]);
    break;
  case P_INTPTR:
    fprintf(Outfile, "\tldr\t%s, [%s]\n", reglist[r], reglist[r]);
    break;
  case P_LONGPTR:
    fprintf(Outfile, "\tldr\t%s, [%s]\n", reglist[r], reglist[r]);
    break;
  }
  return (r);
}
int cgstorderef(int r1, int r2, int type) {
  switch (type) {
    case P_CHAR:
      fprintf(Outfile, "\tstrb\t%s, [%s]\n", reglist[r1], reglist[r2]);//方括号 间接寻址
      break;
    case P_INT:
    case P_LONG:
      fprintf(Outfile, "\tstr\t%s, [%s]\n", reglist[r1], reglist[r2]);
      break;
    default:
      fatald("Can't cgstoderef on type:", type);
  }
  return (r1);
}
