TOOLPATH = ../../tolset/z_tools/
MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

# 默认动作

default :
	$(MAKE) img

# 文件生成规则

ipl.bin : ipl.nas Makefile
	$(NASK) ipl.nas ipl.bin ipl.lst

MyOS.sys : MyOS.nas Makefile
	$(NASK) MyOS.nas MyOS.sys MyOS.lst

MyOS.img : ipl.bin MyOS.sys Makefile
	$(EDIMG)   imgin:../../tolset/z_tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 \
		copy from:MyOS.sys to:@: \
		imgout:MyOS.img

# 添加命令

img :
	$(MAKE) MyOS.img


clean :
	-$(DEL) ipl.bin
	-$(DEL) ipl.lst
	-$(DEL) MyOS.sys
	-$(DEL) MyOS.lst

src_only :
	$(MAKE) clean
	-$(DEL) MyOS.img