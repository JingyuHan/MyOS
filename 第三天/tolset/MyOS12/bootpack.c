#include<stdio.h>
#include "bootpack.h"
	
void HariMain(void)
{
	char *s,mcursor[256];
	int mx,my;
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	
	init_gdtidt();
	init_pic();
	io_sti(); //
	
	init_palette();		//设定调色板

	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, 14);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, 9, s);
	
	io_out8(PIC0_IMR, 0xf9); 
	io_out8(PIC1_IMR, 0xef); 
	
	while(1){
		io_hlt();
	}
	
	

}
