;MyOS

[INSTRSET "i486p"]

VBEMODE	EQU		0x105			; 1024 x  768 x 8bit彩色
; （画面模式号码）
;	0x100 :  640 x  400 x 8bit彩色
;	0x101 :  640 x  480 x 8bit彩色
;	0x103 :  800 x  600 x 8bit彩色
;	0x105 : 1024 x  768 x 8bit彩色
;	0x107 : 1280 x 1024 x 8bit彩色


BOTPAK	EQU		0x00280000		
DSKCAC	EQU		0x00100000		
DSKCAC0	EQU		0x00008000

;有关BOOT_INFO
CYLS	EQU		0x0ff0			;设定启动区
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			;关于颜色数目的信息，颜色的位置
SCRNX	EQU		0x0ff4			;屏幕分辨率的X(screen x)
SCRNY	EQU		0x0ff6			;屏幕分辨率的Y(screen y)
VRAM	EQU		0x0ff8			;图像缓冲区的开始位置

		ORG		0xc200			; 程序被加载到内存的地址
		
; 确认VBE是否存在

		MOV		AX,0x9000
		MOV		ES,AX
		MOV		DI,0
		MOV		AX,0x4f00
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; 检查VBE版本

		MOV		AX,[ES:DI+4]
		CMP		AX,0x0200
		JB		scrn320			; if (AX < 0x0200) goto scrn320

; 取得画面模式信息

		MOV		CX,VBEMODE
		MOV		AX,0x4f01
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; 确认画面模式信息

		CMP		BYTE [ES:DI+0x19],8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b],4
		JNE		scrn320
		MOV		AX,[ES:DI+0x00]
		AND		AX,0x0080
		JZ		scrn320			; 模式属性的bit7是0，则放弃

; 画面模式切换

		MOV		BX,VBEMODE+0x4000
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8	; 记下画面模式（参考C语言）
		MOV		AX,[ES:DI+0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14]
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28]
		MOV		[VRAM],EAX
		JMP		keystatus

scrn320:
		MOV		AL,0x13			; VGA显卡、320x200x8位彩色
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE[VMODE],8	;记录画面模式
		MOV		WORD[SCRNX],320
		MOV		WORD[SCRNY],200
		MOV		DWORD[VRAM],0x000a0000
		
;用BIOS取得键盘上各种LED指示灯的状态
keystatus:
		MOV 	AH,0x02
		INT		0x16			;键盘BIOS
		MOV		[LEDS],AL
		
		
;PIC关闭一切中断
;	根据AT兼容机的规格，初始化PIC必须在CLT之前进行，否则有时会挂起
;	下面进行PIC的初始化		
		
		
		
		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; 如果连续执行OUT指令，有些机种会无法正常运行
		OUT		0xa1,AL

		CLI						; 禁止CPU级别的中断

; 为让CPU可以访问1MB以上的内存空间，设定A20GATE

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; 使能 A20
		OUT		0x60,AL
		CALL	waitkbdout

; 切换到保护模式

[INSTRSET "i486p"]				; 使用486指令

		LGDT	[GDTR0]			; 设定临时GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; 设bit31为0（禁止颁）
		OR		EAX,0x00000001	; 设bit0为1（切换到保护模式）
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			;  可读写的段32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpackの転送

		MOV		ESI,bootpack	; 转送源
		MOV		EDI,BOTPAK		; 转送目的地
		MOV		ECX,512*1024/4
		CALL	memcpy

; 磁盘数据最终转送到它本来的位置去

; 首先从启动扇区开始

		MOV		ESI,0x7c00		; 转送源
		MOV		EDI,DSKCAC		; 转送目的地
		MOV		ECX,512/4
		CALL	memcpy

; 剩下的

		MOV		ESI,DSKCAC0+512	; 转送源
		MOV		EDI,DSKCAC+512	; 转送目的地
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 从柱面数变换为字节数/4
		SUB		ECX,512/4		; 减去IPL
		CALL	memcpy

; myOS工作完成，剩下由bootpack完成

; bootpack启动

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 没有转送的东西时
		MOV		ESI,[EBX+20]	; 转送源
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 转送目的地
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 找初始值
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		IN		 AL,0x60
		JNZ		waitkbdout		; AND的结果不是0，则跳转到waitkbdout
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 减法运算的结果若不是0，就跳转到memcpy
		RET


		ALIGNB	16
GDT0:
		RESB	8				; NULL selector
		DW		0xffff,0x0000,0x9200,0x00cf	; 可读写的段 32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; 可执行的段（bootpack用）

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack: