/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Jz.
 *       Filename:  kfifo.h
 *
 *    Description:  
 *         Others:
 *
 *        Version:  1.0
 *        Created:  2016-04-28 13:22:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joy (Joy), 
 *   Organization:  jz
 *
 * =====================================================================================
 */
#ifndef __KFIFO_H__
#define __KFIFO_H__

#define KFIFO_EXT

typedef struct kfifo {
    unsigned char *buffer;/* the buffer holding the data */
    unsigned int size;/* the size of the allocated buffer */
    unsigned int in;/* data is added at offset (in % size) */
    unsigned int out;/* data is extracted from off. (out % size) */
}kfifo_t;			

void kfifo_init(struct kfifo *fifo, void *buffer, unsigned int size);
unsigned int kfifo_in(struct kfifo *fifo, const void *from,
				unsigned int len);
unsigned int kfifo_out(struct kfifo *fifo, void *to, unsigned int len);

#ifdef KFIFO_EXT
unsigned int kfifo_out2(struct kfifo *fifo, unsigned int len);
unsigned int kfifo_tail(struct kfifo *fifo);
#else
inline unsigned int kfifo_out2(struct kfifo *fifo, unsigned int len)
{
    (void *)fifo;(void *)len;
    return 0;
}
inline unsigned int kfifo_tail(struct kfifo *fifo);
{
    (void *)fifo;
    return 0;
}
#endif

#endif
