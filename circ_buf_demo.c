#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "circ_buf.h"
/* ref ipc-msg drivers/rpmsg/qcom_glink_smem.c */
#define dsb(opt)	asm volatile("dsb " #opt : : : "memory")
#define wmb()		dsb(st)

static void circ_rx_peak(struct circ_buf *circ,
			       void *data, unsigned int offset, size_t count)
{
	size_t len;
	u32 tail;

	tail = le32_to_cpu(circ->tail);
	tail += offset;
	if (tail >= circ->size)
		tail -= circ->size;

	len = min(count, circ->size - tail);
	if (len)
		memcpy(data, circ->buf + tail, len);

	if (len != count)
		memcpy(data + len, circ->buf, (count - len));
}
static void circ_rx_advance(struct circ_buf *circ,
				  size_t count)
{
	u32 tail;

	tail = le32_to_cpu(circ->tail);

	tail += count;
	if (tail >= circ->size)
		tail -= circ->size;

	circ->tail= cpu_to_le32(tail);
}
static unsigned int msg_tx_write_one(struct circ_buf *circ,
					    unsigned int head,
					    const void *data, size_t count)
{
	size_t len;

	len = min(count, circ->size - head);
	if (len)
		memcpy(circ->buf + head, data, len);

	if (len != count)
		memcpy(circ->buf, data + len, count - len);

	head += count;
	if (head >= circ->size)
		head -= circ->size;

	return head;
}

static void msg_tx_write(struct circ_buf *circ,
				const void *hdr, size_t hlen,
				const void *data, size_t dlen)
{
	unsigned int head;

	head = le32_to_cpu(circ->head);

	head = msg_tx_write_one(circ, head, hdr, hlen);
	head = msg_tx_write_one(circ, head, data, dlen);

	/* Ensure head is always aligned to 8 bytes */
	head = ALIGN(head, 8);
	if (head >= circ->size)
		head -= circ->size;

	/* Ensure ordering of fifo and head update */
	wmb();

	circ->head = cpu_to_le32(head);
}

static size_t msg_tx_avail(struct circ_buf *circ)
{
	int head;
	int tail;
	uint32_t avail = 0;

	head = le32_to_cpu(circ->head);
	tail = le32_to_cpu(circ->tail);

    avail = CIRC_SPACE(head, tail, circ->size);
    
	return avail;
}
static int test_circ_buf()
{
    unsigned long flags = 0;
    struct circ_buf *circ = &test_circ;
    struct hdr hdr = {0};
    int avail;

        for (;;) {
            spin_lock_irqsave(&rlock, flags);
            avail = CIRC_CNT(circ->head, circ->tail, circ->size);
            if (avail < sizeof(hdr)){               
                spin_unlock_irqrestore(&rlock, flags);
                break;
            }
            circ_rx_peak(circ, &hdr, 0, sizeof(hdr));
            if (hdr.msg_len > circ->size || hdr.msg_len < 0){
                printf("hdr.msg_len overflow\n");
            }
            if (avail < sizeof(hdr) + hdr.msg_len){
                spin_unlock_irqrestore(&rlock, flags);
                
                printf("invalid msg len\n");
                break;
            }
            circ_rx_peak(circ, message, sizeof(hdr), hdr.msg_len);
            circ_rx_advance(circ, ALIGN(sizeof(hdr) + hdr.msg_len , 8));
            spin_unlock_irqrestore(&rlock, flags);
            
        }
        
        spin_lock_irqsave(&wlock, flags);
        
        while (msg_tx_avail(circ) < hdr.msg_len + sizeof(hdr)) {
            if (!false) {
                lost_msg++;
                LogMsg_ISR(LOG_ERR, "logmsg lost_count[%d] msg[%s]\n", lost_msg, message);
                goto out;
            }
        }
            
        msg_tx_write(circ, &hdr, sizeof(hdr), message, hdr.msg_len);
out:
        //unlock
        spin_unlock_irqrestore(&wlock, flags);
}
