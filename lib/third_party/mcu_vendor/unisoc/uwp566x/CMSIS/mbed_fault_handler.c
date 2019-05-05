/* mbed Microcontroller Library
 * Copyright (c) 2006-2018 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>
#include "UWP_5661.h"
#include "mbed_fault_handler.h"

//Functions Prototypes
static void print_context_info(void);
extern void printk(const char *pcFormat, ... );
#define mbed_error_printf printk

//Global for populating the context in exception handler
mbed_fault_context_t mbed_fault_context;

//This is a handler function called from Fault handler to print the error information out.
//This runs in fault context and uses special functions(defined in mbed_rtx_fault_handler.c) to print the information without using C-lib support.
void mbed_fault_handler (uint32_t fault_type, void *mbed_fault_context_in)
{
    printk("\n++ FreeRTOS Fault Handler ++\n\nFaultType: ");
        
    switch( fault_type ) {
      case MEMMANAGE_FAULT_EXCEPTION: 
    	mbed_error_printf("MemManageFault\r\n");
        //faultStatus = MBED_ERROR_MEMMANAGE_EXCEPTION;
        break;
      
      case BUS_FAULT_EXCEPTION: 
    	mbed_error_printf("BusFault\r\n");
        //faultStatus = MBED_ERROR_BUSFAULT_EXCEPTION;
        break;
      
      case USAGE_FAULT_EXCEPTION: 
    	mbed_error_printf("UsageFault\r\n");
        //faultStatus = MBED_ERROR_USAGEFAULT_EXCEPTION;
        break;
      
      //There is no way we can hit this code without getting an exception, so we have the default treated like hardfault
      case HARD_FAULT_EXCEPTION: 
      default:    
    	mbed_error_printf("HardFault\r\n");
        //faultStatus = MBED_ERROR_HARDFAULT_EXCEPTION;
        break;
    }
    mbed_error_printf("Context:");
    print_context_info();
    
    mbed_error_printf("\n\n-- FreeRTOS Fault Handler --\n\n");
    for(;;);
    //Now call mbed_error, to log the error and halt the system
    //mbed_error( faultStatus, "Fault exception", mbed_fault_context.PC_reg, NULL, 0 );
    
}

static void print_context_info(void)
{
    //Context Regs
    for(int i=0;i<13;i++) {
        mbed_error_printf("\r\nR%-4d: %08" PRIX32, i, ((uint32_t *)&mbed_fault_context)[i]);
    }
        
    mbed_error_printf("\r\nSP   : %08" PRIX32
                      "\r\nLR   : %08" PRIX32
                      "\r\nPC   : %08" PRIX32
                      "\r\nxPSR : %08" PRIX32
                      "\r\nPSP  : %08" PRIX32
                      "\r\nMSP  : %08" PRIX32, mbed_fault_context.SP_reg, mbed_fault_context.LR_reg, mbed_fault_context.PC_reg,
                                     mbed_fault_context.xPSR, mbed_fault_context.PSP, mbed_fault_context.MSP );
                       
    //Capture CPUID to get core/cpu info
    mbed_error_printf("\r\nCPUID: %08" PRIX32, SCB->CPUID);
    
#if !defined(TARGET_M0) && !defined(TARGET_M0P)
    //Capture fault information registers to infer the cause of exception
    mbed_error_printf("\r\nHFSR : %08" PRIX32
                    "\r\nMMFSR: %08" PRIX32
                    "\r\nBFSR : %08" PRIX32
                    "\r\nUFSR : %08" PRIX32
                    "\r\nDFSR : %08" PRIX32
                    "\\rnAFSR : %08" PRIX32  ////Split/Capture CFSR into MMFSR, BFSR, UFSR
                    ,SCB->HFSR, (0xFF & SCB->CFSR), ((0xFF00 & SCB->CFSR) >> 8), ((0xFFFF0000 & SCB->CFSR) >> 16), SCB->DFSR, SCB->AFSR ); 
    
    //Print MMFAR only if its valid as indicated by MMFSR
    if ((0xFF & SCB->CFSR) & 0x80) {
        mbed_error_printf("\r\nMMFAR: %08" PRIX32, SCB->MMFAR);
    }
    //Print BFAR only if its valid as indicated by BFSR
    if (((0xFF00 & SCB->CFSR) >> 8) & 0x80) {
        mbed_error_printf("\r\nBFAR : %08" PRIX32, SCB->BFAR);
    }
#endif
    
    //Print Mode
    if (mbed_fault_context.EXC_RETURN & 0x8) {
        mbed_error_printf("\r\nMode : Thread");
        //Print Priv level in Thread mode - We capture CONTROL reg which reflects the privilege.
        //Note that the CONTROL register captured still reflects the privilege status of the 
        //thread mode eventhough we are in Handler mode by the time we capture it.
        if(mbed_fault_context.CONTROL & 0x1) {
            mbed_error_printf("\r\nPriv : User");
        } else {
            mbed_error_printf("\r\nPriv : Privileged");
        }        
    } else {
        mbed_error_printf("\r\nMode : Handler");
        mbed_error_printf("\r\nPriv : Privileged");
    }
    //Print Return Stack
    if (mbed_fault_context.EXC_RETURN & 0x4) {
        mbed_error_printf("\r\nStack: PSP");
    } else {
        mbed_error_printf("\r\nStack: MSP");
    }
}

void NMI_Handler(void *unused){
	printk("NMI_Handler\r\n");
	for(;;);
}
