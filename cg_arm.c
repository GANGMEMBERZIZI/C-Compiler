#include "defs.h"
#include "data.h"
#include "decl.h"

// Segment management
enum { no_seg, text_seg, data_seg } currSeg = no_seg;

void cgtextseg() {
  if (currSeg != text_seg) {
    fputs("\t.text\n", Outfile);
    currSeg = text_seg;
  }
}

void cgdataseg() {
  if (currSeg != data_seg) {
    fputs("\t.data\n", Outfile);
    currSeg = data_seg;
  }
}

// Local variable stack offset tracking
static int localOffset;
static int stackOffset;

void cgresetlocals(void) {
  localOffset = 0;
}

static int newlocaloffset(int type) {
  localOffset += (cgprimsize(type) > 4) ? cgprimsize(type) : 4;
  return (-localOffset);
}

// Register management
#define NUMFREEREGS 4
#define FIRSTPARAMREG 7
static int freereg[NUMFREEREGS];
static char *reglist[] = { "r4", "r5", "r6", "r7",   // 4 个自由劳工（打杂计算用）
    "r3", "r2", "r1", "r0" };// 4 个传参通道（倒序排列）

void freeall_registers(void) {
  freereg[0] = freereg[1] = freereg[2] = freereg[3] = 1;
}

static int alloc_register() {
  for (int i = 0; i < NUMFREEREGS; i++) {
    if (freereg[i]) {
      freereg[i] = 0;
      return i;
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

void cgpreamble() {
  freeall_registers();
}

void cgpostamble() {
}

void cgfuncpreamble(int id) {
  char *name = Symtable[id].name;
  int i;
  int paramOffset = 24;//ARM 压栈了 6 个寄存器 (r4-r7, fp, lr)，共 6 * 4 = 24 字节。
  int paramReg = FIRSTPARAMREG;
  cgtextseg();
  localOffset= 0;
  fprintf(Outfile,
    "\t.global\t%s\n"
    "\t.type\t%s, %%function\n"   // @在某些ARM汇编器里是注释符，改用 %function 更安全
    "%s:\n"
    "\tpush\t{r4, r5, r6, r7, fp, lr}\n" // 保护打杂劳工、备份老 fp 和返回地址 lr
    "\tmov\tfp, sp\n", name, name, name);
    for (i = NSYMBOLS - 1; i > Locls; i--) {
    if (Symtable[i].class != C_PARAM)
      break;
    if (i < NSYMBOLS - 4) // x86是6个，ARM 硬件限制最多只有 4 个寄存器传参！
      break;
    Symtable[i].posn = newlocaloffset(Symtable[i].type);
    cgstorlocal(paramReg--, i); // 生成类似 str r0, [fp, #-8] 的代码
  }
  for (; i > Locls; i--) {
    if (Symtable[i].class == C_PARAM) {
      Symtable[i].posn = paramOffset;
      paramOffset += 4; // x86是8字节，ARM32 作为 32 位系统，参数槽是 4 字节对齐！
    } else {
      Symtable[i].posn = newlocaloffset(Symtable[i].type);
    }
  }
  stackOffset = (localOffset + 7) & ~7;//8字节对齐 x86是16
  if (stackOffset > 0) {
    fprintf(Outfile, "\tsub\tsp, sp, #%d\n", stackOffset);
}
  }

void cgfuncpostamble(int id) {
  cglabel(Symtable[id].endlabel);
  fprintf(Outfile, "\tmov\tsp, fp\n");
  fprintf(Outfile, "\tpop\t{r4, r5, r6, r7, fp, pc}\n");
}

int cgloadint(int value, int type) {
  int r = alloc_register();
  if (value >= 0 && value <= 255) {
    fprintf(Outfile, "\tmov\t%s, #%d\n", reglist[r], value);
  } else {
    fprintf(Outfile, "\tldr\t%s, =%d\n", reglist[r], value);
  }
  return r;
}

int cgloadglob(int id, int op) {
  int r = alloc_register();

  switch (Symtable[id].type) {
    case P_CHAR:
      if (op == A_PREINC) {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldrb\t%s, [r3]\n", reglist[r]);
        fprintf(Outfile, "\tadd\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstrb\t%s, [r3]\n", reglist[r]);
      } else if (op == A_PREDEC) {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldrb\t%s, [r3]\n", reglist[r]);
        fprintf(Outfile, "\tsub\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstrb\t%s, [r3]\n", reglist[r]);
      } else if (op == A_POSTINC) {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldrb\t%s, [r3]\n", reglist[r]);
        fprintf(Outfile, "\tadd\tr0, %s, #1\n", reglist[r]);
        fprintf(Outfile, "\tstrb\tr0, [r3]\n");
      } else if (op == A_POSTDEC) {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldrb\t%s, [r3]\n", reglist[r]);
        fprintf(Outfile, "\tsub\tr0, %s, #1\n", reglist[r]);
        fprintf(Outfile, "\tstrb\tr0, [r3]\n");
      } else {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldrb\t%s, [r3]\n", reglist[r]);
      }
      break;
    case P_INT:
      if (op == A_PREINC) {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
        fprintf(Outfile, "\tadd\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstr\t%s, [r3]\n", reglist[r]);
      } else if (op == A_PREDEC) {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
        fprintf(Outfile, "\tsub\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstr\t%s, [r3]\n", reglist[r]);
      } else if (op == A_POSTINC) {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
        fprintf(Outfile, "\tadd\tr0, %s, #1\n", reglist[r]);
        fprintf(Outfile, "\tstr\tr0, [r3]\n");
      } else if (op == A_POSTDEC) {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
        fprintf(Outfile, "\tsub\tr0, %s, #1\n", reglist[r]);
        fprintf(Outfile, "\tstr\tr0, [r3]\n");
      } else {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
      }
      break;
    case P_LONG:
    case P_CHARPTR:
    case P_INTPTR:
    case P_LONGPTR:
      if (op == A_PREINC) {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
        fprintf(Outfile, "\tadd\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstr\t%s, [r3]\n", reglist[r]);
      } else if (op == A_PREDEC) {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
        fprintf(Outfile, "\tsub\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstr\t%s, [r3]\n", reglist[r]);
      } else if (op == A_POSTINC) {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
        fprintf(Outfile, "\tadd\tr0, %s, #1\n", reglist[r]);
        fprintf(Outfile, "\tstr\tr0, [r3]\n");
      } else if (op == A_POSTDEC) {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
        fprintf(Outfile, "\tsub\tr0, %s, #1\n", reglist[r]);
        fprintf(Outfile, "\tstr\tr0, [r3]\n");
      } else {
        fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
        fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
      }
      break;
    default:
      fatald("Bad type in cgloadglob:", Symtable[id].type);
  }
  return r;
}

int cgloadlocal(int id, int op) {
  int r = alloc_register();

  switch (Symtable[id].type) {
    case P_CHAR:
      if (op == A_PREINC) {
        fprintf(Outfile, "\tldrb\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tadd\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstrb\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
      } else if (op == A_PREDEC) {
        fprintf(Outfile, "\tldrb\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tsub\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstrb\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
      } else if (op == A_POSTINC) {
        fprintf(Outfile, "\tldrb\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tadd\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstrb\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tsub\t%s, %s, #1\n", reglist[r], reglist[r]);
      } else if (op == A_POSTDEC) {
        fprintf(Outfile, "\tldrb\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tsub\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstrb\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tadd\t%s, %s, #1\n", reglist[r], reglist[r]);
      } else {
        fprintf(Outfile, "\tldrb\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
      }
      break;
    case P_INT:
      if (op == A_PREINC) {
        fprintf(Outfile, "\tldr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tadd\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
      } else if (op == A_PREDEC) {
        fprintf(Outfile, "\tldr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tsub\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
      } else if (op == A_POSTINC) {
        fprintf(Outfile, "\tldr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tadd\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tsub\t%s, %s, #1\n", reglist[r], reglist[r]);
      } else if (op == A_POSTDEC) {
        fprintf(Outfile, "\tldr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tsub\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tadd\t%s, %s, #1\n", reglist[r], reglist[r]);
      } else {
        fprintf(Outfile, "\tldr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
      }
      break;
    case P_LONG:
    case P_CHARPTR:
    case P_INTPTR:
    case P_LONGPTR:
      if (op == A_PREINC) {
        fprintf(Outfile, "\tldr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tadd\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
      } else if (op == A_PREDEC) {
        fprintf(Outfile, "\tldr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tsub\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
      } else if (op == A_POSTINC) {
        fprintf(Outfile, "\tldr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tadd\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tsub\t%s, %s, #1\n", reglist[r], reglist[r]);
      } else if (op == A_POSTDEC) {
        fprintf(Outfile, "\tldr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tsub\t%s, %s, #1\n", reglist[r], reglist[r]);
        fprintf(Outfile, "\tstr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
        fprintf(Outfile, "\tadd\t%s, %s, #1\n", reglist[r], reglist[r]);
      } else {
        fprintf(Outfile, "\tldr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
      }
      break;
    default:
      fatald("Bad type in cgloadlocal:", Symtable[id].type);
  }
  return r;
}

int cgloadglobstr(int id) {
  int r = alloc_register();
  fprintf(Outfile, "\tldr\t%s, =L%d\n", reglist[r], id);
  return r;
}

int cgadd(int r1, int r2) {
  fprintf(Outfile, "\tadd\t%s, %s, %s\n", reglist[r2], reglist[r1], reglist[r2]);
  free_register(r1);
  return r2;
}

int cgmul(int r1, int r2) {
  fprintf(Outfile, "\tmul\t%s, %s, %s\n", reglist[r2], reglist[r1], reglist[r2]);
  free_register(r1);
  return r2;
}

int cgsub(int r1, int r2) {
  fprintf(Outfile, "\tsub\t%s, %s, %s\n", reglist[r1], reglist[r1], reglist[r2]);
  free_register(r2);
  return r1;
}

int cgdiv(int r1, int r2) {
  fprintf(Outfile, "\tmov\tr0, %s\n", reglist[r1]);
  fprintf(Outfile, "\tmov\tr1, %s\n", reglist[r2]);
  fprintf(Outfile, "\tbl\t__aeabi_idiv\n");
  fprintf(Outfile, "\tmov\t%s, r0\n", reglist[r1]);
  free_register(r2);
  return r1;
}

int cgand(int r1, int r2) {
  fprintf(Outfile, "\tand\t%s, %s, %s\n", reglist[r2], reglist[r2], reglist[r1]);
  free_register(r1);
  return r2;
}

int cgor(int r1, int r2) {
  fprintf(Outfile, "\torr\t%s, %s, %s\n", reglist[r2], reglist[r2], reglist[r1]);
  free_register(r1);
  return r2;
}

int cgxor(int r1, int r2) {
  fprintf(Outfile, "\teor\t%s, %s, %s\n", reglist[r2], reglist[r2], reglist[r1]);
  free_register(r1);
  return r2;
}

int cgnegate(int r) {
  fprintf(Outfile, "\tneg\t%s, %s\n", reglist[r], reglist[r]);
  return r;
}

int cginvert(int r) {
  fprintf(Outfile, "\tmvn\t%s, %s\n", reglist[r], reglist[r]);
  return r;
}

int cgshl(int r1, int r2) {
  fprintf(Outfile, "\tlsl\t%s, %s, %s\n", reglist[r1], reglist[r1], reglist[r2]);
  free_register(r2);
  return r1;
}

int cgshr(int r1, int r2) {
  fprintf(Outfile, "\tlsr\t%s, %s, %s\n", reglist[r1], reglist[r1], reglist[r2]);
  free_register(r2);
  return r1;
}

int cglognot(int r) {
  fprintf(Outfile, "\tcmp\t%s, #0\n", reglist[r]);
  fprintf(Outfile, "\tmoveq\t%s, #1\n", reglist[r]);
  fprintf(Outfile, "\tmovne\t%s, #0\n", reglist[r]);
  return r;
}

int cgboolean(int r, int op, int label) {
  fprintf(Outfile, "\tcmp\t%s, #0\n", reglist[r]);
  if (op == A_IF || op == A_WHILE) {
    fprintf(Outfile, "\tbeq\tL%d\n", label);
  } else {
    fprintf(Outfile, "\tmovne\t%s, #1\n", reglist[r]);
    fprintf(Outfile, "\tmoveq\t%s, #0\n", reglist[r]);
  }
  return r;
}

void cgprintint(int r) {
  fprintf(Outfile, "\tmov\tr0, %s\n", reglist[r]);
  fprintf(Outfile, "\tbl\tprintint\n");
  free_register(r);
}
void cgcopyarg(int r, int argposn) {
    if (argposn > 4) {
        fprintf(Outfile, "\tstr\t%s, [sp, #-4]!\n", reglist[r]);
    } else {
        fprintf(Outfile, "\tmov\t%s, %s\n", reglist[FIRSTPARAMREG - argposn + 1], reglist[r]);
    }
}
int cgcall(int id, int numargs) {
    int outr = alloc_register();
    fprintf(Outfile, "\tbl\t%s\n", Symtable[id].name);
    if (numargs > 4) {
        fprintf(Outfile, "\tadd\tsp, sp, #%d\n", 4 * (numargs - 4));
    }
    fprintf(Outfile, "\tmov\t%s, r0\n", reglist[outr]);
    return (outr);
}
int cgshlconst(int r, int val) {
  fprintf(Outfile, "\tlsl\t%s, %s, #%d\n", reglist[r], reglist[r], val);
  return r;
}

int cgstorglob(int r, int id) {
  fprintf(Outfile, "\tldr\tr3, =%s\n", Symtable[id].name);
  switch (Symtable[id].type) {
    case P_CHAR:
      fprintf(Outfile, "\tstrb\t%s, [r3]\n", reglist[r]);
      break;
    case P_INT:
    case P_LONG:
    case P_CHARPTR:
    case P_INTPTR:
    case P_LONGPTR:
      fprintf(Outfile, "\tstr\t%s, [r3]\n", reglist[r]);
      break;
    default:
      fatald("Bad type in cgstorglob:", Symtable[id].type);
  }
  return r;
}

int cgstorlocal(int r, int id) {
  switch (Symtable[id].type) {
    case P_CHAR:
      fprintf(Outfile, "\tstrb\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
      break;
    case P_INT:
    case P_LONG:
    case P_CHARPTR:
    case P_INTPTR:
    case P_LONGPTR:
      fprintf(Outfile, "\tstr\t%s, [fp, #%d]\n", reglist[r], Symtable[id].posn);
      break;
    default:
      fatald("Bad type in cgstorlocal:", Symtable[id].type);
  }
  return r;
}

static int psize[] = { 0, 0, 1, 4, 4, 4, 4, 4, 4 };

int cgprimsize(int type) {
  if (type < P_NONE || type > P_LONGPTR)
    fatal("Bad type in cgprimsize()");
  return psize[type];
}

void cgglobsym(int id) {
  int typesize;
  if (Symtable[id].stype == S_FUNCTION)
    return;
  typesize = cgprimsize(Symtable[id].type);
  cgdataseg();
  fprintf(Outfile, "\t.globl\t%s\n", Symtable[id].name);
  fprintf(Outfile, "%s:", Symtable[id].name);
  for (int i = 0; i < Symtable[id].size; i++) {
    switch (typesize) {
      case 1: fprintf(Outfile, "\t.byte\t0\n"); break;
      case 4: fprintf(Outfile, "\t.word\t0\n"); break;
      default: fatald("Unknown typesize in cgglobsym: ", typesize);
    }
  }
}

void cgglobstr(int l, char *strvalue) {
  char *cptr;
  cglabel(l);
  for (cptr = strvalue; *cptr; cptr++) {
    fprintf(Outfile, "\t.byte\t%d\n", *cptr);
  }
  fprintf(Outfile, "\t.byte\t0\n");
}

static char *cmplist[] = { "moveq", "movne", "movlt", "movgt", "movle", "movge" };
static char *invcmplist[] = { "movne", "moveq", "movge", "movle", "movgt", "movlt" };

int cgcompare_and_set(int ASTop, int r1, int r2) {
  if (ASTop < A_EQ || ASTop > A_GE)
    fatal("Bad ASTop in cgcompare_and_set()");

  fprintf(Outfile, "\tcmp\t%s, %s\n", reglist[r1], reglist[r2]);
  fprintf(Outfile, "\t%s\t%s, #1\n", cmplist[ASTop - A_EQ], reglist[r2]);
  fprintf(Outfile, "\t%s\t%s, #0\n", invcmplist[ASTop - A_EQ], reglist[r2]);
  free_register(r1);
  return r2;
}

void cglabel(int l) {
  fprintf(Outfile, "L%d:\n", l);
}

void cgjump(int l) {
  fprintf(Outfile, "\tb\tL%d\n", l);
}

static char *brlist[] = { "bne", "beq", "bge", "ble", "bgt", "blt" };

int cgcompare_and_jump(int ASTop, int r1, int r2, int label) {
  if (ASTop < A_EQ || ASTop > A_GE)
    fatal("Bad ASTop in cgcompare_and_jump()");

  fprintf(Outfile, "\tcmp\t%s, %s\n", reglist[r1], reglist[r2]);
  fprintf(Outfile, "\t%s\tL%d\n", brlist[ASTop - A_EQ], label);
  freeall_registers();
  return NOREG;
}

int cgwiden(int r, int oldtype, int newtype) {
  return r;
}

void cgreturn(int reg, int id) {
  switch (Symtable[id].type) {
    case P_VOID:
      fprintf(Outfile, "\tmov\tr0, #0\n");
      break;
    case P_CHAR:
    case P_INT:
    case P_LONG:
    case P_CHARPTR:
    case P_INTPTR:
    case P_LONGPTR:
      fprintf(Outfile, "\tmov\tr0, %s\n", reglist[reg]);
      break;
    default:
      fatald("Bad function type in cgreturn:", Symtable[id].type);
  }
  cgjump(Symtable[id].endlabel);
}

int cgaddress(int id) {
  int r = alloc_register();
  if (Symtable[id].class == C_LOCAL)
    fprintf(Outfile, "\tsub\t%s, fp, #%d\n", reglist[r], -Symtable[id].posn);
  else
    fprintf(Outfile, "\tldr\t%s, =%s\n", reglist[r], Symtable[id].name);
  return r;
}

int cgderef(int r, int type) {
  switch (type) {
    case P_CHARPTR:
      fprintf(Outfile, "\tldrb\t%s, [%s]\n", reglist[r], reglist[r]);
      break;
    case P_INTPTR:
    case P_LONGPTR:
      fprintf(Outfile, "\tldr\t%s, [%s]\n", reglist[r], reglist[r]);
      break;
    default:
      fatald("Can't cgderef on type:", type);
  }
  return r;
}

int cgstorderef(int r1, int r2, int type) {
  switch (type) {
    case P_CHAR:
      fprintf(Outfile, "\tstrb\t%s, [%s]\n", reglist[r1], reglist[r2]);
      break;
    case P_INT:
    case P_LONG:
      fprintf(Outfile, "\tstr\t%s, [%s]\n", reglist[r1], reglist[r2]);
      break;
    default:
      fatald("Can't cgstorderef on type:", type);
  }
  return r1;
}
