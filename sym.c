#include "defs.h"
#include "data.h"
#include "decl.h"
//GLOB  定义变量的数组索引 内连接的全局变量
int findglob(char *s){//s是整个字符串，*s是字符串首地址,寻找s在不在Symtable里，有的输出索引，没有输出-1 
    int i;
    for(i=0;i<Globs;i++){
        if(*s==*Symtable[i].name&&!strcmp(s,Symtable[i].name))//短路求值 由于strcmp遍历整个字符串，比较费时，先*s比较第一个字符，然后在strcmp会省时 两个条件取交集
        return i;
    }
    return -1;
}
static int newglob(){//已经定义（或者说已经被分配了槽位）的全局符号（变量、函数等）的数量
    int p;
    if((p=Globs++)>=Locls){ //比局部的最后一个还多 说明装不下了 抢了局部的位置
        fatal("Too many global symbols");
    }
    return p;
}
int findlocl(char *s){
    int i;
    for(i=Locls+1;i<NSYMBOLS;i++){
        if(*s==*Symtable[i].name&&!strcmp(s,Symtable[i].name))
        return i;
    }
    return -1;

}
static int newclol(){
    int p;
    if((p=Locls--)<=Globs){
        fatal("Too many local symbols");
    }
    return p;
}
static void updatesym(int slot, char *name, int type, int stype,
		      int class, int endlabel, int size, int posn) {
  if (slot < 0 || slot >= NSYMBOLS)
    fatal("Invalid symbol slot number in updatesym()");
  Symtable[slot].name = strdup(name);
  Symtable[slot].type = type;
  Symtable[slot].stype = stype;
  Symtable[slot].class = class;
  Symtable[slot].endlabel = endlabel;
  Symtable[slot].size = size;
  Symtable[slot].posn = posn;
}
int addglob(char *name,int type,int stype,int endlabel,int size){
    int slot;
    if((slot=findglob(name))!=-1){//如果已经存在了 直接返回
        return slot;
    }
    slot=newglob();
    updatesym(slot,name,type,stype,C_GLOBAL,endlabel,size,0);
    genglobsym(slot);
    //Symtable[i].name 内存很有可能不连续
    //name 是局部遍历 所以不能直接相等
    return slot;
}
// 1. 普通变量：size = 1（1个元素）
// 2. 数组：size = 元素个数
// 3. 函数：size = 0（不需要数据空间）
int addlocl(char *name, int type, int stype, int endlabel, int size) {
  int slot, posn;
  if ((slot = findlocl(name)) != -1)
    return (slot);
  slot = newclol();
  posn = gengetlocaloffset(type, 0);	
  updatesym(slot, name, type, stype, C_LOCAL, endlabel, size, posn);
  return (slot);
}
int findsymbol(char *s){//局部在前 全局在后
    int slot;
    slot=findlocl(s);
    if(slot==-1){
        slot=findglob(s);
    }
    return slot;
}

