#include "apilib.h"

#include <stdio.h>

int strtol(char *s, char **endp, int base);	/* ?弝乮stdlib.h乯 */

char *skipspace(char *p);
void textview(int win, int w, int h, int xskip, char *p, int tab, int lang);
char *lineview(int win, int w, int y, int xskip, unsigned char *p, int tab, int lang);
int puttab(int x, int w, int xskip, char *s, int tab);

void HariMain(void)
{
	char winbuf[1024 * 757], txtbuf[240 * 1024];
	int w = 70, h = 20, t = 4, spd_x = 1, spd_y = 1;
	int win, i, j, lang = api_getlang(), xskip = 0;
	char s[30], *p, *q = 0, *r = 0;

	/* 柦椷峴夝愅 */
	api_cmdline(s, 30);
	for (p = s; *p > ' '; p++) { }	/* ?摓嬻奿?巭 */
	for (; *p != 0; ) {
		p = skipspace(p);
		if (*p == '-') {
			if (p[1] == 'w') {
				w = strtol(p + 2, &p, 0);
				if (w < 20) {
					w = 20;
				}
				if (w > 126) {
					w = 126;
				}
			} else if (p[1] == 'h') {
				h = strtol(p + 2, &p, 0);
				if (h < 1) {
					h = 1;
				}
				if (h > 45) {
					h = 45;
				}
			} else if (p[1] == 't') {
				t = strtol(p + 2, &p, 0);
				if (t < 1) {
					t = 4;
				}
			} else {
err:
				api_putstr0(" >tview file [-w30 -h10 -t4]\n");
				api_end();
			}
		} else {	/* 漄摓暥審柤 */
			if (q != 0) {
				goto err;
			}
			q = p;
			for (; *p > ' '; p++) { }	/* ?摓嬻奿?巭 */
			r = p;
		}
	}
	if (q == 0) {
		goto err;
	}

	/* 弝?鈞岥 */
	win = api_openwin(winbuf, w * 8 + 16, h * 16 + 37, -1, "tview");
	api_boxfilwin(win, 6, 27, w * 8 + 9, h * 16 + 30, 7);

	/* ?擖暥審 */
	*r = 0;
	i = api_fopen(q);
	if (i == 0) {
		api_putstr0("file open error.\n");
		api_end();
	}
	j = api_fsize(i, 0);
	if (j >= 240 * 1024 - 1) {
		j = 240 * 1024 - 2;
	}
	txtbuf[0] = 0x0a; /* ?暫梡揑?峴戙? */
	api_fread(txtbuf + 1, j, i);
	api_fclose(i);
	txtbuf[j + 1] = 0;
	q = txtbuf + 1;
	for (p = txtbuf + 1; *p != 0; p++) {	/* ?椆??棟?摼??丆?漿0x0d揑戙? */
		if (*p != 0x0d) {
			*q = *p;
			q++;
		}
	}
	*q = 0;

	/* 庡懱 */
	p = txtbuf + 1;
	for (;;) {
		textview(win, w, h, xskip, p, t, lang);
		i = api_getkey(1);
		if (i == 'Q' || i == 'q') {
			api_end();
		}
		if ('A' <= i && i <= 'F') {
			spd_x = 1 << (i - 'A');	/* 1, 2, 4, 8, 16, 32 */
		}
		if ('a' <= i && i <= 'f') {
			spd_y = 1 << (i - 'a');	/* 1, 2, 4, 8, 16, 32 */
		}
		if (i == '<' && t > 1) {
			t /= 2;
		}
		if (i == '>' && t < 256) {
			t *= 2;
		}
		if (i == '4') {
			for (;;) {
				xskip -= spd_x;
				if (xskip < 0) {
					xskip = 0;
				}
				if (api_getkey(0) != '4') { /* 傕偆'4'擛壥杤桳埪壓乬4乭,??棟?懇 */
					break;
				}
			}
		}
		if (i == '6') {
			for (;;) {
				xskip += spd_x;
				if (api_getkey(0) != '6') {
					break;
				}
			}
		}
		if (i == '8') {
			for (;;) {
				for (j = 0; j < spd_y; j++) {
					if (p == txtbuf + 1) {
						break;
					}
					for (p--; p[-1] != 0x0a; p--) { } /* 夞摓忋堦槩帤晞?0x0a?巭 */
				}
				if (api_getkey(0) != '8') {
					break;
				}
			}
		}
		if (i == '2') {
			for (;;) {
				for (j = 0; j < spd_y; j++) {
					for (q = p; *q != 0 && *q != 0x0a; q++) { }
					if (*q == 0) {
						break;
					}
					p = q + 1;
				}
				if (api_getkey(0) != '2') {
					break;
				}
			}
		}
	}
}

char *skipspace(char *p)
{
	for (; *p == ' '; p++) { }	/* 挼?嬻奿 */
	return p;
}

void textview(int win, int w, int h, int xskip, char *p, int tab, int lang)
{
	int i;
	api_boxfilwin(win + 1, 8, 29, w * 8 + 7, h * 16 + 28, 7);
	for (i = 0; i < h; i++) {
		p = lineview(win, w, i * 16 + 29, xskip, p, tab, lang);
	}
	api_refreshwin(win, 8, 29, w * 8 + 8, h * 16 + 29);
	return;
}

char *lineview(int win, int w, int y, int xskip, unsigned char *p, int tab, int lang)
{
	int x = - xskip;
	char s[130];
	for (;;) {
		if (*p == 0) {
			break;
		}
		if (*p == 0x0a) {
			p++;
			break;
		}
		if (lang == 0) {	/* ASCII */
			if (*p == 0x09) {
				x = puttab(x, w, xskip, s, tab);
			} else {
				if (0 <= x && x < w) {
					s[x] = *p;
				}
				x++;
			}
			p++;
		}
		if (lang == 1) {	/* SJIS */
			if (*p == 0x09) {
				x = puttab(x, w, xskip, s, tab);
				p++;
			} else if ((0x81 <= *p && *p <= 0x9f) || (0xe0 <= *p && *p <= 0xfc)) {
				/* 慡妏帤晞 */
				if (x == -1) {
					s[0] = ' ';
				}
				if (0 <= x && x < w - 1) {
					s[x]     = *p;
					s[x + 1] = p[1];
				}
				if (x == w - 1) {
					s[x] = ' ';
				} 
				x += 2;
				p += 2;
			} else {
				if (0 <= x && x < w) {
					s[x] = *p;
				}
				x++;
				p++;
			}
		}
		if (lang == 2) {	/* EUC */
			if (*p == 0x09) {
				x = puttab(x, w, xskip, s, tab);
				p++;
			} else if (0xa1 <= *p && *p <= 0xfe) {
				/* 慡妏帤晞 */
				if (x == -1) {
					s[0] = ' ';
				}
				if (0 <= x && x < w - 1) {
					s[x]     = *p;
					s[x + 1] = p[1];
				}
				if (x == w - 1) {
					s[x] = ' ';
				} 
				x += 2;
				p += 2;
			} else {
				if (0 <= x && x < w) {
					s[x] = *p;
				}
				x++;
				p++;
			}
		}
		if (lang == 3) {	/* EUC */
			if (*p == 0x09) {
				x = puttab(x, w, xskip, s, tab);
				p++;
			} else if (0xa1 <= *p && *p <= 0xfe) {
				/* 慡妏帤晞 */
				if (x == -1) {
					s[0] = ' ';
				}
				if (0 <= x && x < w - 1) {
					s[x]     = *p;
					s[x + 1] = p[1];
				}
				if (x == w - 1) {
					s[x] = ' ';
				} 
				x += 2;
				p += 2;
			} else {
				if (0 <= x && x < w) {
					s[x] = *p;
				}
				x++;
				p++;
			}
		}
	}
	if (x > w) {
		x = w;
	}
	if (x > 0) {
		s[x] = 0;
		api_putstrwin(win + 1, 8, y, 0, x, s);
	}
	return p;
}

int puttab(int x, int w, int xskip, char *s, int tab)
{
	for (;;) {
		if (0 <= x && x < w) {
			s[x] = ' ';
		}
		x++;
		if ((x + xskip) % tab == 0) {
			break;
		}
	}
	return x;
}
