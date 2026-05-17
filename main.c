#include "defs.h"
#define extern_
#include "data.h"
#undef extern_
#include "decl.h"
#include <errno.h>
static void init(){
    Line=1;
    Putback='\n';
    Globs=0;
    Locls = NSYMBOLS - 1;
    O_dumpAST=0;
}
static void usage(char *prog) {
  fprintf(stderr, "Usage: %s infile\n", prog);
  exit(1);
}
//当程序检测到命令行参数不正确（例如，用户没有提供必需的输入文件），
//它将一条包含正确程序使用方法的错误消息打印到标准错误流 stderr 上，
//然后立即终止程序，并返回一个非零的退出状态码，告知操作系统程序执行失败
char *tokstr[]={"+","-","*","/","intlit"};//内存是双引号
static void scanfile(){
    struct token T ;
    while(scan(&T)){
        printf("Token %s",tokstr[T.token]);//枚举的数
        if (T.token==T_INTLIT){
            printf("value %d",T.intvalue);
        }
        printf("\n");
    }
}
//主程序：检查参数并打印用法
// 如果没有参数，则打开输入
// 文件并调用 scanfile() 来扫描其中的标记
int main(int argc, char *argv[]) {
  int i;

  init();//初始化

  for (i=1; i<argc; i++) {
    if (*argv[i] != '-') break;
    for (int j=1; argv[i][j]; j++) {
      switch (argv[i][j]) {
	case 'T': O_dumpAST =1; break;
	default: usage(argv[0]);
      }
    }
  }

  if (i >= argc)
    usage(argv[0]);

  if ((Infile = fopen(argv[i], "r")) == NULL) {
    fprintf(stderr, "Unable to open %s: %s\n", argv[i], strerror(errno));
    exit(1);
  }
  if ((Outfile = fopen("out.s", "w")) == NULL) {
    fprintf(stderr, "Unable to create out.s: %s\n", strerror(errno));
    exit(1);
  }
  addglob("printint", P_CHAR, S_FUNCTION,0,0);
  addglob("printchar", P_VOID, S_FUNCTION,0,0);

  scan(&Token);		
  genpreamble();		
  global_declarations();	
  genpostamble();		
  fclose(Outfile);		
  return (0);
}
