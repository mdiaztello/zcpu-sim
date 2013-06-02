
/*
 * Defines the outward facing interface of the cpu module for the rest of the
 * program
 */



#ifndef __CPU_H_
#define __CPU_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


typedef struct cpu cpu_t;

cpu_t* make_cpu(void);
void init_cpu(cpu_t* cpu);
void destroy_cpu(cpu_t* cpu);
void cpu_cycle(cpu_t* cpu);

#endif
