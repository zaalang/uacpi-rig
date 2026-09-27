cmake_minimum_required(VERSION 3.16)

project(Rig C ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/../uacpi.cmake)

add_executable(
    Rig
    rig.c
    uefi.c
    uefi.s
)

target_sources(
    Rig
    PRIVATE
    ${UACPI_SOURCES}
)

target_include_directories(
    Rig
    PRIVATE
    ${UACPI_INCLUDES}
)

target_link_options(
    Rig
    PRIVATE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/uefi.ld"
    "-nostdlib"
    "-pie"
    "-Wl,--no-dynamic-linker"
    "-Bsymbolic"
    "-znocombreloc"
)

set_target_properties(
    Rig
    PROPERTIES
    OUTPUT_NAME Rig.elf
    LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/uefi.ld"
)

add_custom_command(
    TARGET Rig
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -j .text -j .data -j .tls -j .dynamic -j .rela -j .reloc --output-target pei-x86-64 --subsystem=efi-app ${CMAKE_CURRENT_BINARY_DIR}/Rig.elf ${CMAKE_CURRENT_BINARY_DIR}/Rig.efi
)