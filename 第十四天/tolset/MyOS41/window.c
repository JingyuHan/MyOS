#include "bootpack.h"

void make_window(unsigned char *buf, int xsize, int ysize, char *title,char act)
{
	
	boxfill8(buf, xsize,  8, 	  	 0,         0, xsize - 1, 		  0);
	boxfill8(buf, xsize,  7, 	     1,         1, xsize - 2,		  1);
	boxfill8(buf, xsize,  8, 	 	 0,         0,         0, ysize - 1);
	boxfill8(buf, xsize,  7, 	 	 1,         1,         1, ysize - 2);
	boxfill8(buf, xsize, 15, xsize - 2, 		1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize,  0, xsize - 1, 		0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize,  8,		 2,         2, xsize - 3, ysize - 3);
	
	boxfill8(buf, xsize, 15,		 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize,  0,		 0, ysize - 1, xsize - 1, ysize - 1);
	
	make_wtitle8(buf, xsize, title, act);
	return;
}

void make_wtitle8(unsigned char *buf, int xsize, char *title, char act){
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
	unsigned char c,tbc;
	if (act != 0) {
		tbc = 12;
	} else {
		tbc = 15;
	}
	boxfill8(buf, xsize,tbc, 		 3,         3, xsize - 4, 		 20);
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
				c=99;
			} else {
				c = 7;
			}
			buf[(5 + y) * xsize + (xsize - 38 + x)] = c;
		}
	}
	return;
}

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, 15, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, 15, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, 7, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, 7, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, 0, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, 0, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, 8, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, 8, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c, x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}


void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}

void change_wtitle8(struct SHEET *sht, char act)
{



	int x, y, xsize = sht->bxsize;
	char c, tc_new, tbc_new, tc_old, tbc_old, *buf = sht->buf;
	if (act != 0) {
		tc_new  = 7;
		tbc_new = 12;
		tc_old  = 8;
		tbc_old = 15;
	} else {
		tc_new  = 8;
		tbc_new = 15;
		tc_old  = 7;
		tbc_old = 12;
	}
	for (y = 3; y <= 20; y++) {
		for (x = 3; x <= xsize - 4; x++) {
			c = buf[y * xsize + x];
			if (c == tc_old && x <= xsize - 22) {
				c = tc_new;
			} else if (c == tbc_old) {
				c = tbc_new;
			}
			buf[y * xsize + x] = c;
		}
	}
	sheet_refresh(sht, 3, 3, xsize, 21);
	return;
}