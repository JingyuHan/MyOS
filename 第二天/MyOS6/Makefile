
# 默认动作

default :
	../../tolset/z_tools/make.exe img

# 文件生成规则

ipl.bin : ipl.nas Makefile
	../../tolset/z_tools/nask.exe ipl.nas ipl.bin ipl.lst

MyOS.img : ipl.bin Makefile
	../../tolset/z_tools/edimg.exe   imgin:../../tolset/z_tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0   imgout:MyOS.img

# 添加命令

asm :
	../../tolset/z_tools/make.exe -r ipl.bin

img :
	../../tolset/z_tools/make.exe -r MyOS.img

clean :
	-del ipl.bin
	-del ipl.lst

src_only :
	../../tolset/z_tools/make.exe clean
	-del MyOS.img
