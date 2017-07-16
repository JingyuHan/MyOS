#include<stdio.h>
#include "bootpack.h"

void make_window(unsigned char *buf, int xsize, int ysize, char *title);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO8 timerfifo, timerfifo2, timerfifo3;
	char s[30], keybuf[32], mousebuf[128], timerbuf[8], timerbuf2[8], timerbuf3[8];
	struct TIMER *timer, *timer2, *timer3;
	int mx,my,i;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, buf_mouse[256], *buf_win;
	
	init_gdtidt();
	init_pic();
	io_sti(); //IDT/PIC初始化完成，开放CPU中断
	
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	init_pit();
	io_out8(PIC0_IMR, 0xf8); 	//开放PIC1和键盘中断（11111001）
	io_out8(PIC1_IMR, 0xef);	//开放鼠标中断（11101111）
	
	fifo8_init(&timerfifo, 8, timerbuf);
	timer = timer_alloc();
	timer_init(timer, &timerfifo, 1);
	timer_settime(timer, 1000);
	fifo8_init(&timerfifo2, 8, timerbuf2);
	timer2 = timer_alloc();
	timer_init(timer2, &timerfifo2, 1);
	timer_settime(timer2, 300);
	fifo8_init(&timerfifo3, 8, timerbuf3);
	timer3 = timer_alloc();
	timer_init(timer3, &timerfifo3, 1);
	timer_settime(timer3, 50);
	
	init_keyboard();
	enable_mouse(&mdec);
	
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	
	init_palette();		//设定调色板
	
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back  = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win   = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); 
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, buf_win, 160, 52, 99);	
	init_screen(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);
	make_window(buf_win, 160, 52, "counter");
	sheet_slide(sht_back, 0, 0);
	
	
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	
	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_back,  0);
	sheet_updown(sht_win,   1);
	sheet_updown(sht_mouse, 2);
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, 9, s);
	
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, 7, s);
	sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);
	
	while(1){
		
		sprintf(s, "%010d", timerctl.count);
		boxfill8(buf_win, 160, 8, 40, 28, 119, 43);
		putfonts8_asc(buf_win, 160, 40, 28, 0, s);
		sheet_refresh(sht_win, 40, 28, 120, 44);
		
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo)+ fifo8_status(&timerfifo)
				+ fifo8_status(&timerfifo2) + fifo8_status(&timerfifo3)== 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(buf_back, binfo->scrnx, 14,  0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, 7, s);
				sheet_refresh(sht_back, 0, 16, 16, 32);
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
					/* 数据的3个字节到齐，显示 */
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(buf_back, binfo->scrnx, 14, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(buf_back, binfo->scrnx, 32, 16, 7, s);
					sheet_refresh(sht_back, 32, 16, 32 + 15 * 8, 32);
					/* 鼠标指针移动 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(buf_back, binfo->scrnx, 14, 0, 0, 79, 15); /* 隐藏坐标 */
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, 7, s); /* 显示坐标 */
					sheet_refresh(sht_back, 0, 0, 80, 16);
					sheet_slide( sht_mouse, mx, my);
				}
			}else if(fifo8_status(&timerfifo) != 0){
				i = fifo8_get(&timerfifo); /* 读入（设定起始点） */
				io_sti();
				putfonts8_asc(buf_back, binfo->scrnx, 0, 64, 7, "10[sec]");
				sheet_refresh(sht_back, 0, 64, 56, 80);
			} else if (fifo8_status(&timerfifo2) != 0) {
				i = fifo8_get(&timerfifo2); /* 读入（设定起始点） */
				io_sti();
				putfonts8_asc(buf_back, binfo->scrnx, 0, 80, 7, "3[sec]");
				sheet_refresh(sht_back, 0, 80, 48, 96);
			} else if (fifo8_status(&timerfifo3) != 0) {
				i = fifo8_get(&timerfifo3);
				io_sti();
				if (i != 0) {
					timer_init(timer3, &timerfifo3, 0); /* 设为0 */
					boxfill8(buf_back, binfo->scrnx, 7, 8, 96, 15, 111);
				} else {
					timer_init(timer3, &timerfifo3, 1); /* 设为1 */
					boxfill8(buf_back, binfo->scrnx, 8, 8, 96, 15, 111);
				}
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 96, 16, 112);
			}
		}
	}
}

void make_window(unsigned char *buf, int xsize, int ysize, char *title)
{
	static char closebtn[14][16] = {
		"QQOOOOOOOOOOOOQQ",
		"QOQQQQQQQQQQQQOQ",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"QO$$$$$$$$$$$$OQ",
		"QQ@@@@@@@@@@@@QQ"
	};
	static char smallbtn[14][16] = {
		"QQOOOOOOOOOOOOQQ",
		"QOQQQQQQQQQQQQOQ",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQ@@@@@@@@@@Q$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"QO$$$$$$$$$$$$OQ",
		"QQ@@@@@@@@@@@@QQ"
	};
	int x, y;
	char c;
	boxfill8(buf, xsize, 8, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, 7, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, 8, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, 7, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, 15, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, 0, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, 8, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, 99, 3,         3,         xsize - 4, 20       );
	boxfill8(buf, xsize, 15, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, 0, 0,         ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, 7, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = 0;
			} else if (c == '$') {
				c = 15;
			} else if (c == 'Q') {
				c = 99;
			} else {
				c = 7;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = smallbtn[y][x];
			if (c == '@') {
				c = 0;
			} else if (c == '$') {
				c = 15;
			} else if (c == 'Q') {
				//c = 8;
				c=99;
			} else {
				c = 7;
			}
			buf[(5 + y) * xsize + (xsize - 38 + x)] = c;
		}
	}
	return;
}


