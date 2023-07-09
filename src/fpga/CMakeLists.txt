set(FPGA_TARGET ${TARGET_NAME}.fpga)

###############################################################################
### FPGA Hardware
###############################################################################
# To compile manually:
#   icpx -fsycl -fintelfpga -qactypes -c host.cpp -o host.o
#   icpx -fsycl -fintelfpga -qactypes -Xshardware -Xstarget=<FPGA_DEVICE> -fsycl-link=image kernel/kernel.cpp -o dev_image.a
#   icpx -fsycl -fintelfpga -qactypes host.o dev_image.a -o min-ibf-fpga-oneapi.fpga

add_custom_target(fpga DEPENDS ${FPGA_TARGET})
set(DEVICE_IMAGE_OBJ "dev_image.a")

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_IMAGE_OBJ}
                   COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS_LIST} ${HARDWARE_LINK_FLAGS} -fsycl-link=image ${DEVICE_SOURCE_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_IMAGE_OBJ} -I${CMAKE_CURRENT_LIST_DIR}/../../include
                   DEPENDS ${DEVICE_SOURCE_FILE} ${KERNEL_HEADER_FILE})

add_library(fpga_device.fpga OBJECT IMPORTED)
set_property(TARGET fpga_device.fpga PROPERTY IMPORTED_OBJECTS
    "${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_IMAGE_OBJ}" # <build-dir>/dev_image.a
)

add_executable(${FPGA_TARGET} ${HOST_SOURCE_FILE})
target_include_directories(${FPGA_TARGET} PRIVATE ../../include)
target_compile_options(${FPGA_TARGET} PRIVATE ${HARDWARE_COMPILE_FLAGS})
target_link_options(${FPGA_TARGET} PRIVATE -fsycl -fintelfpga -qactypes -Xshyper-optimized-handshaking=off)
target_link_libraries(${FPGA_TARGET} PRIVATE fpga_device.fpga)