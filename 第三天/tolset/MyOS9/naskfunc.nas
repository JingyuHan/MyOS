; naskfunc

[FORMAT "WCOFF"]				; 制作目标文件的模式	
[BITS 32]						; 制作32位模式用的机械语言


; 制作目标文件的信息

[FILE "naskfunc.nas"]			; 源文件名的信息

		GLOBAL	_io_hlt			; 程序汇总包含的函数名


; 实际函数

[SECTION .text]					; 目标文件中写了这个后面接程序

_io_hlt:						; void io_hlt(void);
		HLT
		RET
