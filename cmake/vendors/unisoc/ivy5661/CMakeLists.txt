cmake_minimum_required(VERSION 3.13)

set(simplelink_dir "${AFR_3RDPARTY_DIR}/mcu_vendor/unisoc/uwp566x")
#set(board_demos_dir "${AFR_ROOT_DIR}/demos/unisoc/ivy5661/common")
set(board_tests_dir "${AFR_ROOT_DIR}/tests/unisoc/ivy5661/common")
#if(AFR_IS_TESTING)
    set(board_dir "${board_tests_dir}")
#else()
    #set(board_dir "${board_demos_dir}")
#endif()

# -------------------------------------------------------------------------------------------------
# Amazon FreeRTOS Console metadata
# -------------------------------------------------------------------------------------------------
afr_set_board_metadata(NAME "UNISOC-IVY5661")
afr_set_board_metadata(DISPLAY_NAME "UNISOC-IVY5661")
afr_set_board_metadata(DESCRIPTION "UNISOC-IVY5661 Wi-Fi® LaunchPad™ Development Kit")
afr_set_board_metadata(VENDOR_NAME "UNISOC")
afr_set_board_metadata(FAMILY_NAME "UWP566X")
afr_set_board_metadata(DATA_RAM_MEMORY "963KB")
afr_set_board_metadata(PROGRAM_MEMORY "4MB")
afr_set_board_metadata(CODE_SIGNER "AmazonFreeRTOS-UNISOC-IVY5661")
afr_set_board_metadata(SUPPORTED_IDE "ECLIPSE")
afr_set_board_metadata(IDE_ECLIPSE_NAME "ECLIPSE")
afr_set_board_metadata(IDE_ECLIPSE_COMPILERS "ECLIPSE")

# -------------------------------------------------------------------------------------------------
# Compiler settings
# -------------------------------------------------------------------------------------------------
afr_mcu_port(compiler)
target_compile_definitions(
    AFR::compiler::mcu_port
    INTERFACE IVY5661
)

set(common_flags "-mcpu=cortex-m4" "-mthumb" "-mlittle-endian" "-mfloat-abi=soft" "-Og" "-fmessage-length=0" "-fsigned-char" "-ffunction-sections" "-fdata-sections" "-fstrict-volatile-bitfields" "-Wall" "-g3" "-ggdb")
set(c_flags "-std=gnu99" "-nostdlib")
set(asm_flags "-x" "assembler-with-cpp")
set(linker_flags "-specs=nosys.specs" "-Wl,-Map=output.map,--gc-sections" "-u _printf_float" "-lgcc")

target_compile_options(
    AFR::compiler::mcu_port
    INTERFACE
    $<$<COMPILE_LANGUAGE:C>:${common_flags} ${c_flags}> # This only have effect on C files.
    $<$<COMPILE_LANGUAGE:ASM>:${common_flags} ${asm_flags}> # This only have effect on ASM files.
)
target_link_options(
    AFR::compiler::mcu_port
    INTERFACE ${common_flags} ${linker_flags}
)

# -------------------------------------------------------------------------------------------------
# Amazon FreeRTOS portable layers
# -------------------------------------------------------------------------------------------------
# Normally the portable layer for kernel should be vendor's driver code.
afr_mcu_port(kernel)
target_sources(
    AFR::kernel::mcu_port
    INTERFACE
		"${simplelink_dir}/CMSIS/mbed_fault_handler.c"
        "${simplelink_dir}/CMSIS/device/system_UWP5661.c"
		"${simplelink_dir}/gcc/except.S"
		"${simplelink_dir}/gcc/startup_ARMCM4.S"
		"${simplelink_dir}/HAL_Driver/hal_adapt/hal_ramfunc.c"
		"${simplelink_dir}/HAL_Driver/hal_adapt/uwp_rtos_posix.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/drivers/src/hal_pinmux_uwp5661.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/drivers/src/hal_pinmux_uwp5662.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/drivers/src/sfc/hal_sfc.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/drivers/src/sfc/hal_sfc_phy.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/drivers/src/sipc/uwp_irq_relay.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/drivers/src/sipc/sblock.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/drivers/src/sipc/sipc.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/drivers/src/sipc/smsg.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/soc/cache.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/soc/gnss.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/soc/load_fw.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/soc/system.c"
		"${simplelink_dir}/HAL_Driver/uwp_int/uwp_intc.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal_api/gpio_api.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal_api/ramfunc_flash_api.c"
		"${simplelink_dir}/HAL_Driver/uwp_hal_api/serial_api.c"
		"${simplelink_dir}/logging/aws_logging_task_dynamic_buffers.c"
		"${simplelink_dir}/uwpwifi/driver/source/uwp_buf_mgmt.c"
		"${simplelink_dir}/uwpwifi/driver/source/uwp_psk.c"
		"${simplelink_dir}/uwpwifi/driver/source/uwp_sys_wrapper.c"
		"${simplelink_dir}/uwpwifi/driver/source/uwp_wifi_cmdevt.c"
		"${simplelink_dir}/uwpwifi/driver/source/uwp_wifi_ipc.c"
		"${simplelink_dir}/uwpwifi/driver/source/uwp_wifi_main.c"
		"${simplelink_dir}/uwpwifi/driver/source/uwp_wifi_txrx.c"
        "${AFR_MODULES_DIR}/FreeRTOS/portable/GCC/ARM_CM3/port.c"
		"${AFR_MODULES_DIR}/FreeRTOS/portable/GCC/ARM_CM3/portmacro.h"
		"${AFR_MODULES_DIR}/third_party/unity/extras/fixture/src/unity_fixture.c"
)
target_include_directories(
    AFR::kernel::mcu_port
    INTERFACE
        "${simplelink_dir}/CMSIS"
		"${simplelink_dir}/CMSIS/device"
		"${simplelink_dir}/CMSIS/include"
		"${simplelink_dir}/gcc"
		"${simplelink_dir}/HAL_Driver"
		"${simplelink_dir}/HAL_Driver/hal_adapt"
		"${simplelink_dir}/HAL_Driver/uwp_hal_api"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/drivers/include"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/drivers/src/sipc"
		"${simplelink_dir}/HAL_Driver/uwp_hal5/soc"
		"${simplelink_dir}/HAL_Driver/uwp_int"
		"${simplelink_dir}/uwpwifi/driver/include"
		"${board_dir}/config_files"
        "${board_dir}/application_code/vendor_code/bsp"
        "${board_dir}/application_code/vendor_code/ota_include"
		"${AFR_ROOT_DIR}/tests/common/ota"
		"${AFR_MODULES_DIR}/FreeRTOS-Plus-TCP/include"
		"${AFR_MODULES_DIR}/FreeRTOS-Plus-TCP/source/portable/Compiler/GCC"
		"${AFR_MODULES_DIR}/third_party/unity/extras/fixture/src"
		"${AFR_MODULES_DIR}/third_party/unity/src"
		"${AFR_MODULES_DIR}/third_party/mbedtls/include"
		"${AFR_MODULES_DIR}/third_party/pkcs11"
		"${AFR_MODULES_DIR}/FreeRTOS/portable/GCC/ARM_CM3"
		"${AFR_TESTS_DIR}/include"
        # Need aws_clientcredential.h
        #"$<IF:${AFR_IS_TESTING},${AFR_TESTS_DIR},${AFR_DEMOS_DIR}>/include"
)

# utils
afr_mcu_port(utils)

# WiFi
afr_mcu_port(wifi)
target_sources(
    AFR::wifi::mcu_port
    INTERFACE "${AFR_MODULES_DIR}/wifi/portable/unisoc/ivy5661/aws_wifi.c"
)

# freertos_plus_tcp
afr_mcu_port(freertos_plus_tcp)
target_sources(
    AFR::freertos_plus_tcp::mcu_port
    INTERFACE
        "${AFR_MODULES_DIR}/FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_2.c"
        "${AFR_MODULES_DIR}/FreeRTOS-Plus-TCP/source/portable/NetworkInterface/unisoc/NetworkInterface.c"
)

# PKCS11
afr_mcu_port(pkcs11)
target_sources(
    AFR::pkcs11::mcu_port
    INTERFACE
        "${AFR_MODULES_DIR}/pkcs11/portable/unisoc/uwp566x/aws_pkcs11_pal.c"
)
target_link_libraries(
    AFR::pkcs11::mcu_port
    INTERFACE AFR::pkcs11_mbedtls
)

# Secure sockets
afr_mcu_port(secure_sockets)
target_link_libraries(
    AFR::secure_sockets::mcu_port
    INTERFACE AFR::tls 
	AFR::freertos_plus_tcp
)
target_sources(
    AFR::secure_sockets::mcu_port
    INTERFACE "${AFR_MODULES_DIR}/secure_sockets/portable/freertos_plus_tcp/aws_secure_sockets.c"
)

# OTA
afr_mcu_port(ota)
target_sources(
    AFR::ota::mcu_port
    INTERFACE 
        "${AFR_MODULES_DIR}/ota/portable/unisoc/aws_ota_pal.c"
        "${AFR_ROOT_DIR}/demos/common/ota/aws_ota_update_demo.c"
)

# -------------------------------------------------------------------------------------------------
# Amazon FreeRTOS demos and tests
# -------------------------------------------------------------------------------------------------
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

#if(AFR_IS_TESTING)
    set(exe_target aws_tests)
#else()
#    set(exe_target aws_demos)
#endif()

afr_glob_src(board_code_src DIRECTORY "${board_dir}/application_code/vendor_code/bsp")
add_executable(
    ${exe_target}
    ${board_code_src}
    "${board_dir}/application_code/main.c"
)
target_link_libraries(
    ${exe_target}
    PRIVATE
        AFR::wifi
        AFR::utils
		-T "${simplelink_dir}/gcc/linker.ld"
)

find_program(gcc_objcopy arm-none-eabi-objcopy)
find_program(gcc_size arm-none-eabi-size)
if(NOT gcc_objcopy)
    message(FATAL_ERROR "Cannot find arm-none-eabi-objcopy.")
endif()

set(output_file "$<TARGET_FILE_DIR:${exe_target}>/${exe_target}.bin")
set(output_signed_file "$<TARGET_FILE_DIR:${exe_target}>/${exe_target}_signed.bin")
add_custom_command(
    TARGET ${exe_target} POST_BUILD
    COMMAND "${gcc_objcopy}" -O binary "$<TARGET_FILE:${exe_target}>" "${output_file}"
    COMMAND "${gcc_size}" --format=berkeley "$<TARGET_FILE:${exe_target}>"
)

add_custom_command(
    TARGET ${exe_target} POST_BUILD
    COMMAND "/D/Python3/python.exe" "${board_tests_dir}/application_code/vendor_code/scripts/imgtool.py" sign --key    "${board_tests_dir}/application_code/vendor_code/scripts/root-rsa-2048.pem" --header-size 0x1000 --align 8 --version 1.2 --slot-size 0x60000 "${output_file}" "${output_signed_file}"
)

add_custom_command(
    TARGET ${exe_target} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "${output_file}" "${CMAKE_BINARY_DIR}"
)

add_custom_command(
    TARGET ${exe_target} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "${output_signed_file}" "${CMAKE_BINARY_DIR}"
)
