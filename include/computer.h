

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

#endif
