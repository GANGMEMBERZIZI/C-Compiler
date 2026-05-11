#ifndef extern_
#define extern_ extern
#endif
extern_ int Line;
extern_ int Putback;
extern_ int Functionid;
extern_ int Globs;
extern_ FILE	*Infile;  //FILE 是 C 语言标准库 <stdio.h> 中定义的一个结构体类型（struct type）
extern_ FILE	*Outfile;
extern_ struct  token Token;
#define TEXTLEN		512
extern_ char Text[TEXTLEN+1];
#define NSYMBOLS    1024
extern_ struct symtable Gsym[NSYMBOLS];
#define NOREG	-1
#define NOLABEL 0
extern_ int O_dumpAST;//全局调试标志 
//不加 extern 在函数体外部是“定义”，并自带“外部链接”，是“内存的实际拥有者”。在一个程序中只能有一次。
//加 extern 是“声明”，它“引用”其他文件中的定义，是“内存的共享访问者”。可以有多次。
//extern_ 宏的目的是“巧妙地将同一个头文件，在不同源文件中扮演不同的角色”：在定义文件里，它让变量变成实际定义；在其他使用文件里，它让变量变成外部声明。


