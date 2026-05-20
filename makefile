# x86 版本的源文件列表 (用 cg.c)
SRCS= cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c

# ARM 版本的源文件列表 (用 cg_arm.c)
ARMSRCS= cg_arm.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c

# 目标1: 编译 x86 版编译器
comp1: $(SRCS)
	cc -o comp1 -g $(SRCS)

# 目标2: 编译 ARM 版编译器
comp1arm: $(ARMSRCS)
	cc -o comp1arm -g $(ARMSRCS)

# 清理
clean:
	rm -f comp1 comp1arm *.o *.s out

# 测试 x86 (本机架构)
test: comp1 input.txt
	./comp1 input.txt             
	cc -o out out.s lib/printint.c 
	./out                         

# 测试 ARM (交叉编译 + 模拟器)
armtest: comp1arm input.txt
	./comp1arm input.txt           
	arm-linux-gnueabi-gcc -o out out.s lib/printint.c -static
	qemu-arm ./out
	
test27: comp1 input27a.c input27b.c
	./comp1 input27a.c
	cc -o out input27b.c out.s lib/printint.c 
	./out

test272: comp1arm input27a.c input27b.c
	./comp1 input27a.c
	cc -o out input27b.c out.s lib/printint.c 
	./out
