/*
 * unisoc_init.c
 *
 *  Created on: 2019Äê3ÔÂ5ÈÕ
 *      Author: xiangkai.gao
 */

#include "uwp_rtos_posix.h"

static k_thread_t prvUnisocInitTaskHandle = NULL;
static void prvUWPInitTask(void *pvParameter);

void vUnisocInitialize(void){
    if ( k_thread_create("uwpInitTask", prvUWPInitTask,
    		              NULL, NULL, 1024 * 2, 1, prvUnisocInitTaskHandle) != pdPASS){
    	configPRINT("Unisoc Init Task failed\r\n");
    	return;
    }
    configPRINT("Unisoc init ...\r\n");
}

/* this task shuold be deleted after uwp complete init */
extern int sipc_init(void);
extern int uwp_mcu_init(void);
static void prvUWPInitTask(void *pvParameter){
    if(sipc_init() != 0)
    	configPRINT("sipc init failed\r\n");
    if(uwp_mcu_init() != 0)
    	configPRINT("fw load failed\r\n");
    for(;;){
    	//configPRINT("unisoc init task\r\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
