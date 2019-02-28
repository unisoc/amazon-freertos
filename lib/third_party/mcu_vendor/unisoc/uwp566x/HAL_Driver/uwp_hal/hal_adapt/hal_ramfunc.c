#include "hal_ramfunc.h"

void uwp_temp_printgf(const char *fmt, ...){
    return;
}

void vLoggingPrintf(char *fmt, ...){
    return;
}

void _irq_spurious(void *unused)
{
	for (;;)
		;
}

void smsg_irq_handler(void *unused){
    for(;;);
}

void wifi_int_irq_dpd_handler(void *unused){
    for(;;);
}

void wifi_int_irq_mac_handler(void *unused){
    for(;;);
}

void uart_uwp_isr(void *unused){
    for(;;);
}

void wifi_int_irq_com_tmr_handler(void *unused){
    for(;;);
}


