#include "defs.h"
#include "data.h"
#include "decl.h"
//构建表达式的ast
 // function_call: identifier '(' expression ')' ;
struct ASTnode *funccall(){
    struct ASTnode *tree;
    int id;
     if ((id = findsymbol(Text)) == -1) {
    fatals("Undeclared function", Text);
  }
  lparen();
  tree=binexpr(0);//解析expression
  tree=mkastunary(A_FUNCCALL,Symtable[id].type,tree,id);//根节点为A_FUNCCAL 并且记录函数名变量的id
  rparen();
  return tree;
} 
static int binastop(int tokentype){//在defs里T_add的枚举值是1 A_ADD的值也是1 这样隐形映射了
    if(tokentype>T_E0F && tokentype<T_INTLIT){//证明是运算符号不是逻辑符号
        return tokentype;
    }
        fatald("Syntax error, token", tokentype);
        return 0;
}
struct ASTnode *array_access(){
    struct ASTnode *left,*right;
    int id;
    if((id=findsymbol(Text))==-1||Symtable[id].stype!=S_ARRAY){
        fatals("Undeclared array", Text);
    }
    left=mkastleaf(A_ADDR,Symtable[id].type,id);//左子树 变量名 
    scan(&Token);// '['
    right=binexpr(0);//解析索引
    match(T_RBRACKET,"]");
    if (!inttype(right->type))
    fatal("Array index is not of integer type");
    right = modify_type(right, left->type, A_ADD);//比例缩放 把数组变成 *(p+1) 把i变成偏移量
    left=mkastnode(A_ADD,Symtable[id].type,left,NULL,right,0);//将数组的首地址与计算出的偏移量相加，从而得到目标元素的内存地址。
    left = mkastunary(A_DEREF, value_at(left->type), left, 0);//解引用
    return left;
}
static struct ASTnode *postfix(){
    struct ASTnode *n;
    int id;
    scan(&Token);
        if(Token.token==T_LPAREN){//如果前面是括号 就是函数调用
            return funccall();
        }
        if(Token.token==T_LBRACKET){//如果是[ 就是数组
            return array_access();
        }
        id=findsymbol(Text);
        if(id==-1||Symtable[id].stype != S_VARIABLE){
            fatals("Unknown variable", Text);
        }
        switch(Token.token){
            case T_INC:
            scan(&Token);//消耗++ -- 
            n = mkastleaf(A_POSTINC, Symtable[id].type, id);
            break;
            case T_DEC:
            scan(&Token);
            n = mkastleaf(A_POSTDEC, Symtable[id].type, id);
            break;
            default:
            n=mkastleaf(A_IDENT,Symtable[id].type,id);
        }
        return n;
}

//识别整数文字，构建叶子节点
static struct ASTnode *primary(){
    struct ASTnode *n;
    int id;
    switch(Token.token){
        case T_INTLIT:
        if((Token.intvalue)>=0 && (Token.intvalue<256)){//定义如果整数常量的值在 0 到 255 之间，它被认为是 P_CHAR 类型
            n=mkastleaf(A_INTLIT,P_CHAR,Token.intvalue);//token是数字，则肯定是叶子节点.
        }else{
            n = mkastleaf(A_INTLIT, P_INT, Token.intvalue);
        }
        break;
        case T_STRLIT:
        id=genglobstr(Text);//处理内存和汇编指令
        n= mkastleaf(A_STRLIT, P_CHARPTR, id);//生成叶子节点
        break;
        case T_IDENT:
        return postfix();
        case T_LPAREN:
        scan(&Token);
        n=binexpr(0);
        rparen();
        return n;
        default:
        fatald("Expecting a primary expression, got token", Token.token);
    }
    scan(&Token);
        return n;
}
static int rightassoc(int tokentype){//如果是等号 则是右结合 赋值为1
    if (tokentype==T_ASSIGN){
        return 1;
    }
    return 0;
}
static int OpPrec[] = {
  0, 10, 20, 30,                // T_EOF, T_ASSIGN, T_LOGOR, T_LOGAND
  40, 50, 60,                   // T_OR, T_XOR, T_AMPER 
  70, 70,                       // T_EQ, T_NE
  80, 80, 80, 80,               // T_LT, T_GT, T_LE, T_GE
  90, 90,                       // T_LSHIFT, T_RSHIFT
  100, 100,                     // T_PLUS, T_MINUS
  110, 110                      // T_STAR, T_SLASH
};
//检查我们是否一个二元运算符并返回它的优先级
//数组只会包含那些具有运算符语义的令牌的优先级，而其他非运算符令牌则不会包含在内
static int op_precedence(int tokentype){
    int prec;
    if(tokentype>=T_VOID){
        fatald("Token with no precedence in op_precedence:", tokentype);
    }
    prec=OpPrec[tokentype];//tokentype 此时枚举变成数 刚好是数组下标
    if(prec==0){
        fatald("Syntax error, token", tokentype);
    }
    return prec;
}
// prefix_expression: primary
//     | '*' prefix_expression
//     | '&' prefix_expression
//     ;
struct ASTnode *prefix(){
    struct ASTnode *tree;
    switch (Token.token)
    {
    case T_AMPER://取地址符号 x=&y
        scan(&Token);
        tree=prefix();//接着处理 **p
        if(tree-> op !=A_IDENT) //递归返回 primary的节点 之后 查看该树的op是不是变量
        fatal("& operator must be followed by an identifier");
        tree->op=A_ADDR;//原地修改
        tree->type=pointer_to(tree->type);
        break;
    case T_STAR: //处理 *p = 5; 
    scan(&Token);
    tree=prefix();//递归 
    if(tree->op!=A_IDENT && tree->op!=A_DEREF)//检查
        fatal("* operator must be followed by an identifier or *");
        tree = mkastunary(A_DEREF, value_at(tree->type), tree, 0);
        break;
    case T_MINUS:
    scan(&Token);
    tree=prefix();
    tree->rvalue = 1;
    tree = modify_type(tree, P_INT, 0);
    tree = mkastunary(A_NEGATE, tree->type, tree, 0);
    break;
    case T_INVERT:
    scan(&Token); tree = prefix();
    tree->rvalue = 1; // 剥夺左值特权
    tree = mkastunary(A_INVERT, tree->type, tree, 0);
    break;
    case T_LOGNOT:
    scan(&Token); tree = prefix();
    tree->rvalue = 1;
    tree = mkastunary(A_LOGNOT, tree->type, tree, 0);
    break;
    case T_INC://++ -- 会写在变量上 替代 所以还是左值
    scan(&Token);
    tree=prefix();
    if(tree->op!=A_IDENT){
        fatal("++ operator must be followed by an identifier");
    }
    tree = mkastunary(A_PREINC, tree->type, tree, 0);
    break;
    case T_DEC:
    scan(&Token);
    tree=prefix();
    if(tree->op!=A_IDENT){
        fatal("-- operator must be followed by an identifier");
    }
    tree = mkastunary(A_PREDEC, tree->type, tree, 0);
    break;
    default:
        tree=primary();
    }
    return tree; 
}
struct  ASTnode *binexpr(int ptp){ //构建AST ptp是符号优先级，一开始是0
    struct ASTnode *left,*right;
    struct ASTnode *ltemp,*rtemp;
    int ASTop;
    int tokentype;
    left=prefix();//解析取地址符 指针 primary  优先级高的会在下面 左右子树 并没有优先级区分
    tokentype=Token.token; 
    if(tokentype==T_SEMI||tokentype==T_RPAREN||tokentype == T_RBRACKET){
        left->rvalue=1;//在表达式解析完成的瞬间，将“代表位置”的节点（l-value）自动转化为“代表数值”的节点（r-value），确保代码生成阶段能正确地从内存中加载数据
        return left;
    }
    while(op_precedence(tokentype)>ptp||(rightassoc(tokentype) && op_precedence(tokentype)==ptp)){ //左结合当这个符号的优先级大于上一个符号的优先级时 或者 右结合 连等
        scan(&Token);//先消耗这个token
        right=binexpr(OpPrec[tokentype]);//使用我们标记的优先级 递归调用binexpr来构建子树 最终 递归到right为数 时候停止 遇到 ;就停止
        ASTop=binastop(tokentype);//将T node 转换成 A node
        if (ASTop==A_ASSIGN){
            right->rvalue=1;//如果遇到= 强制右结合
            right=modify_type(right,left->type,ASTop);//检查 右边类型 是不是符合左边变量
            if (right==NULL)//如果类型不匹配
            fatal("Incompatible expression in assignment");
            ltemp=left;
            left=right;
            right=ltemp;//将等号右边的算术表达式的部分 作为 左子树
        }else{
            left->rvalue=1;
            right->rvalue=1;//此时 左右子树都是右值
            ltemp=modify_type(left,right->type,ASTop);//左边整数 右边指针 缩放
            rtemp=modify_type(right,left->type,ASTop);//右边char(源数据) 左边long 拓宽吗
            if (ltemp == NULL && rtemp == NULL)
            fatal("Incompatible types in binary expression");
            if (ltemp != NULL)
            left = ltemp;//更新
            if (rtemp != NULL)
            right = rtemp;//更新
        }
        left=mkastnode(binastop(tokentype),left->type,left,NULL,right,0);//连接子树 拓宽和缩放只更新左子树
        tokentype=Token.token;//更新token 如果token为e0f直接返回left
        if(tokentype == T_SEMI||tokentype==T_RPAREN||tokentype==T_RBRACKET){
            left->rvalue=1;//结束时 整棵树为右值
            return left;
        }
    }
    left->rvalue=1;
    return left; //优先级更低时返回我们的子树
}
/*外层调用：binexpr(ptp = 0) (处理 a + b * c)

left = primary();

primary() 解析 a。
返回 a 的AST节点。
当前栈帧的 left 现在是 a 的AST节点。
Token 变为 T_PLUS。
tokentype = Token.token; (tokentype 是 T_PLUS)
if (tokentype == T_EOF) -> false。

while (op_precedence(T_PLUS) > ptp) (假设 OpPrec[T_PLUS] 是 10，所以 10 > 0 为 true，进入循环)

a.  scan(&Token);

Token 变为 T_STAR。
b.  right = binexpr(OpPrec[tokentype]);

这里的 tokentype 是 T_PLUS，所以调用 binexpr(OpPrec[T_PLUS])，即 binexpr(10)。
进入 binexpr(ptp = 10) 的栈帧 (处理 b * c)：
left = primary();
primary() 解析 b。
返回 b 的AST节点。
当前栈帧的 left 现在是 b 的AST节点。
Token 变为 T_STAR。
tokentype = Token.token; (tokentype 是 T_STAR) if (tokentype == T_EOF) -> false。
while (op_precedence(T_STAR) > ptp) (假设 OpPrec[T_STAR] 是 20，20 > 10 为 true，进入循环) a. scan(&Token);
Token 变为 c 的令牌 (例如 T_INTLIT)。 b. right = binexpr(OpPrec[tokentype]);
这里的 tokentype 是 T_STAR，所以调用 binexpr(OpPrec[T_STAR])，即 binexpr(20)。
进入 binexpr(ptp = 20) 的栈帧 (处理 c)：
left = primary();
primary() 解析 c。
返回 c 的AST节点。
当前栈帧的 left 现在是 c 的AST节点。
Token 变为 T_EOF (假设没有更多内容)。
tokentype = Token.token; (tokentype 是 T_EOF) if (tokentype == T_EOF) -> true。
return (left); -> 返回 c 的AST节点。
binexpr(20) 返回 c 的AST节点。 c. 回到 binexpr(10) 的栈帧：
right (当前栈帧的 right) 得到 c 的AST节点。 d. left = mkastnode(arithop(tokentype), left, right, 0);
这里的 tokentype 是 T_STAR。
左侧 left (当前栈帧的 left) 是 b 的AST节点。
右侧 right (当前栈帧的 right) 是 c 的AST节点。
创建 b * c 的AST节点。
将 b * c 的AST节点赋值给当前栈帧的 left。 e. tokentype = Token.token; (Token 是 T_EOF，所以 tokentype 是 T_EOF) f. if (tokentype == T_EOF) -> true。
return (left); -> 返回 b * c 的AST节点。
binexpr(10) 返回 b * c 的AST节点。
c.  回到最外层 binexpr(0) 的栈帧：

right (当前栈帧的 right) 得到 b * c 的AST节点。
所以，你这里说的 right 是 b * c，是完全正确的！
d.  left = mkastnode(arithop(tokentype), left, right, 0);

这里的 tokentype 是 T_PLUS (循环开始时保存的)。
左侧 left (当前栈帧的 left) 是 a 的AST节点。
右侧 right (当前栈帧的 right) 是 b * c 的AST节点。
创建 a + (b * c) 的AST节点。
将 a + (b * c) 的AST节点赋值给当前栈帧的 left。
e.  tokentype = Token.token; (Token 是 T_EOF，所以 tokentype 是 T_EOF)
f.  if (tokentype == T_EOF) -> true。

return (left); -> 返回 a + (b * c) 的AST节点。 
struct  ASTnode *binexpr(int ptp){ //构建AST ptp是符号优先级，一开始是0
    struct ASTnode *n ,*left,*right;
    int tokentype;
    left=primary();//primary只能处理叶子节点的情况
    tokentype=Token.token; 
    if(tokentype==T_SEMI||tokentype==T_RPAREN){
        return left;
    }
    while(op_precedence(tokentype)>ptp){ //当这个符号的优先级大于上一个符号的优先级时
        scan(&Token);//先消耗这个token
        right=binexpr(OpPrec[tokentype]);//使用我们标记的优先级 递归调用binexpr来构建子树
        left=mkastnode(arithop(tokentype),left,NULL,right,0);//连接子树
        tokentype=Token.token;//更新token 如果token为e0f直接返回left
        if(tokentype == T_SEMI||tokentype==T_RPAREN){
            return left;
        }
    }
    return left; //优先级更低时返回我们的子树
}*/
// 解析 *ptr+2
// 进入 binexpr。
// 执行 left = prefix();。
// prefix 看到 *，递归调用自己，然后调用 primary 读取 ptr。
// prefix 返回一个 AST树：DEREF( IDENT(ptr) )。
// left 变量现在持有了这个树。
// binexpr 读取 +。
// binexpr 继续解析右边的 2。
// 最终构建出完整的树：
// + (ADD)
//      /     
// (left)      (right)
//   |            |
// * (DEREF)      2
//   |
//  ptr
