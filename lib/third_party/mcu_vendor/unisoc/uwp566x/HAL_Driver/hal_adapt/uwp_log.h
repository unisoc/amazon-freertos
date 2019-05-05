#ifndef MBED_UWP_LOG_H
#define MBED_UWP_LOG_H

#include "hal_ramfunc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_LOG_ERR
//#define WIFI_LOG_WRN
//#define WIFI_LOG_DBG
//#define WIFI_LOG_INF
//#define WIFI_DUMP

#ifdef  WIFI_LOG_ERR
extern void vLoggingPrintf(const char *fmt, ... );
#define LOG_ERR(fmt, ...) do {\
        vLoggingPrintf("%s"fmt"\r\n", __func__, ##__VA_ARGS__);\
    }while(0)
#else
#define LOG_ERR(fmt, ...)
#endif

#ifdef  WIFI_LOG_WRN
#define LOG_WRN(fmt, ...) do {\
        vLoggingPrintf("%s"fmt"\r\n", __func__, ##__VA_ARGS__);\
    }while(0)
#else
#define LOG_WRN(fmt, ...)
#endif

#ifdef  WIFI_LOG_DBG
extern void vLoggingPrintf(const char *fmt, ... );
#define LOG_DBG(fmt, ...) do {\
        vLoggingPrintf("%s"fmt"\r\n", __func__, ##__VA_ARGS__);\
    }while(0)
#else
#define LOG_DBG(fmt, ...)
#endif

#ifdef  WIFI_LOG_INF
extern void vLoggingPrintf(const char *fmt, ... );
#define LOG_INF(fmt, ...) do {\
        vLoggingPrintf("%s"fmt"\r\n", __func__, ##__VA_ARGS__);\
    }while(0)
#else
#define LOG_INF(fmt, ...)
#endif

#ifdef  WIFI_DUMP
extern void vLoggingPrintf(const char *fmt, ... );
extern void vLoggingPrint( const char * pcMessage );
#define DUMP_DATA(buff, len) do {\
            u8_t *data = (u8_t *)buff;\
            for (int i=0;i<len;i++) {\
                    if(i%8 == 0)\
                        vLoggingPrint("  ");\
                    vLoggingPrintf("%02x ", data[i]);\
                    if((i+1)%16 == 0)\
                        vLoggingPrint("\r\n");\
            }\
            vLoggingPrintf("len:%d\r\n", len);\
    }while(0)
#else
#define DUMP_DATA(buff, len)
#endif

#define WIFI_ASSERT(expression,fmt,...) do {\
            if(!(expression)){\
                printk("fatal err:%s   ",__func__);\
                printk(fmt"\r\n",##__VA_ARGS__);\
                while(1);\
            }\
       } while(0)

extern void printk(const char *pcFarmat, ... );

#ifdef __cplusplus
}
#endif

#endif

