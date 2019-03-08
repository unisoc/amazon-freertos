#ifndef UWP_RTOS_POSIX_H__
#define UWP_RTOS_POSIX_H__

#ifdef __cplusplus
extern "C"{
#endif

#define FREERTOS_AND_INTERRUPT_DEFER
//#define UWP_USE_RTOS_POSIX

#ifndef UWP_USE_RTOS_POSIX

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "hal_type.h"

#define K_FOREVER portMAX_DELAY

#define RTOS_RETURN_VALUE_SUCCESS  1
#define RTOS_RETURN_VALUE_FAILED   0

#define k_sleep( X ) vTaskDelay( pdMS_TO_TICKS(X) )

#define k_queue_init( QueueHandle, ulQueueLength, ulQueueItemSize) \
	do { \
	    QueueHandle = xQueueCreate( ulQueueLength, ulQueueItemSize ); \
	} while(0)

#define k_queue_send( QueueHandle, pvQueueItem, ulTimeout) \
	xQueueSend( QueueHandle, pvQueueItem, pdMS_TO_TICKS(ulTimeout) )

#define k_queue_send_from_isr( QueueHandle, pvQueueItem, ulTimeout) \
	xQueueSendFromISR( QueueHandle, pvQueueItem, pdMS_TO_TICKS(ulTimeout) )

#define k_queue_receive( QueueHandle, pvQueueItem, ulTimeout ) \
	xQueueReceive( QueueHandle, pvQueueItem, pdMS_TO_TICKS(ulTimeout) )

#define k_queue_receive_from_isr( QueueHandle, pvQueueItem, ulTimeout) \
	xQueueReceiveFromISR( QueueHandle, pvQueueItem, pdMS_TO_TICKS(ulTimeout) )


#define k_sem_init( SemHandle, ulMax, ulInit) \
    do { \
    	SemHandle = xSemaphoreCreateCounting( (ulMax), (ulInit) ); \
    } while(0);

#define k_sem_take( SemHandle, ulTimeout) \
    xSemaphoreTake( (SemHandle), pdMS_TO_TICKS(ulTimeout) )

#define k_sem_give( SemHandle ) \
    xSemaphoreGive( (SemHandle))

#define k_sem_give_from_isr( SemHandle, pxHigherPriorityTaskWoken) \
    xSemaphoreGiveFromISR( (SemHandle), (pxHigherPriorityTaskWoken) )

#define k_sem_getvalue( SemHandle ) \
	uxSemaphoreGetCount( (SemHandle) )


#define k_mutex_init( MutexHandle ) \
	do { \
	    MutexHandle = xSemaphoreCreateMutex(); \
	} while(0)

#define k_mutex_lock k_sem_take

#define k_mutex_unlock k_sem_give


#define k_thread_create( pcTaskName, pvTaskCode, pvParameter, pvTaskStack, usStackSize, uxPriority, TaskHandle ) \
    xTaskCreate( pvTaskCode, pcTaskName, usStackSize, pvParameter, uxPriority, &(TaskHandle) )

#else
#error "no support posix yet"

#endif

#ifdef __cplusplus
}
#endif

#endif

