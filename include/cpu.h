
/*
 * Defines the outward facing interface of the cpu module for the rest of the
 * program
 */



#ifndef __CPU_H_
#define __CPU_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory_bus.h"
#include "interrupt_controller.h"


typedef struct cpu cpu_t;

cpu_t* make_cpu(memory_bus_t* bus, interrupt_controller_t* ic);
void cpu_reset(cpu_t* cpu);
void init_cpu(cpu_t* cpu);
void destroy_cpu(cpu_t* cpu);
void cpu_cycle(cpu_t* cpu);
void cpu_load_program(cpu_t* cpu, uint32_t program[], size_t program_length);
void dump_cpu_state(cpu_t* cpu);
bool cpu_completed_instruction(cpu_t* cpu);

#endif
