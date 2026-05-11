#include "defs.h"
#include "data.h"
#include "decl.h"
int inttype(int type){
  if(type==P_CHAR||type==P_INT||type==P_LONG){
    return 1;
  }
  return 0;
}
int ptrtype(int type){
  if(type==P_CHARPTR||type==P_INTPTR||type==P_LONGPTR)
  return 1;
  return 0;
}
int pointer_to(int type){//给一个原始类型 返回指针类型
  int newtype;
  switch(type){
    case P_VOID: newtype=P_VOIDPTR; break;
    case P_CHAR: newtype=P_CHARPTR; break;
    case P_INT: newtype=P_INTPTR; break;
    case P_LONG: newtype=P_LONGPTR; break;
    default:
  fatald("Unrecognised in pointer_to: type", type);
  }
  return newtype;
}
int value_at(int type){
  int newtype;
  switch(type){
    case P_VOIDPTR: newtype = P_VOID; break;
    case P_CHARPTR: newtype = P_CHAR; break;
    case P_INTPTR:  newtype = P_INT;  break;
    case P_LONGPTR: newtype = P_LONG; break;
    default:
      fatald("Unrecognised in value_at: type", type);
  }
  return newtype;
}
struct ASTnode *modify_type(struct ASTnode *tree,int rtype,int op){//检查原树的type(ltype)是否匹配rtype
  int ltype;
  int lsize,rsize;
  ltype=tree->type;
  if(inttype(ltype) && inttype(rtype)){//拓宽或截断
    if(ltype==rtype)//如果类型一样 返回原树
    return tree;
    lsize=genprimsize(ltype);
    rsize=genprimsize(rtype);
    // 如果目标类型更大，需要拓宽；如果目标类型更小，需要截断，两种情况都允许
    if (rsize>lsize)
    return mkastunary(A_WIDEN,rtype,tree,0);
    if (rsize<lsize)
    return NULL; // 报错
  }
  if (ptrtype(ltype)){//如果是指针
    if(op==0 && ltype==rtype)//在原树 op就是符号 是中间节点
    return tree;
    if(op==A_ASSIGN && ltype==rtype)//赋值操作：相同指针类型可以赋值
    return tree;
  }
  if(op==A_ADD||op==A_SUBTRACT){
    if (inttype(ltype) &&ptrtype(rtype))//该函数只能改变ltype 只缩放整数，绝不缩放指针
    {
      rsize=genprimsize(value_at(rtype));//获取指针的源数据类型大小
      if(rsize>1)//char* 和long也可以匹配 缩放 long 如果是char* 就不乘法了
      return mkastunary(A_SCALE, rtype, tree, rsize);
      else//如果为char * 不需要scale 直接返回
      return tree;
    }
  }
    return NULL;//类型不兼容
}