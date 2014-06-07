

#ifndef __COMPUTER_H_
#define __COMPUTER_H_

#include <stdint.h>
#include <stdlib.h>

typedef struct computer_t computer_t;

computer_t* build_computer(void);
void computer_reset(computer_t* computer);
void computer_load_program(computer_t* computer, uint32_t* program, size_t program_length);
void computer_single_step(computer_t* computer);
void computer_run(computer_t* computer);

void dump_computer_cpu_state(computer_t* computer);
void dump_computer_memory(computer_t* computer, size_t starting_address, size_t ending_address);
void computer_print_elapsed_cycles(computer_t* computer);

#endif
