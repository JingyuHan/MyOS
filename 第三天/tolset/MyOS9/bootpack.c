void io_hlt(void);

void HariMain(void)
{

fin:
	/* 实现HLT */
	io_hlt();
	goto fin;

}
