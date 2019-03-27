#include "hal_ramfunc.h"
#include "FreeRTOSConfig.h"

void _irq_spurious(void *unused)
{
    for (;;);
}

void default_smsg_irq_handler(void *unused){
    configPRINT_STRING("FreeRTOS unexpected handler: default_smsg_irq_handler\r\n");
    for(;;);
}

void wifi_int_irq_dpd_handler(void *unused){
    configPRINT_STRING("FreeRTOS unexpected handler: wifi_int_irq_dpd_handler\r\n");
    for(;;);
}

void wifi_int_irq_mac_handler(void *unused){
    configPRINT_STRING("FreeRTOS unexpected handler: wifi_int_irq_mac_handler\r\n");
    for(;;);
}

void uart_uwp_isr(void *unused){
    configPRINT_STRING("FreeRTOS unexpected handler: uart_uwp_isr\r\n");
    for(;;);
}

void wifi_int_irq_com_tmr_handler(void *unused){
    configPRINT_STRING("FreeRTOS unexpected handler: wifi_int_irq_com_tmr_handler\r\n");
    for(;;);
}


