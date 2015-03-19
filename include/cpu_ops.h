
//
// Defines a function purely for internal use within the cpu module.
// The table of cpu ops is defined in cpu_ops.c and this function gives a way
// for the functions in cpu.c to get at the table.
//



#ifndef __CPU_OPS_H
#define __CPU_OPS_H

#include "cpu.h"
#include "cpu_private.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

opcode_table_t* get_instruction_table(void);
bool is_memory_instruction(uint8_t opcode);
bool is_pc_relative_instruction(uint8_t opcode);


#endif
