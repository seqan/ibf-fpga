###############################################################################
### FPGA Hardware
###############################################################################
# To compile manually:
#   icpx -fsycl -fintelfpga -qactypes -c host.cpp -o host.o
#   icpx -fsycl -fintelfpga -qactypes -Xshardware -Xstarget=<FPGA_DEVICE> -fsycl-link=image kernel/kernel.cpp -o dev_image.a
#   icpx -fsycl -fintelfpga -qactypes host.o dev_image.a -o min-ibf-fpga-oneapi.fpga

set(DEVICE_IMAGE_OBJ "dev_image.a")

set(CMAKE_CXX_FLAGS_LIST "${CMAKE_CXX_FLAGS}")
separate_arguments(CMAKE_CXX_FLAGS_LIST)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_IMAGE_OBJ}
                   COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS_LIST} -fsycl -fintelfpga -qactypes -Xshardware -Xstarget=${FPGA_DEVICE} ${USER_HARDWARE_FLAGS} -Xshyper-optimized-handshaking=off -fsycl-link=image ${CMAKE_CURRENT_SOURCE_DIR}/../fpga_device.cpp -o ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_IMAGE_OBJ} -I${CMAKE_CURRENT_LIST_DIR}/../../include
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../fpga_device.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../kernel_w23_k19_b64.hpp)

add_library(fpga_device.fpga OBJECT IMPORTED)
set_property(TARGET fpga_device.fpga PROPERTY IMPORTED_OBJECTS
    "${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_IMAGE_OBJ}" # <build-dir>/dev_image.a
)

add_executable(min-ibf-fpga-oneapi.fpga ../host.cpp)
target_include_directories(min-ibf-fpga-oneapi.fpga PRIVATE ../../include)
target_link_libraries(min-ibf-fpga-oneapi.fpga PRIVATE intel_fpga_host)
target_link_libraries(min-ibf-fpga-oneapi.fpga PRIVATE fpga_device.fpga)

add_custom_target(fpga DEPENDS min-ibf-fpga-oneapi.fpga)