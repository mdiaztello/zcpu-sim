
#ifndef __MEMORY_MAP_H_
#define __MEMORY_MAP_H_




/*
    PHYSICAL MEMORY MAP (32-bit address space)
    ==========================================

    Address Range               Mapping                 Size(4-byte locs)
    ---------------------------------------------------------------------
    0x00000000 - 0x00000FFF     Boot ROM                4096
    0x00001000 - 0x000010FF     Interrupt Vector Table  256
    0x00001100 - 0x0004C0FF     Graphics Frame Buffer   307200 = 640 x 480
    0x0004C100 - 0x0004C101     Keyboard                2
    0x0004C102 - 0x0004C1??     Timers                  ?
    0x0004C1?? - 0x0004C2??     PWM?/"serial"?/GPIO?    ?
    0x00050000 - 0xFFFFFFFF     RAM                     several GB
*/


//This header defines useful magic constants that show the memory map of the computer system

#define BOOT_ROM_START                      (0x00)
#define BOOT_ROM_SIZE                       (0x1000)
#define BOOT_ROM_END                        (BOOT_ROM_START + BOOT_ROM_SIZE - 1)
#define INTERRUPT_VECTOR_TABLE_START        (BOOT_ROM_END + 1)
#define INTERRUPT_VECTOR_TABLE_SIZE         (256)
#define INTERRUPT_VECTOR_TABLE_END          (INTERRUPT_VECTOR_TABLE_START + INTERRUPT_VECTOR_TABLE_SIZE - 1)
#define GRAPHICS_REGION_START               (INTERRUPT_VECTOR_TABLE_END + 1)
#define GRAPHICS_REGION_SIZE                (640*480)
#define GRAPHICS_REGION_END                 (GRAPHICS_REGION_START + GRAPHICS_REGION_SIZE - 1)
#define KEYBOARD_REGION_START               (GRAPHICS_REGION_END + 1)
#define KEYBOARD_REGION_SIZE                (2)
#define KEYBOARD_REGION_END                 (KEYBOARD_REGION_START + KEYBOARD_REGION_SIZE - 1)


#endif // __MEMORY_MAP_H_
