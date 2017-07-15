/* FIFO���C�u���� */

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
/* FIFO���n�� */
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /* ?�t��召 */
	fifo->flags = 0;
	fifo->p = 0; /* ���꘢�����ʓ��I�ʒu */
	fifo->q = 0; /* ���꘢����?�o�I�ʒu */
	return;
}

int fifo8_put(struct FIFO8 *fifo, unsigned char data)
/* ��FIFO??������ۑ� */
{
	if (fifo->free == 0) {
		/* ��o */
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	return 0;
}

int fifo8_get(struct FIFO8 *fifo)
/* ��FIFO?��꘢����*/
{
	int data;
	if (fifo->free == fifo->size) {
		/* ��?�t��?��C�ԉ�-1 */
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

int fifo8_status(struct FIFO8 *fifo)
/* �ԉ�?�I�������� */
{
	return fifo->size - fifo->free;
}
