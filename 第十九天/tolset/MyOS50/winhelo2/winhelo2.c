#include "apilib.h"



void HariMain(void)
{
	int win;
	char buf[150 * 50];
	win = api_openwin(buf, 250, 150, -1, "hello");
	api_boxfilwin(win,  50, 50, 170, 70, 3 /* 黄色 */);
	api_putstrwin(win, 50, 50, 0 /* 黑色 */, 15, "window open API");
	for (;;) {
		if (api_getkey(1) == 0x0a) {
			break; /* Enter则break; */
		}
	}
	api_end();
}
