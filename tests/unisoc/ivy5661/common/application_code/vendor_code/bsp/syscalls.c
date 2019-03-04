#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <malloc.h>

extern uint32_t __sbrk_start;
extern uint32_t __krbs_start;

caddr_t _sbrk(int incr)
{
    static uint32_t heap_ind = (uint32_t)(&__sbrk_start);
    uint32_t heap_ind_pre = heap_ind;
    uint32_t heap_ind_new = (heap_ind_pre + incr + 0x07) & ~0x07;
    if (heap_ind_new > (uint32_t)(&__krbs_start)) {
        errno = ENOMEM;
        return (caddr_t)(-1);
    }
    heap_ind = heap_ind_new;
    return (caddr_t) heap_ind_pre;
}

#include "uwp_uart.h"
extern serial_t stdio_uart;
int _write(int file, char *ptr, int len)
{
	int i;
	if(file < 3){
		serial_init(&stdio_uart, NC, NC);
		for(i = 0; i < len; i++)
			serial_putc(&stdio_uart, ptr[i]);
	}
	return len;
}

int _read (int file, char *ptr, int len)
{
	/* The I/O library uses an internal buffer */
	/* It asks for 1024 characters even if only getc() is used. */
	/* If we use a for(;;) loop on the number of characters requested, */
	/* the user is forced to enter the exact number requested, even if only one is needed. */
	/* So here we return only 1 character even if requested length is > 1 */
	//*ptr = __io_getchar();

	return 1;
}

int _close(int file)
{
	return -1;
}


int _fstat(int file, struct stat *st)
{
	//st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}


// ================================================================================================
// Implement FreeRTOS's memory API using newlib-provided malloc family.
// ================================================================================================

void *pvPortMalloc( size_t xSize )  {
    void *p = malloc(xSize);
    return p;
}
void vPortFree( void *pv )  {
    free(pv);
};
#if 0
size_t xPortGetFreeHeapSize( void )  {
    struct mallinfo mi = mallinfo();
    return mi.fordblks; + heapBytesRemaining;
}
#endif
// GetMinimumEverFree is not available in newlib's malloc implementation.
// So, no implementation provided: size_t xPortGetMinimumEverFreeHeapSize( void ) PRIVILEGED_FUNCTION;

//! No implementation needed, but stub provided in case application already calls vPortInitialiseBlocks
void vPortInitialiseBlocks( void )  {};
