/****************************************************************************
 * configs/sc2355a/scripts/ld.script
 *
 *   Copyright (C) 2016, 2017 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *           Sebastien Lorquet <sebastien@lorquet.fr>
 *           dev@ziggurat29.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/* The sc2355a totally has 963KB sram and 4MB flash. The start address of
 * sram is 0x00100000(0x00100000~0x0017FFFF is IRAM; 0x00180000~1F0C00 is DRAM)
 * and the start address of flash is 0x02000000.
 *
 * The memory map of sram is:
 * |--------------------------------------------------------------|
 * | 0x00100000     | 0x00167000 | 0x00180000 | 0x001E4000
 * | 412K(ram boot) | 100K(ipc)  | 400K(data) | 51K(LWIP stack)
 * |--------------------------------------------------------------|
 *
 * The memory map of flash is:
 * |----------------------------------------------------------------------------------------------------------------------------------------|
 * | 0x02000000       | 0x02030000           | 0x02040000   | 0x020C0000      | 0x02180000         | 0x022C0000       | 0x02340000          |
 * | 192K(bootloader) | 64K(bootloader env)  | 512K(kernel) | 768K(Modem WCN) | 1280K(File system) | 512K(kernel OTA) | 768K(Modem WCN OTA) |
 * |----------------------------------------------------------------------------------------------------------------------------------------|
 *
 *
 */

MEMORY
{

	sram           (rwx) : ORIGIN = 0x00100240, LENGTH = (324K - 0x240)
	dram           (rwx) : ORIGIN = 0x00180000, LENGTH =  400K
	wifi_data_ram  (rwx) : ORIGIN = 0x00151000, LENGTH =  188K
	flash           (rx) : ORIGIN = 0x02000000, LENGTH =  192K
	critical_info   (rx) : ORIGIN = 0x02030000, LENGTH =    4K
	flash_app       (rx) : ORIGIN = 0x02040000, LENGTH =  512K
	flash_modem     (rx) : ORIGIN = 0x020C0000, LENGTH =  768K
	flash_filesys   (rx) : ORIGIN = 0x02180000, LENGTH = 1280K
	flash_app_ota   (rx) : ORIGIN = 0x022C0000, LENGTH =  512K
	flash_modem_ota (rx) : ORIGIN = 0x02340000, LENGTH =  768K
/*	flash (rx) : ORIGIN = 0x02000000, LENGTH = 512K
	sram (rwx) : ORIGIN = 0x00100000, LENGTH = 400K */
}

__SRAM_segment_start__ = ORIGIN( sram );
__SRAM_segment_end__ = __SRAM_segment_start__ + LENGTH( sram );

OUTPUT_ARCH(arm)
ENTRY(__start)
SECTIONS
{
	.isr_vector : {
		. = ALIGN(4);
		_isr_vector = .;
		KEEP(*(.isr_vector))
	} > flash

	.text : {
		*(.vectors)
		*( EXCLUDE_FILE (*hal_sfc.o *hal_sfc_phy.o *ramfunc_flash_api.o) .text.*)
		*(.fixup)
		*(.gnu.warning)
		*(.rodata .rodata.*)
		*(.gnu.linkonce.t.*)
		*(.glue_7)
		*(.glue_7t)
		*(.got)
		*(.gcc_except_table)
		*(.gnu.linkonce.r.*)
		_etext = ABSOLUTE(.);
	} > flash

	.init_section : {
		_sinit = ABSOLUTE(.);
		*(.init_array .init_array.*)
		_einit = ABSOLUTE(.);
	} > flash

	.ARM.extab : {
		*(.ARM.extab*)
	} > flash

	__exidx_start = ABSOLUTE(.);
	.ARM.exidx : {
		*(.ARM.exidx*)
	} > flash
	__exidx_end = ABSOLUTE(.);

    . = ALIGN(4);
    _etext = .;

	/* The UWP566x has 400Kb of SRAM beginning at the following address */

	_sidata = _etext;
	
	.data : AT( _sidata ) {
		. = ALIGN(4);
		_sdata = ABSOLUTE(.);
		*(.devconfig.init)
		*(.init_)
		*(.data .data.*)
		*(.gnu.linkonce.d.*)
		CONSTRUCTORS
		. = ALIGN(4);
		_edata = ABSOLUTE(.);
	} > sram

	__ramfunc_start__ = _etext + SIZEOF(.data);
	.ramtext : AT( __ramfunc_start__ ) {
		. = ALIGN(4);
		__ramtext_start__ = ABSOLUTE(.);
		*ramfunc_flash_api.o(.text*)
		*hal_sfc.o(.text*)
		*hal_sfc_phy.o(.text*)
		__ramtext_end__ = ABSOLUTE(.);
	} > sram

	.bss : {
		. = ALIGN(4);
		_sbss = ABSOLUTE(.);
		*(.bss .bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = ABSOLUTE(.);
	} > sram

	.stack_dummy : {
		*(.stack)
	} > sram
	
	__StackTop = ORIGIN(sram) + LENGTH(sram);
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __StackTop);
	
	ASSERT(__StackLimit >= _ebss, "region sram overflowed with stack")
	
	.iram : {
		. = ALIGN(4);
		_siram = ABSOLUTE(.);
		*(.iram .iram.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(4);
		_eiram = ABSOLUTE(.);
	} > sram

	.heap :
	{
		*(.heap*)
	} > dram
	
	.SECTIONWIFIDATARAM (NOLOAD) :
	{
		*(SECTIONWIFIDATARAM)
	} > wifi_data_ram

	.critical_info_region (NOLOAD) : {
		*(.CERTIFICATE_ENTRY)
		*(.UWP_OTA_INFO)
	} > critical_info
	
	/* Stabs debugging sections. */
	.stab 0 : { *(.stab) }
	.stabstr 0 : { *(.stabstr) }
	.stab.excl 0 : { *(.stab.excl) }
	.stab.exclstr 0 : { *(.stab.exclstr) }
	.stab.index 0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment 0 : { *(.comment) }
	.debug_abbrev 0 : { *(.debug_abbrev) }
	.debug_info 0 : { *(.debug_info) }
	.debug_line 0 : { *(.debug_line) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	.debug_aranges 0 : { *(.debug_aranges) }
}
