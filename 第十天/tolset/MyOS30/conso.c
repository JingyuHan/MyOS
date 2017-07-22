#include "bootpack.h"
#include <stdio.h>
#include <string.h>

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int i, fifobuf[128] , *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	char cmdline[30];
	struct CONSOLE cons;
	
	cons.sht = sheet;
	cons.cur_x =  8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	
	fifo32_init(&task->fifo, 128, fifobuf, task);

	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);
	
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));

	//显示提示符
	cons_putchar(&cons,'>',1);
	
	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) { /* 光标定时器 */
				if (i != 0) {
					timer_init(timer, &task->fifo, 0); /* 接下来置0 */
					if (cons.cur_c >= 0) {
						cons.cur_c = 7;
					}
				} else {
					timer_init(timer, &task->fifo, 1); /* 接下来置1 */
					if (cons.cur_c >= 0) {
						cons.cur_c = 0;
					}
				}
				timer_settime(timer, 50);
			}
			if (i == 2) {	/* 光标ON */
				cons.cur_c = 7;
			}
			if (i == 3) {	/* 光标OFF */
				boxfill8(sheet->buf, sheet->bxsize, 0, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				cons.cur_c = -1;
			}
			if (256 <= i && i <= 511) { /* 键盘数据（通过任务A） */
				if (i == 8 + 256) {
					/* 退格键 */
					if (cons.cur_x > 16) {
						/* 用空白擦除光标，光标前移一位 */
						cons_putchar(&cons , ' ' , 0);
						cons.cur_x -= 8;
					}
				} else if (i == 10 + 256) {
					/* Enter */
					/* 将光标用空格擦除后换行 */
					cons_putchar(&cons , ' ' , 0);
					cmdline[cons.cur_x / 8 - 2] = 0;
					cons_newline(&cons);
					
					cons_runcmd(cmdline, &cons, fat, memtotal);
					
					/* 显示提示符 */
					cons_putchar(&cons, '>', 1);
				} else {
					/* 一般字符 */
					if (cons.cur_x < 240) {
						/* 显示一个字符，光标后移一位 */
						cmdline[cons.cur_x / 8 - 2] = i - 256;
						cons_putchar(&cons , i-256 , 1);
					}
				}
			}
			/* 重新显示光标 */
			if (cons.cur_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
			}
			sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
		}
	}
}

void cons_putchar(struct CONSOLE *cons, int chr, char move){
	char s[2];
	s[0] = chr;
	s[1] = 0;
	if (s[0] == 0x09) {	/* Tab */
		for (;;) {
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, 7, 0, " ", 1);
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 240) {
				cons_newline(cons);
			}
			if (((cons->cur_x - 8) & 0x1f) == 0) {
				break;	/* 被32整除则break */
			}
		}
	} else if (s[0] == 0x0a) {	/* 换行 */
		cons_newline(cons);
	} else if (s[0] == 0x0d) {	/* 回车 */
		/* 暂不做处理 */
	} else {	/* 一般字符 */
		putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, 7, 0, s, 1);
		if (move != 0) {
			/* move为0时光标不后移 */
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 240) {
				cons_newline(cons);
			}
		}
	}
	return;
}

void cons_newline(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	if (cons->cur_y < sheet->bysize-33) {
		cons->cur_y += 16; /* 换行 */
	} else {
		/* 滚动 */
		for (y = 28; y < sheet->bysize-33; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		for (y = sheet->bysize-33; y < sheet->bysize-17; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = 0;
			}
		}
		sheet_refresh(sheet, 8, 28, 8 + 240, sheet->bysize-17);
	}
	cons->cur_x = 8;
	return ;
}

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal)
{
	if (strcmp(cmdline, "mem") == 0) {
		cmd_mem(cons, memtotal);
	} else if (strcmp(cmdline, "cls") == 0) {
		cmd_cls(cons);
	} else if (strcmp(cmdline, "ls") == 0) {
		cmd_ls(cons);
	} else if (strncmp(cmdline, "cat ", 4) == 0) {
		cmd_cat(cons, fat, cmdline);
	} else if (strcmp(cmdline, "hlt") == 0) {
		cmd_hlt(cons, fat);
	} else if (cmdline[0] != 0) {
		/* 不是命令，也不是空行 */
		putfonts8_asc_sht(cons->sht, 8, cons->cur_y, 7, 0, "No such command.", 16);
		cons_newline(cons);
		cons_newline(cons);
	}
	return;
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal){
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[30];
	sprintf(s, "total   %dMB", memtotal / (1024 * 1024));
	putfonts8_asc_sht(cons->sht, 8, cons->cur_y, 7, 0, s, 30);
	cons_newline(cons);
	sprintf(s, "free %dKB", memman_total(memman) / 1024);
	putfonts8_asc_sht(cons->sht, 8, cons->cur_y, 7, 0, s, 30);
	cons_newline(cons);
	cons_newline(cons);
	return;
}

void cmd_cls(struct CONSOLE *cons){
	int x, y;
	struct SHEET *sheet = cons->sht;
	for (y = 28; y < sheet->bysize-9; y++) {
		for (x = 8; x < 8 + 240; x++) {
			sheet->buf[x + y * sheet->bxsize] = 0;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + 240, sheet->bysize-9 );
	cons->cur_y = 28;
}

void cmd_ls(struct CONSOLE *cons){
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];
	for (i = 0; i < 224; i++) {
		if (finfo[i].name[0] == 0x00) {
			break;
		}
		if (finfo[i].name[0] != 0xe5) {
			if ((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d", finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j];
				}
				s[ 9] = finfo[i].ext[0];
				s[10] = finfo[i].ext[1];
				s[11] = finfo[i].ext[2];
				putfonts8_asc_sht(cons->sht, 8, cons->cur_y, 7, 0, s, 30);
				cons_newline(cons);
			}
		}
	}
	cons_newline(cons);
	return;
}

void cmd_cat(struct CONSOLE *cons, int *fat, char *cmdline){
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo = file_search(cmdline + 4, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	char *p;
	int i;
	if (finfo != 0) {
		/* 找到文件的情况 */
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		for (i = 0; i < finfo->size; i++) {
			cons_putchar(cons, p[i], 1);
		}
		memman_free_4k(memman, (int) p, finfo->size);
	} else {
		/* 没有找到文件的情况 */
		putfonts8_asc_sht(cons->sht, 8, cons->cur_y, 7, 0, "File not found.", 15);
		cons_newline(cons);
	}
	cons_newline(cons);
	return;
}

void cmd_hlt(struct CONSOLE *cons, int *fat){
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo = file_search("HLT.HRB", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	char *p;
	
	if (finfo != 0) {
		/* 找到文件的情况 */
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER);
		farjmp(0, 1003 * 8);
		memman_free_4k(memman, (int) p, finfo->size);
	} else {
		/* 没有找到文件的情况 */
		putfonts8_asc_sht(cons->sht, 8, cons->cur_y, 7, 0, "File not found.", 15);
		cons_newline(cons);
	}
	cons_newline(cons);
	return;
}
