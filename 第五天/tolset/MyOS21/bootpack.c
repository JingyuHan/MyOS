#include<stdio.h>
#include "bootpack.h"

void make_window(unsigned char *buf, int xsize, int ysize, char *title);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO32 fifo;
	char s[30];
	int fifobuf[128];
	struct TIMER *timer, *timer2, *timer3;
	int mx,my,i, count = 0;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, buf_mouse[256], *buf_win;
	
	init_gdtidt();
	init_pic();
	io_sti(); //IDT/PIC初始化完成，开放CPU中断
	
	fifo32_init(&fifo, 128, fifobuf);
	init_pit();
	
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	
	io_out8(PIC0_IMR, 0xf8); 	//开放PIC1和键盘中断（11111001）
	io_out8(PIC1_IMR, 0xef);	//开放鼠标中断（11101111）
	
	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000);
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);
	
	
	
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
	putfonts8_asc_sht(sht_back, 0, 0, 7, 14, s, 10);
	
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, 7, s);
	putfonts8_asc_sht(sht_back, 0, 32, 7, 14, s, 40);
	
	while(1){
		
		count++;
		
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511) {
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, 7, 14, s, 2);
			} else if (512 <= i && i <= 767) {
				if (mouse_decode(&mdec, i - 512) != 0) {
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
					putfonts8_asc_sht(sht_back, 32, 16, 7, 14, s, 15);
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
					putfonts8_asc_sht(sht_back, 0, 0, 7, 14, s, 10);
					sheet_slide( sht_mouse, mx, my);
				}
			}else if(i == 10){
				putfonts8_asc_sht(sht_back, 0, 64, 7, 14, "10[sec]", 7);
				sprintf(s, "%010d", count);
				putfonts8_asc_sht(sht_win, 40, 28, 0, 8, s, 10);
			} else if (i == 3) {
				putfonts8_asc_sht(sht_back, 0, 80, 7, 14, "3[sec]", 6);
				count = 0; /* 测试开始 */
			}else if (i == 1) { 
				timer_init(timer3, &fifo, 0);
				boxfill8(buf_back, binfo->scrnx, 7, 8, 96, 15, 111);
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 96, 16, 112);
			} else if (i == 0) {
				timer_init(timer3, &fifo, 1); 
				boxfill8(buf_back, binfo->scrnx, 14, 8, 96, 15, 111);
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

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}
