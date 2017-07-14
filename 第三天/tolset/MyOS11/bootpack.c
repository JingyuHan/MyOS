void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
//boxfill8 参数表：内存位置指针，屏幕宽度，颜色，宽起始，高起始，宽截止，高截止
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);


#define COL8_000000		0			//黒 
#define COL8_FF0000		1           //亮红
#define COL8_00FF00		2           //亮绿
#define COL8_FFFF00		3           //亮黄
#define COL8_0000FF		4           //亮蓝
#define COL8_FF00FF		5           //亮紫
#define COL8_00FFFF		6           //浅亮蓝
#define COL8_FFFFFF		7           //白
#define COL8_C6C6C6		8           //亮灰
#define COL8_840000		9           //暗红
#define COL8_008400		10          //暗绿
#define COL8_848400		11          //暗黄
#define COL8_000084		12          //暗青
#define COL8_840084		13          //暗紫
#define COL8_008484		14          //浅暗蓝
#define COL8_848484		15          //暗灰

void HariMain(void)
{

	int xsize,ysize;
	char *vram;			//地址指针
	
	init_palette();		//设定调色板
	
	vram = (char *)0xa0000;
	xsize = 320;
	ysize = 200;
	
	boxfill8(vram,xsize,14,0	   ,0		,xsize-1 ,ysize-29);
	boxfill8(vram,xsize,8 ,0	   ,ysize-28,xsize-1 ,ysize-28);
	boxfill8(vram,xsize,7 ,0	   ,ysize-27,xsize-1 ,ysize-27);
	boxfill8(vram,xsize,8 ,0	   ,ysize-26,xsize-1 ,ysize-1 );
	
	boxfill8(vram,xsize, 6,80	   ,ysize-160,87	,ysize-60);
	boxfill8(vram,xsize, 6,80	   ,ysize-59,140	,ysize-52);
	boxfill8(vram,xsize, 6,190	   ,ysize-104,240	,ysize-100);
	boxfill8(vram,xsize, 6,183	   ,ysize-160,189	,ysize-52);
	boxfill8(vram,xsize, 6,241	   ,ysize-160,247	,ysize-52);
	
	
	boxfill8(vram,xsize,7 ,3	   ,ysize-24,59	     ,ysize-24);
	boxfill8(vram,xsize,7 ,2	   ,ysize-24,2 	     ,ysize- 4);
	boxfill8(vram,xsize,15,3	   ,ysize- 4,59	     ,ysize- 4);
	boxfill8(vram,xsize,15,59	   ,ysize-23,59	     ,ysize- 5);
	boxfill8(vram,xsize,0 ,2	   ,ysize-3 ,59	     ,ysize- 3);
	boxfill8(vram,xsize,0 ,60	   ,ysize-24,60	     ,ysize- 3);
	
	boxfill8(vram,xsize,15,xsize-47,ysize-24,xsize- 4,ysize-24);
	boxfill8(vram,xsize,15,xsize-47,ysize-23,xsize-47,ysize- 4);
	boxfill8(vram,xsize,7 ,xsize-47,ysize- 3,xsize- 4,ysize- 3);
	boxfill8(vram,xsize,7 ,xsize- 3,ysize-24,xsize- 3,ysize- 3);
	
	while(1){
		io_hlt();
	}
	
	

}

void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	//  0:黒 
		0xff, 0x00, 0x00,	//  1:亮红
		0x00, 0xff, 0x00,	//  2:亮绿
		0xff, 0xff, 0x00,	//  3:亮黄
		0x00, 0x00, 0xff,	//  4:亮蓝
		0xff, 0x00, 0xff,	//  5:亮紫
		0x00, 0xff, 0xff,	//  6:浅亮蓝
		0xff, 0xff, 0xff,	//  7:白
		0xc6, 0xc6, 0xc6,	//  8:亮灰
		0x84, 0x00, 0x00,	//  9:暗红
		0x00, 0x84, 0x00,	// 10:暗绿
		0x84, 0x84, 0x00,	// 11:暗黄
		0x00, 0x00, 0x84,	// 12:暗青
		0x84, 0x00, 0x84,	// 13:暗紫
		0x00, 0x84, 0x84,	// 14:浅暗蓝
		0x84, 0x84, 0x84	// 15:暗灰
	};
	set_palette(0, 15, table_rgb);
	return;

}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();	// 记录中断许可标志位 
	io_cli(); 					// 将中断许可标志位置0，禁止许可
	io_out8(0x03c8, start);
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	// 复原中断许可标志
	return;
}
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}
