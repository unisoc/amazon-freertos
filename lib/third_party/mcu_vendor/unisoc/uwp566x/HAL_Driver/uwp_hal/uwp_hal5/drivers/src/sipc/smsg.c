/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include "mbed_retarget.h"
#include "sipc.h"
#include "sipc_priv.h"
#include "hal_ipi.h"
#include "hal_sys.h"
#include "UWP_5661.h"
#include "hal_ramfunc.h"

//#define WIFI_LOG_DBG
#include "uwp_log.h"

#define K_FOREVER 0xFFFFFFFF

#define SMSG_STACK_SIZE		(2048)
//struct k_thread smsg_thread;
//K_THREAD_STACK_MEMBER(smsg_stack, SMSG_STACK_SIZE);
static struct smsg_ipc smsg_ipcs[SIPC_ID_NR];

#define SMSG_IRQ_TXBUF_ADDR  (0)
#define SMSG_IRQ_TXBUF_SIZE	 (0x200)
#define SMSG_IRQ_RXBUF_ADDR	 (SMSG_IRQ_TXBUF_ADDR + SMSG_IRQ_TXBUF_SIZE)
#define SMSG_IRQ_RXBUF_SIZE	 (0x100)

#define SMSG_PRIO_TXBUF_ADDR  (SMSG_IRQ_RXBUF_ADDR + SMSG_IRQ_RXBUF_SIZE)
#define SMSG_PRIO_TXBUF_SIZE  (0x200)
#define SMSG_PRIO_RXBUF_ADDR  (SMSG_PRIO_TXBUF_ADDR + SMSG_PRIO_TXBUF_SIZE)
#define SMSG_PRIO_RXBUF_SIZE  (0x200)

#define SMSG_TXBUF_ADDR	 (SMSG_PRIO_RXBUF_ADDR + SMSG_PRIO_RXBUF_SIZE)
#define SMSG_TXBUF_SIZE	 (SZ_1K)
#define SMSG_RXBUF_ADDR	 (SMSG_TXBUF_ADDR + SMSG_TXBUF_SIZE)
#define SMSG_RXBUF_SIZE	 (SZ_1K)

#define SMSG_RINGHDR  (SMSG_TXBUF_ADDR + SMSG_TXBUF_SIZE + SMSG_RXBUF_SIZE)

#define SMSG_IRQ_TXBUF_RDPTR	(SMSG_RINGHDR + 0)
#define SMSG_IRQ_TXBUF_WRPTR	(SMSG_RINGHDR + 4)
#define SMSG_IRQ_RXBUF_RDPTR	(SMSG_RINGHDR + 8)
#define SMSG_IRQ_RXBUF_WRPTR	(SMSG_RINGHDR + 12)

#define SMSG_PRIO_TXBUF_RDPTR	(SMSG_RINGHDR + 16)
#define SMSG_PRIO_TXBUF_WRPTR	(SMSG_RINGHDR + 20)
#define SMSG_PRIO_RXBUF_RDPTR	(SMSG_RINGHDR + 24)
#define SMSG_PRIO_RXBUF_WRPTR	(SMSG_RINGHDR + 28)

#define SMSG_TXBUF_RDPTR	(SMSG_RINGHDR + 32)
#define SMSG_TXBUF_WRPTR	(SMSG_RINGHDR + 36)
#define SMSG_RXBUF_RDPTR	(SMSG_RINGHDR + 40)
#define SMSG_RXBUF_WRPTR	(SMSG_RINGHDR + 44)

// TODO:need porting
void wakeup_smsg_task_all(void *sem){
    uwp_temp_printf("%s\r\n",__func__);
    return;
}

/*
static inline void uwp_ipi_irq_trigger(void){
    return;
}

typedef void (*uwp_ipi_callback_t) (void *data);
void uwp_ipi_set_callback(uwp_ipi_callback_t cb, void *arg){
    return;
}
*/

void sipc_init_smsg_queue_buf(struct smsg_queue_buf *buf,
		u32_t size, u32_t addr, u32_t rdptr, u32_t wrptr)
{
	buf->size = size / sizeof(struct smsg);
	buf->addr = addr;
	buf->rdptr = rdptr;
	buf->wrptr = wrptr;
}

static struct smsg_ipc *smsg_set_addr(struct smsg_ipc *ipc, u32_t base)
{
	sipc_init_smsg_queue_buf(&ipc->queue[QUEUE_PRIO_IRQ].tx_buf,
			SMSG_IRQ_TXBUF_SIZE,
			base + SMSG_IRQ_TXBUF_ADDR,
			base + SMSG_IRQ_TXBUF_RDPTR,
			base + SMSG_IRQ_TXBUF_WRPTR);

	sipc_init_smsg_queue_buf(&ipc->queue[QUEUE_PRIO_IRQ].rx_buf,
			SMSG_IRQ_RXBUF_SIZE,
			base + SMSG_IRQ_RXBUF_ADDR,
			base + SMSG_IRQ_RXBUF_RDPTR,
			base + SMSG_IRQ_RXBUF_WRPTR);

/*prio msg deq init*/
	sipc_init_smsg_queue_buf(&ipc->queue[QUEUE_PRIO_HIGH].tx_buf,
			SMSG_PRIO_TXBUF_SIZE,
			base + SMSG_PRIO_TXBUF_ADDR,
			base + SMSG_PRIO_TXBUF_RDPTR,
			base + SMSG_PRIO_TXBUF_WRPTR);

	sipc_init_smsg_queue_buf(&ipc->queue[QUEUE_PRIO_HIGH].rx_buf,
			SMSG_PRIO_RXBUF_SIZE,
			base + SMSG_PRIO_RXBUF_ADDR,
			base + SMSG_PRIO_RXBUF_RDPTR,
			base + SMSG_PRIO_RXBUF_WRPTR);

/*normal msg deq init*/
	sipc_init_smsg_queue_buf(&ipc->queue[QUEUE_PRIO_NORMAL].tx_buf,
			SMSG_TXBUF_SIZE,
			base + SMSG_TXBUF_ADDR,
			base + SMSG_TXBUF_RDPTR,
			base + SMSG_TXBUF_WRPTR);

	sipc_init_smsg_queue_buf(&ipc->queue[QUEUE_PRIO_NORMAL].rx_buf,
			SMSG_RXBUF_SIZE,
			base + SMSG_RXBUF_ADDR,
			base + SMSG_RXBUF_RDPTR,
			base + SMSG_RXBUF_WRPTR);

return ipc;
}

void smsg_clear_queue_buf(struct smsg_queue *queue)
{
	sci_write32(queue->tx_buf.rdptr, 0);
	sci_write32(queue->tx_buf.wrptr, 0);
	sci_write32(queue->rx_buf.rdptr, 0);
	sci_write32(queue->rx_buf.wrptr, 0);
}

void smsg_clear_queue(struct smsg_ipc *ipc, int prio)
{
	struct smsg_queue *queue;

	if (prio >= QUEUE_PRIO_MAX) {
		LOG_ERR("Invalid queue priority %d.\n", prio);
		return;
	}

	queue = &ipc->queue[prio];

	smsg_clear_queue_buf(queue);
}

extern void sblock_process(struct smsg *msg);
void smsg_msg_dispatch_thread(const char *arg)
{
	int prio;
	struct smsg_ipc *ipc = &smsg_ipcs[0];
	struct smsg *msg;
	struct smsg recv_smsg;
	struct smsg_channel *ch;
	uintptr_t rxpos;
	struct smsg_queue_buf *rx_buf;

	while (1) {

		k_sem_acquire(ipc->irq_sem, K_FOREVER);

		for (prio = QUEUE_PRIO_IRQ; prio < QUEUE_PRIO_MAX; prio++) {
			rx_buf = &(ipc->queue[prio].rx_buf);
			if (sys_read32(rx_buf->wrptr) !=
					sys_read32(rx_buf->rdptr))
				break;
		}

		while (sys_read32(rx_buf->wrptr) != sys_read32(rx_buf->rdptr)) {
			rxpos = (sys_read32(rx_buf->rdptr) & (rx_buf->size - 1))
				* sizeof(struct smsg) + rx_buf->addr;

			msg = (struct smsg *)rxpos;

			memcpy(&recv_smsg, msg, sizeof(struct smsg));
			sys_write32(sys_read32(rx_buf->rdptr) + 1,
					rx_buf->rdptr);

			LOG_DBG("read smsg: channel=%d, type=%d, flag=0x%04x, value=0x%08x %d %d",
					msg->channel, msg->type, msg->flag,
					msg->value, sys_read32(rx_buf->wrptr),
					sys_read32(rx_buf->rdptr));

			if (recv_smsg.channel >= SMSG_CH_NR
					|| recv_smsg.type >= SMSG_TYPE_NR
					|| SMSG_TYPE_DIE == recv_smsg.type) {
				LOG_ERR("invalid smsg: channel=%d, type=%d",
					recv_smsg.channel, recv_smsg.type);
				continue;
			}
			if (recv_smsg.type == SMSG_TYPE_WIFI_IRQ) {
				if (recv_smsg.flag == SMSG_WIFI_IRQ_OPEN) {
					sprd_wifi_irq_enable_num(
						recv_smsg.value);
					LOG_DBG("wifi irq %d open\n",
					recv_smsg.value);
				} else if (recv_smsg.flag ==
						SMSG_WIFI_IRQ_CLOSE) {
					sprd_wifi_irq_disable_num(
						recv_smsg.value);
					LOG_DBG("wifi irq %d close\n",
					recv_smsg.value);
				}
				continue;
			}
			ch = &ipc->channels[recv_smsg.channel];

			sblock_process(&recv_smsg);
		}
	}

}

int smsg_ipc_destroy(u8_t dst)
{
	struct smsg_ipc *ipc = &smsg_ipcs[dst];

	k_thread_terminate(ipc->pid);

	return 0;
}

int smsg_ch_open(u8_t dst, u8_t channel, int prio, int timeout)
{
	struct smsg_ipc *ipc = &smsg_ipcs[dst];
	struct smsg_channel *ch;
	struct smsg mopen;

	int ret = 0;

	LOG_DBG("open dst %d channel %d", dst, channel);
	if (!ipc) {
		LOG_ERR("get ipc %d failed.\n", dst);
		return -ENODEV;
	}

	ch = &ipc->channels[channel];
	if (ch->state != CHAN_STATE_UNUSED) {
		LOG_ERR("ipc channel %d had been opened.\n", channel);
		return -ENODEV;
	}

	ch->rxsem = k_sem_create(1, 0);
	ch->rxlock = k_mutex_create();

    if(ch->txlock == NULL)
	    ch->txlock = k_mutex_create();
    else
		LOG_ERR("channel:%d txlock has created",channel);

	smsg_set(&mopen, channel, SMSG_TYPE_OPEN, SMSG_OPEN_MAGIC, 0);
	ret = smsg_send(dst, prio, &mopen, timeout);
	if (ret != 0) {
		LOG_WRN("smsg send error, errno %d!\n", ret);
		ch->state = CHAN_STATE_UNUSED;

		return ret;
	}
	LOG_DBG("send open success");

	ch->state = CHAN_STATE_OPENED;
	LOG_DBG("open channel success");

	return 0;
}

int smsg_ch_close(u8_t dst, u8_t channel, int prio, int timeout)
{
	struct smsg_ipc *ipc = &smsg_ipcs[dst];
	struct smsg_channel *ch = &ipc->channels[channel];
	struct smsg mclose;

	smsg_set(&mclose, channel, SMSG_TYPE_CLOSE, SMSG_CLOSE_MAGIC, 0);
	smsg_send(dst, prio, &mclose, timeout);

	ch->state = CHAN_STATE_FREE;

	/* finally, update the channel state*/
	ch->state = CHAN_STATE_UNUSED;

	return 0;
}

int smsg_send_irq(u8_t dst, struct smsg *msg)
{
	struct smsg_ipc *ipc = &smsg_ipcs[dst];
	struct smsg_queue_buf *tx_buf;
	uintptr_t txpos;
	int ret = 0;

	if (!ipc) {
		return -ENODEV;
	}

	tx_buf = &ipc->queue[QUEUE_PRIO_IRQ].tx_buf;

	if (sys_read32(tx_buf->wrptr) - sys_read32(tx_buf->rdptr)
		>= tx_buf->size) {
		LOG_DBG("smsg irq txbuf is full! %d %d %d\n",
			sys_read32(tx_buf->wrptr),
			sys_read32(tx_buf->rdptr), msg->value);
		ret = -EBUSY;
		goto send_failed;
	}
	/* calc txpos and write smsg */
	txpos = (sys_read32(tx_buf->wrptr) & (tx_buf->size - 1)) *
		sizeof(struct smsg) + tx_buf->addr;
	memcpy((void *)txpos, msg, sizeof(struct smsg));
	/*
	 *  ipc_info("write smsg: wrptr=%u, rdptr=%u, txpos=0x%lx %d\n",
	 *	sys_read32(ipc->queue_irq.tx_buf.wrptr),
	 *	sys_read32(ipc->queue_irq.tx_buf.rdptr), txpos,msg->value);
	 */

	/* update wrptr */
	sys_write32(sys_read32(tx_buf->wrptr) + 1, tx_buf->wrptr);

    uwp_ipi_trigger(IPI_CORE_BTWF, IPI_TYPE_IRQ0);

send_failed:
	return ret;
}

int smsg_send(u8_t dst, u8_t prio, struct smsg *msg, int timeout)
{
	struct smsg_channel *ch;
	struct smsg_queue_buf *tx_buf;
	struct smsg_ipc *ipc = &smsg_ipcs[dst];
	u32_t txpos;
	int ret = 0;

	ch = &ipc->channels[msg->channel];

	if (ch->state != CHAN_STATE_OPENED &&
			msg->type != SMSG_TYPE_OPEN &&
			msg->type != SMSG_TYPE_CLOSE &&
			msg->type != SMSG_TYPE_DONE &&
			msg->channel != SMSG_CH_IRQ_DIS) {
		LOG_WRN("channel %d not opened!\n", msg->channel);
		return -EINVAL;
	}

	if (prio >= QUEUE_PRIO_MAX) {
		LOG_ERR("Invalid queue priority %d.\n", prio);
		return -EINVAL;
	}

	tx_buf = &(ipc->queue[prio].tx_buf);

	LOG_DBG("%d smsg txbuf wr %d rd %d!", prio,
			sys_read32(tx_buf->wrptr),
			sys_read32(tx_buf->rdptr));

	if ((int)(sys_read32(tx_buf->wrptr)	- sys_read32(tx_buf->rdptr))
			>= tx_buf->size) {
		LOG_WRN("smsg txbuf is full! %d %d\n",
				sys_read32(tx_buf->wrptr),
				sys_read32(tx_buf->rdptr));

		return -EBUSY;
	}

    /* sometimes response to CP however the channel is not created at AP */
	if(ch->txlock == NULL){
		//LOG_ERR("channel:%d create txlock",msg->channel);
        ch->txlock = k_mutex_create();
	}

	k_mutex_lock(ch->txlock, K_FOREVER);
	/* calc txpos and write smsg */
	txpos = (sys_read32(tx_buf->wrptr) & (tx_buf->size - 1)) *
		sizeof(struct smsg) + tx_buf->addr;
	memcpy((void *)txpos, msg, sizeof(struct smsg));

	/*
	 *  LOG_DBG("write smsg: wrptr=%u, rdptr=%u, txpos=0x%x\n",
	 *		sys_read32(tx_buf->wrptr),
	 *		sys_read32(tx_buf->rdptr), txpos);
	 */

	/* update wrptr */
	sys_write32(sys_read32(tx_buf->wrptr) + 1, tx_buf->wrptr);
	k_mutex_unlock(ch->txlock);

    uwp_ipi_trigger(IPI_CORE_BTWF, IPI_TYPE_IRQ0);

	return ret;
}

static void smsg_irq_handler(void *arg)
{
    struct smsg_ipc *ipc = &smsg_ipcs[0];

    NVIC_DisableIRQ(GNSS2BTWIFI_IPI_IRQn);

    LOG_DBG("\r\nipi\r\n");

    uwp_ipi_clear_remote(IPI_CORE_BTWF, IPI_TYPE_IRQ0);

    k_sem_release(ipc->irq_sem);

    NVIC_EnableIRQ(GNSS2BTWIFI_IPI_IRQn);
}
#if 0
int smsg_init(u32_t dst, u32_t smsg_base)
{
	struct smsg_ipc *ipc = &smsg_ipcs[dst];

	LOG_INF("smsg init dst %d addr 0x%x.", dst, smsg_base);

	smsg_set_addr(ipc, smsg_base);

	smsg_clear_queue(ipc, QUEUE_PRIO_NORMAL);
	smsg_clear_queue(ipc, QUEUE_PRIO_HIGH);
	smsg_clear_queue(ipc, QUEUE_PRIO_IRQ);

	//uwp_ipi_set_callback(smsg_irq_handler, (void *)ipc);

	ipc->irq_sem = k_sem_create(1, 0);

    ipc->pid = k_thread_create("smsg_thread",smsg_msg_dispatch_thread,NULL,NULL,SMSG_STACK_SIZE,osPriorityNormal);
    if(ipc->pid == NULL)
        LOG_ERR("smsg thread create failed");
#if 1
    NVIC_DisableIRQ(GNSS2BTWIFI_IPI_IRQn);
    uwp_sys_enable(BIT(APB_EB_IPI));
    uwp_sys_reset(BIT(APB_EB_IPI));
	// TODO: isr priority
    NVIC_SetPriority(GNSS2BTWIFI_IPI_IRQn,0x1FUL);
    NVIC_SetVector(GNSS2BTWIFI_IPI_IRQn,(uint32_t)smsg_irq_handler);
    NVIC_EnableIRQ(GNSS2BTWIFI_IPI_IRQn);
#endif

	return (ipc->pid == NULL);
}
#endif
