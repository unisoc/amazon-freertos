/*
 * uwp_rtos_posix.c
 *
 *  Created on: 2019Äê3ÔÂ4ÈÕ
 *      Author: xiangkai.gao
 */

#include <stdio.h>

#include "uwp_rtos_posix.h"

#if (configUSE_UWP_ASSISTANT == 1)

char *pcLastTaskName;
char *pcCurrentTaskName;

void k_get_current_task(void){
    vLoggingPrintf("Current Task %s\r\n", pcCurrentTaskName);
}

void k_get_last_task(void){
    vLoggingPrintf("Last Task %s\r\n", pcLastTaskName);
}

#if (configUSE_TRACE_FACILITY == 1) && (INCLUDE_uxTaskGetStackHighWaterMark == 1)

static char TaskStateEntry[][4] = {
        "Run", "Rdy", "Blk", "Spd", "Del", "Inv"
};
/* cannot be called from ISR */
void k_task_list(void){

    UBaseType_t  ulCurrentTasks;
    ulCurrentTasks = uxTaskGetNumberOfTasks();
    if(ulCurrentTasks > 0){
        TaskStatus_t *xTaskStatusArray = pvPortMalloc(ulCurrentTasks * sizeof(TaskStatus_t));
        if(xTaskStatusArray == NULL){
            vLoggingPrintf("malloc failed\r\n");
            return;
        }
        ulCurrentTasks = uxTaskGetSystemState( xTaskStatusArray, ulCurrentTasks, NULL );
        vLoggingPrintf("%-20s\t%-10s\t%-10s\t%-10s\t%-10s\r\n","task", "state", "ori_pro", "cur_pro", "stack");

        for(UBaseType_t ulTask = 0; ulTask < ulCurrentTasks; ulTask++){
            vLoggingPrintf("%-20s\t%-10s\t%-10d\t%-10d\t%-10d\r\n",xTaskStatusArray[ulTask].pcTaskName,
                    TaskStateEntry[xTaskStatusArray[ulTask].eCurrentState], xTaskStatusArray[ulTask].uxBasePriority,
                    xTaskStatusArray[ulTask].uxCurrentPriority, xTaskStatusArray[ulTask].usStackHighWaterMark);
        }
    }
}

#else

void k_task_list(void){
    vLoggingPrint("please open in FreeRTOSConfig.h");
}

#endif

#endif
