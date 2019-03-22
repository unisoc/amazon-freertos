#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <malloc.h>

extern char __HeapBase, __HeapLimit, HEAP_SIZE;
static int heapBytesRemaining = (int)&HEAP_SIZE;
caddr_t _sbrk(int incr)
{
	static char *currentHeapEnd = &__HeapBase;
	vTaskSuspendAll(); // Note: safe to use before FreeRTOS scheduler started
	char *previousHeapEnd = currentHeapEnd;
	if (currentHeapEnd + incr > &__HeapLimit) {
		#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	    {
	        extern void vApplicationMallocFailedHook( void );
	        vApplicationMallocFailedHook();
	    }
	    #elif 0
	        // If you want to alert debugger or halt...
	        while(1) { __asm("bkpt #0"); }; // Stop in GUI as if at a breakpoint (if debugging, otherwise loop forever)
	    #else
	        // If you prefer to believe your application will gracefully trap out-of-memory...
	        _impure_ptr->_errno = ENOMEM; // newlib's thread-specific errno
	        xTaskResumeAll();
	    #endif
	    return (char *)-1; // the malloc-family routine that called sbrk will return 0
    }
	currentHeapEnd += incr;
	heapBytesRemaining -= incr;
	xTaskResumeAll();
	return (char *) previousHeapEnd;
}

void __malloc_lock()     {       vTaskSuspendAll(); };
void __malloc_unlock()   { (void)xTaskResumeAll();  };

// newlib also requires implementing locks for the application's environment memory space,
// accessed by newlib's setenv() and getenv() functions.
// As these are trivial functions, momentarily suspend task switching (rather than semaphore).
// ToDo: Move __env_lock/unlock to a separate newlib helper file.
void __env_lock()    {       vTaskSuspendAll(); };
void __env_unlock()  { (void)xTaskResumeAll();  };

#if 0
/// /brief  Wrap malloc/malloc_r to help debug who requests memory and why.
/// Add to the linker command line: -Xlinker --wrap=malloc -Xlinker --wrap=_malloc_r
// Note: These functions are normally unused and stripped by linker.
void *__wrap_malloc(size_t nbytes) {
    extern void * __real_malloc(size_t nbytes);
    void *p = __real_malloc(nbytes); // Solely for debug breakpoint...
    return p;
};
void *__wrap__malloc_r(void *reent, size_t nbytes) {
    extern void * __real__malloc_r(size_t nbytes);
    void *p = __real__malloc_r(nbytes); // Solely for debug breakpoint...
    return p;
};
#endif

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

void _exit(int __status)
{
    printf("_exit\n");
    while (1) {
        ;
    }
}

int _kill(int pid, int sig)
{
    printf("_kill %d\n", sig);
    return -1;
}

pid_t _getpid(void)
{
    printf("_getpid\n");
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

size_t xPortGetFreeHeapSize( void )  {
    struct mallinfo mi = mallinfo();
    return mi.fordblks; + heapBytesRemaining;
}

// GetMinimumEverFree is not available in newlib's malloc implementation.
// So, no implementation provided: size_t xPortGetMinimumEverFreeHeapSize( void ) PRIVILEGED_FUNCTION;

//! No implementation needed, but stub provided in case application already calls vPortInitialiseBlocks
void vPortInitialiseBlocks( void )  {};
