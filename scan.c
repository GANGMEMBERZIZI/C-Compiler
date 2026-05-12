#include "defs.h"
#include "data.h"
#include "decl.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//static作为一个模块的私有数据，防止其他文件直接修改 避免命名冲突： 允许在不同源文件中定义同名的 static 全局变量而不会冲突。
static int scanint(int c);
static int chrpos(char *s,int c);
static int scanident(int c,char *buf,int lim);
static int keyword(char *s);
static int scanstr(char *buf);
static int next(){
    int c;
    if (Putback){
        c=Putback;
        Putback=0;
        return c;      
    }
    c=fgetc(Infile);
    if ('\n'==c){
        Line++;
    }
    return c;
    }
    static void putback(int c){
        Putback=c; 
    }
    static int skip(){
        int c;
        c=next();
        while (' '==c||'\t'==c||'\n'==c||'\r'==c||'\f'==c) // \t制表符 \r回车 \f 换页
        {
            c=next();
        
        }
        return (c);   
    }
    static int scanch(){
        int c;
        c=next();//获取下一个token
        if(c=='\\'){//如果是'\'
            switch(c=next()){//匹配转义符
                case 'a':
                return '\a';
                case 'b':
                return '\b';
                case 'f':
                return '\f';
                case 'n':
                return '\n';
                case 'r':
                return '\r';
                case 't':
                return '\t';
                case 'v':
                return '\v';
                case '\\':
                return '\\';
                case '"':
                return '"';
                case '\'':
                return '\'';
                default:
                fatalc("unknown escape sequence", c);
            }
        }
        return c;//普通的字符
    }
    static struct token *Rejtoken=NULL;//生成一个拒绝token 存储用来分析是函数还是变量 消耗的token'('
    void reject_token(struct token *t){
        if(Rejtoken!=NULL){//检查是否有未被处理的token
            fatal("Can't reject token twice");
        }
        Rejtoken=t;//将t的地址给rejtoken 调用函数构建树的时候返还(
    }
    int scan(struct token *t){
        int c,tokentype;
        if(Rejtoken!=NULL){//扫描需要返回的字符时 会将该字符返还给t 然后再匹配
            t=Rejtoken;
            Rejtoken=NULL;//清空
            return 1;
        }
        c=skip(); //跳过空白
        switch (c) 
        {
        case EOF:
        t->token = T_E0F;
        return 0;//说明分析到头了 分析完了
        case '+':
        if((c=next())=='+'){
            t->token=T_INC;
        }else{
            putback(c);
            t->token=T_PLUS;
        }
        break;
        case '-':
        if((c=next()=='-')){
            t->token=T_DEC;
        }else{
            putback(c);
            t->token=T_MINUS;
        }
        break;
        case '*':
        t->token=T_STAR;
        break;
        case '/':
        t->token=T_SLASH;
        break;
        case ';': //添加终止符
        t->token=T_SEMI;
        break;
        case '{':
        t->token=T_LBRACE;
        break;
        case '}':
        t->token=T_RBRACE;
        break;
        case '(':
        t->token=T_LPAREN;
        break;
        case ')':
        t->token=T_RPAREN;
        break;
        case '[':
        t->token=T_LBRACKET;
        break;
        case ']':
        t->token=T_RBRACKET;
        break;
        case '~':
        t->token = T_INVERT;
        break;
        case '^':
        t->token = T_XOR;
        break;
        case '=':
        if ((c=next())=='='){//匹配==
            t->token=T_EQ;
        }else{
            putback(c);
            t->token=T_ASSIGN;//如果不是== 是= 则匹配T_ASSIGN 
        }
        break;
        case '!':
        if ((c = next()) == '=') {//不等于
             t->token = T_NE;
        } else {
             putback(c);
             t->token = T_LOGNOT;
        }
         break;
    case '<':
    if ((c = next()) == '=') {//<=
      t->token = T_LE;
    }
    if((c=next())=='<'){
        t->token=T_LSHIFT;
    } 
    else {
      putback(c);
      t->token = T_LT;//<
    }
    break;
  case '>':
    if ((c = next()) == '=') {//>=
      t->token = T_GE;
    }
    if((c=next())=='>'){
        t->token=T_RSHIFT;
    } 
    else {
      putback(c);
      t->token = T_GT;//>
    }
    break;
    case '&':
    if((c=next())=='&'){//如果下一个token也是& 则是 &&
        t->token=T_LOGAND;
    }else{
        putback(c);
        t->token=T_AMPER;//取地址符
    }
    break;
    case '|':
    if ((c = next()) == '|') {
	t->token = T_LOGOR;
      } else {
	putback(c);
	t->token = T_OR;
      }
      break;
    case '\''://   \转义特殊字符 \'表示 单引号本身
    t->intvalue=scanch();//扫描字符值
    t->token=T_INTLIT;//设置token类型为整数字面量
    if(next()!='\''){
        fatal("Expected '\\'' at end of char literal");
    }
    break;
    case '"':
    scanstr(Text);//扫描字符串
    t->token=T_STRLIT;
    break;
        default:
        if (isdigit(c)){ //isdigit 是ctype库，如何是十进制数 返回1 否则返回0. 
            t->intvalue=scanint(c); //扫描整数值
            t->token=T_INTLIT;
            break;
        }else if(isalpha(c)||'_'==c){
            scanident(c,Text,TEXTLEN); //读取关键字
            if (tokentype=keyword(Text)){ //识别text是否是关键字 是的话 if里是非零的，如果不是则为0
                t->token=tokentype; //赋值
                break;
            }
            t->token=T_IDENT;//不是keyword就是标识符
            break;
        }
        fatalc("Unrecognised character", c);//如果不是  报错 
        }
        return 1;
    }
     static int scanint(int c){ //从输入的文件扫描返回一个整数值
        int k,val=0;
        while ((k=chrpos("0123456789",c))>=0){ //查找数字字符
            val=val*10+k;
            c=next();//读取下一个字符
        }
        putback(c); //如果遇到非整数字符 放回
        return val;
    }
    //buf 缓冲区 暂时存储字符串
    static int scanstr(char *buf){
        int i,c;
        for(i=0;i<TEXTLEN-1;i++){//由于scan函数 已经消耗掉了第一个" 
            if((c=scanch())=='"'){//遇到最后一个" 表示结束
                buf[i]=0;
                return i;
            }
            buf[i]=c;
        }
        fatal("String literal too long");//太长了 超过TEXTLEN
        return(0);
    }
    static int chrpos(char *s,int c){
        char *p;
        p=strchr(s,c);//查找字符位置
        return(p ? p-s : -1); //三元运算 （条件?表达式1:表达式2）条件为1 return 表达式1 条件为0 return 表达式2
        //地址可以加减 char是一个字节 c=5 起始地址为0，相减返回5
    }
    static int scanident(int c,char *buf,int lim){  //它将字母数字字符读取为 buffer，直到它命中非字母数字字符。 词法分析器
        //c 传入的字符，buf用于存储扫描到的标识符 lim buf最大容量(因为是数组的最大容量包括了null符)
        int i =0;
        while(isalpha(c)||isdigit(c)||'_'==c){ //isalpha判断传入参数是否是字母是传递非0 否则是0 isdigit判断数字 isalpha传入字母会转换成ASCII 码
            if(lim-1==i){
               fatal("Identifier too long");
            }else if(i<lim-1){
                buf[i++]=c;
            }
            c=next();//继续读取下一个字符           
        }
        putback(c);//如果即不是字母 也不是数字 也不是下划线 返回回去
        buf[i]='\0';//终止符 后面不继续读取
        return i; //返回长度
    }
    static int keyword(char *s){ //识别语言中的关键字，这样更加快 如果传入的是字符串的话 默认是第一个字符的地址
        switch (*s) //switch第一个字符
        {
        case 'c':
        if(!strcmp(s,"char")){
            return T_CHAR;
        }    
        case 'e':
        if(!strcmp(s,"else")){
            return T_ELSE;
        }
        break;
        case 'f':
        if(!strcmp(s,"for")){
            return T_FOR;
        }
        case 'i':
        if (!strcmp(s,"if")){
            return T_IF;
        }
        if (!strcmp(s, "int")){
            return (T_INT);
        }
          break;
          case 'l':
       if (!strcmp(s, "long"))
	    return (T_LONG);
         break;
            case 'r':
      if (!strcmp(s, "return"))
	return (T_RETURN);
      break;
        case 'w':
        if(!strcmp(s,"while")){
            return T_WHILE;
        }
        break;
        case 'v':
        if(!strcmp(s,"void")){
            return T_VOID;
        }
        break;
        }
        return 0;
    }
    

