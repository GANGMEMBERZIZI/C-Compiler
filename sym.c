#include "defs.h"
#include "data.h"
#include "decl.h"
//GLOB  定义变量的数组索引 内连接的全局变量
int findglob(char *s){//s是整个字符串，*s是字符串首地址,寻找s在不在gsym里，有的输出索引，没有输出-1 
    int i;
    for(i=0;i<Globs;i++){
        if(*s==*Gsym[i].name&&!strcmp(s,Gsym[i].name))//短路求值 由于strcmp遍历整个字符串，比较费时，先*s比较第一个字符，然后在strcmp会省时 两个条件取交集
        return i;
    }
    return -1;
}
static int newglob(){//已经定义（或者说已经被分配了槽位）的全局符号（变量、函数等）的数量
    int p;
    if((p=Globs++)>=NSYMBOLS){ //因为是索引 所以先等于后+1
        fatal("Too many global symbols");
    }
    return p;
}
int addglob(char *name,int type,int stype,int endlabel,int size){
    int y;
    if((y=findglob(name))!=-1){//如果已经存在了 直接返回
        return y;
    }
    y=newglob();
    Gsym[y].name=strdup(name);//strdup 函数根据 name 指针所指向的字符串内容，在堆上开辟了一块新的内存，将字符串内容复制到新内存中，并将这个新内存的地址赋值给了 gsym[y].name
    //gsym[i].name 内存很有可能不连续
    //name 是局部遍历 所以不能直接相等
    Gsym[y].type=type;//类型
    Gsym[y].stype=stype;//是函数还是变量
    Gsym[y].endlabel = endlabel;
    Gsym[y].size=size;

    return y;
}
// 1. 普通变量：size = 1（1个元素）
// 2. 数组：size = 元素个数
// 3. 函数：size = 0（不需要数据空间）

