#ifndef _UWP_SYS_WRAPPER_H
#define _UWP_SYS_WRAPPER_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#define BIT4     (1<<4)
#define BIT3     (1<<3)
#define BIT2     (1<<2)
#define BIT1     (1<<2)
#define BIT0     (1)

#define UWP_DONE        1
#define UWP_OK          0
#define UWP_FAIL        -1
#ifndef osOK
#define osOK 0
#endif

#define uwpPriorityNormal 1

/*list*/
struct _snode{
    struct _snode *next;
};
typedef struct _snode sys_snode_t;
struct _slist{
    sys_snode_t *head;
    sys_snode_t *tail;
};
typedef struct _slist sys_slist_t;

static inline void sys_slist_init(sys_slist_t *list){
    list->head = NULL;
    list->tail = NULL;
}

struct list_head{
    struct list_head *next, *prev;
};

static inline void __INIT_LIST_HEAD(struct list_head *list){
    list->next = list;
    list->prev = list;
}

static inline void __LIST_ADD(struct list_head *newlist,
            struct list_head *prev, struct list_head *next){
    next->prev = newlist;
    newlist->next = next;
    prev->next = newlist;
    newlist->prev = prev;
}

#define LIST_FIND_ENTRY(ptr, container, member)  \
    ((char *)ptr - (char *)&(((container *)0)->member))

void list_add_tail(struct list_head *newlist, struct list_head *list);
int list_del_node(struct list_head *node, struct list_head *list);
/*list end*/

#define UWP_SLEEP(t) \
	vTaskDelay(pdMS_TO_TICKS(t))


#ifndef UWP_DELAY_FOREVER
#define UWP_DELAY_FOREVER    portMAX_DELAY
#endif // OS_DELAY_FOREVER

#ifndef UWP_NO_WAIT
#define UWP_NO_WAIT    0
#endif // OS_NO_WAIT
typedef SemaphoreHandle_t OS_MutexHandle_t;

#ifndef UWP_MUTEX_INIT
#define UWP_MUTEX_INIT(lock) \
    do { \
        lock = xSemaphoreCreateMutex(); \
        configASSERT(lock != NULL); \
    } while(0);
#endif // OS_MUTEX_INIT
/*xSemaphoreTake(),success,return pdTRUE=1£¬;fail,return pdFALSE=0*/
#ifndef UWP_MUTEX_LOCK
#define UWP_MUTEX_LOCK(lock, timeout)    \
    xSemaphoreTake((lock), (timeout))
#endif // OS_MUTEX_UNLOCK

#ifndef UWP_MUTEX_UNLOCK
#define UWP_MUTEX_UNLOCK(lock)    \
    xSemaphoreGive((lock))
#endif // OS_MUTEX_UNLOCK

typedef QueueHandle_t OS_SemaphoreHandle_t;

#ifndef UWP_SEM_INIT
#define UWP_SEM_INIT(sem, max, init) \
    do { \
        sem = xSemaphoreCreateCounting((max), (init)); \
    } while(0);
#endif // OS_SEM_INIT

#ifndef UWP_SEM_TAKE_FROM_ISR
#define UWP_SEN_TAKE_FROM_ISR(sem, timeout)    \
    xSemaphoreTakeFromISR((sem), (timeout))
#endif // OS_SEM_TAKE_FROM_IRS

#ifndef UWP_SEM_GIVE_FROM_ISR
#define UWP_SEM_GIVE_FROM_ISR(sem, pxHigherPriorityTaskWoken)    \
    xSemaphoreGiveFromISR((sem), (pxHigherPriorityTaskWoken))
#endif // OS_SEM_GIVE_FROM_ISR

#ifndef UWPOS_SEM_TAKE
#define UWP_SEM_TAKE(sem, timeout)    \
    xSemaphoreTake((sem), (timeout))
#endif // OS_SEM_TAKE

#ifndef UWP_SEM_GIVE
#define UWP_SEM_GIVE(sem)    \
    xSemaphoreGive((sem))
#endif // OS_SEM_GIVE

typedef QueueHandle_t OS_Queue_t;

#ifndef UWP_QUEUE_INIT
#define UWPOS_QUEUE_INIT(queue, queue_len, item_size) \
		do { \
			queue = xQueueCreate(queue_len, item_size); \
		} while(0);
#endif // OS_SEM_INIT

#ifndef UWP_QUEUE_DELETE
#define UWP_QUEUE_DELETE(queue) \
		do { \
			vQueueDelete(queue); \
		} while(0);
#endif // OS_SEM_DELETE

#ifndef UWP_QUEUE_SEND
#define UWP_QUEUE_SEND(queue, item, timeout)    \
		xQueueSend((queue), (item), timeout)
#endif // OS_SEND_SEND

#ifndef UWP_QUEUE_SEND_FROM_ISR
#define UWP_QUEUE_SEND_FROM_ISR(queue, item, timeout)    \
		xQueueSendFromISR((queue), (item), timeout)
#endif // OS_SEND_SEND_FROM_ISR

#ifndef UWP_QUEUE_RECV_FROM_ISR
#define UWP_QUEUE_RECV_FROM_ISR(queue, item, timeout)    \
		xQueueReceiveFromISR((queue), (item), (timeout))
#endif // OS_QUEUE_RECV_FROM_ISR

#ifndef UWP_QUEUE_RECV
#define UWP_QUEUE_RECV(queue, item, timeout)    \
		xQueueReceive((queue), (item), (timeout))
#endif // OS_QUEUE_RECV

#ifndef UWP_QUEUE_MSG_WAITING
#define UWP_QUEUE_MSG_WAITING(queue)    \
		uxQueueMessagesWaiting((queue))
#endif // OS_QUEUE_MSG_WAITING


#ifndef UWPxEventGroupCreate
#define UWPxEventGroupCreate()		\
		xEventGroupCreate()
#endif

#ifndef UWPEventGroupWaitBits
#define UWPEventGroupWaitBits(sem, bit, clear, all, timeout)		\
		xEventGroupWaitBits(sem, bit, clear, all, timeout)
#endif

#ifndef UWPEventGroupClearBits
#define UWPEventGroupClearBits(sem, bit)		\
	    xEventGroupClearBits(sem, bit)
#endif

#ifndef	UWPEventGroupSetBits
#define UWPEventGroupSetBits(sem, bit)		\
		xEventGroupSetBits(sem, bit)
#endif


#ifndef ARG_UNUSED
#define ARG_UNUSED(x) (void)(x)
#endif // ARG_UNUSED

#ifndef UWP_task_get_current_task
#define UWP_task_get_current_task() \
    (void*)xTaskGetCurrentTaskHandle()
#endif // os_task_get_current_task

#ifndef UWP_MEM_ALLOC
#define UWP_MEM_ALLOC(size) pvPortMalloc(size)
#endif // HEAP_MEM_ALLOC

#ifndef UWP_MEM_FREE
#define UWP_MEM_FREE(addr) vPortFree(addr)
#endif // HEAP_MEM_FREE


#ifndef UWPTaskCreate
#define UWPTaskCreate(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask)		\
		xTaskCreate(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask)
#endif

#endif

