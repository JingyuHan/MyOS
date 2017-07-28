int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_end(void);

char buf[150 * 50];

void HariMain(void)
{
	int win;
	win = api_openwin(buf, 250, 150, 99, "hello");
	api_boxfilwin(win,  50, 50, 170, 70, 3 /* 黄色 */);
	api_putstrwin(win, 50, 50, 0 /* 黑色 */, 15, "window open API");
	api_end();
}
