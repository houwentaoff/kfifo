/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Jz.
 *       Filename:  ringbuf.c 位于KDIR/kernel/kfifo.c
 *
 *    Description:  环形缓冲区的实现
 *         Others:  1.min的妙用，(验证剩余有效空间和要求要读出或者写入空间 取最小值)
 *                  2.利用unsigned int 的回环,in 和 out一直在加，加到0xffffffff则归为0，任然满足计算偏移等。
 *                  3.分为2部进行copy，一为当前偏移到size-1 二为剩余部分0到(len减去一中的个数)
 *                  4.unsiged int下的(in - out)始终为in和out之间的距离，(in溢出后in:0x1 - out:0xffffffff = 2任然满足)(缓冲区中未脏的数据).
 *                  5.计算偏移(in) & (size - 1) <==> in%size
 *        Version:  1.0
 *        Date:  Monday, April 28, 2016 16:00:53 HKT
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joy (Joy), 
 *   Organization:  jz
 *        History:   Created by housir
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "kfifo.h"

#define min(a,b)  ((a)>(b) ? (b) : (a))            /*  */

#define roundup_pow_of_two(n)   \
    (1UL    <<                             \
        (                                  \
            (                              \
            (n) & (1UL << 31) ? 31 :       \
            (n) & (1UL << 30) ? 30 :       \
            (n) & (1UL << 29) ? 29 :       \
            (n) & (1UL << 28) ? 28 :       \
            (n) & (1UL << 27) ? 27 :       \
            (n) & (1UL << 26) ? 26 :       \
            (n) & (1UL << 25) ? 25 :       \
            (n) & (1UL << 24) ? 24 :       \
            (n) & (1UL << 23) ? 23 :       \
            (n) & (1UL << 22) ? 22 :       \
            (n) & (1UL << 21) ? 21 :       \
            (n) & (1UL << 20) ? 20 :       \
            (n) & (1UL << 19) ? 19 :       \
            (n) & (1UL << 18) ? 18 :       \
            (n) & (1UL << 17) ? 17 :       \
            (n) & (1UL << 16) ? 16 :       \
            (n) & (1UL << 15) ? 15 :       \
            (n) & (1UL << 14) ? 14 :       \
            (n) & (1UL << 13) ? 13 :       \
            (n) & (1UL << 12) ? 12 :       \
            (n) & (1UL << 11) ? 11 :       \
            (n) & (1UL << 10) ? 10 :       \
            (n) & (1UL <<  9) ?  9 :       \
            (n) & (1UL <<  8) ?  8 :       \
            (n) & (1UL <<  7) ?  7 :       \
            (n) & (1UL <<  6) ?  6 :       \
            (n) & (1UL <<  5) ?  5 :       \
            (n) & (1UL <<  4) ?  4 :       \
            (n) & (1UL <<  3) ?  3 :       \
            (n) & (1UL <<  2) ?  2 :       \
            (n) & (1UL <<  1) ?  1 :       \
            (n) & (1UL <<  0) ?  0 : -1    \
            ) + 1                          \
        )                                  \
)
/*  */

/**
 * @brief 
 *
 * @param fifo    缓冲区的数据结构
 * @param buffer  
 * @param size    缓冲区的大小
 */
void kfifo_init(struct kfifo *fifo, void *buffer, unsigned int size)
{
    /* 检查时候是2的指数倍 */
    fifo->buffer = buffer;
    fifo->size = size;

	fifo->in = fifo->out = 0;
}

/**
 * @brief 
 *
 * @param fifo
 * @param size
 *
 * @return 
 */
int kfifo_alloc(struct kfifo *fifo, unsigned int size/*, gfp_t gfp_mask*/)
{
	unsigned char *buffer;
    /* 检查时候是2的指数倍  */
    if (size & (size-1))
    {
//        size = 1UL << (roundup_pow_of_two(size) + 1);
        size = roundup_pow_of_two(size);
    }
    buffer = (unsigned char *)malloc(size);
    if (!buffer)
    {
        kfifo_init(fifo, NULL, 0);
        return -1;
    }

    kfifo_init(fifo, buffer, size);

    return 0;
}

/**
 * @brief 加入缓冲队列
 *
 * @param fifo
 * @param from
 * @param len
 *
 * @return 实际放入缓冲区的长度(可能小于len或者为0)
 */
unsigned int kfifo_in(struct kfifo *fifo, const void *from,
				unsigned int len)
{
    unsigned int l,off;

    /* min(大小 - 已经用了的 , len) 计算剩余可用空间 好代码 */
    len = min(fifo->size - (fifo->in - fifo->out), len);
    /*计算索引(0 ... size-1)*/
    off = (fifo->in + 0) & (fifo->size - 1);/*==> (in+0) % size*/
    /* 分成2段,1.当前位置到size-1 2.0到0或者0到len-l(剩余的) */
    l = min(len, fifo->size - off);
	memcpy(fifo->buffer + off, from, l);

    /* 将剩余的copy到buffer，如果len == l则啥也不干,好代码 */
	memcpy(fifo->buffer, (char *)from + l, len - l);

    fifo->in += len;

    return len;
}

/**
 * @brief 
 *
 * @param fifo
 * @param to    
 * @param len
 *
 * @return 实际读出缓冲区的长度(可能小于len,或者为0)
 */
unsigned int kfifo_out(struct kfifo *fifo, void *to, unsigned int len)
{
    unsigned int off;
	unsigned int l;
    /* 验证len是否大于缓冲区存的值  */
    len = min(fifo->in - fifo->out, len);

    off = (fifo->out + 0) & (fifo->size - 1);/*==> (out+0) % size*/
	/* first get the data from fifo->out until the end of the buffer */
	l = min(len, fifo->size - off);
	memcpy(to, fifo->buffer + off, l);
	/* then get the rest (if any) from the beginning of the buffer */
	memcpy((char *)to + l, fifo->buffer, len - l);

  	fifo->out += len;

	return len;
}

#ifdef KFIFO_EXT

unsigned int kfifo_tail(struct kfifo *fifo)
{
    unsigned int off;

    off = (fifo->out + 0) & (fifo->size - 1);/*==> (out+0) % size*/    

    return off;
}
/**
 * @brief 
 *
 * @param fifo
 * @param len
 *
 * @return 实际读出缓冲区的长度(可能小于len,或者为0)
 */
unsigned int kfifo_out2(struct kfifo *fifo, unsigned int len)
{
    unsigned int off;
	unsigned int l;
    /* 验证len是否大于缓冲区存的值  */
    len = min(fifo->in - fifo->out, len);

    off = (fifo->out + 0) & (fifo->size - 1);/*==> (out+0) % size*/
	/* first get the data from fifo->out until the end of the buffer */
	l = min(len, fifo->size - off);
//	memcpy(to, fifo->buffer + off, l);
	/* then get the rest (if any) from the beginning of the buffer */
//	memcpy((char *)to + l, fifo->buffer, len - l);

  	fifo->out += len;

	return len;
}
#endif
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:
 * =====================================================================================
 */
#if 0 //test 1 KFIFO_EXT
int main ( int argc, char *argv[] )
{
    kfifo_t objFifo;
    kfifo_t prt_Fifo;
    static PRT_OBJ objBuf[BUFF_OBJ_MAX]={0}
    static BYTE prtBuf[BUFF_OBJ_MAX][MAX_LINE_CHAR]={0}; //打印缓冲区

    PRT_OBJ TempObj;
    PRT_OBJ outTempObj;
    kfifo_init(&objFifo, (void *)objBuf, sizeof(objBuf));
    kfifo_init(&prt_Fifo, (void *)prtBuf, sizeof(prtBuf[0]));

    kfifo_in(&prt_Fifo, (void *)g_PrintBuf, MAX_LINE_CHAR);
    kfifo_in(&objFifo, TempObj, sizeof(TempObj));
    TempObj.pBuf = kfifo_tail(&prt_Fifo);

    kfifo_out2(&prt_Fifo, MAX_LINE_CHAR);
    kfifo_out(&objFifo, &outTempObj, sizeof(outTempObj));
    return 0;
}
#else
#endif
#if 0   //test2 KFIFO
int main ( int argc, char *argv[] )
{
	char outbuf[100];
	Kfifo myfifo;
	Kfifo *pmyfifo=&myfifo;

	printf("%u\n", (unsigned int)(0x0 - 0xfffffffe));

	if (-1 == kfifo_alloc(pmyfifo, 0x1<<20))
	{
		return -1;
	}
	if (strlen("123456789") != kfifo_in(pmyfifo, "123456789", strlen("123456789")))
	{
		printf("error!\n");
	}
	if (strlen("asdfghj") != kfifo_in(pmyfifo, "asdfghj", strlen("asdfghj")))
    {
		printf("error!\n");
    }
	if (sizeof("cvbtyn") != kfifo_in(pmyfifo, "cvbtyn", sizeof("cvbtyn")))
    {
		printf("error!\n");
    }

	if (14 != kfifo_out(pmyfifo, outbuf, 14))
    {
		printf("error!\n");
    }
	if (strlen("qwertyuio") != kfifo_in(pmyfifo, "qwertyuio", strlen("qwertyuio")))
    {
		printf("error!\n");
    }
	if (14 != kfifo_out(pmyfifo, outbuf+14, 14))
    {
		printf("error!\n");
    }

    return EXIT_SUCCESS;
}			
#endif
