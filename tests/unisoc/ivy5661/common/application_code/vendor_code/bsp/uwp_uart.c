/*
 * uwp_uart.c
 *
 *  Created on: 2019Äê2ÔÂ27ÈÕ
 *      Author: xiangkai.gao
 */

#include "uwp_uart.h"
#include "FreeRTOSConfig.h"

extern serial_t stdio_uart;

void vStdioUARTOutput(char *DataToOutput){
    short sDataLength = 0;
    while( (DataToOutput[sDataLength] != '\0') && (sDataLength <= configLOGGING_MAX_MESSAGE_LENGTH) )
        serial_putc(&stdio_uart, DataToOutput[sDataLength++]);
}

#include <stdarg.h>
#include <string.h>

void printk(const char *pcFormat, ...){
    char LogForISRBuf[40];
    va_list args;
    va_start(args, pcFormat);
    vsnprintf( LogForISRBuf, 40, pcFormat, args );
    va_end(args);
    vStdioUARTOutput(LogForISRBuf);
}
