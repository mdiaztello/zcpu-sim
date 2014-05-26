
//
//This header contains information that is private for the CPU module, but that
//must be shared among all of the files of the CPU module
//
//

#ifndef __CPU_PRIVATE_H_
#define __CPU_PRIVATE_H_

#include "cpu.h" //for the cpu struct typedef and the cpu_op typdef
#include "memory.h"

#define NUM_REGISTERS 32
#define NUM_INSTRUCTIONS 64

typedef void (*cpu_op)(cpu_t*);
typedef cpu_op opcode_table_t[NUM_INSTRUCTIONS];


struct cpu
{
    uint32_t registers[NUM_REGISTERS]; //the general purpose registers for the processor
    uint32_t PC;            //the program counter of the processor
    uint32_t CCR;           //condition codes register
    uint32_t IR;            //instruction register
    uint32_t MDR;           //memory data register
    uint32_t MAR;           //memory address register
    memory_t* RAM;          //pointer to our memory interface

    uint32_t  opcode;           //the type of instruction we are executing
    uint32_t* source_reg1;      //source register 1 for an integer/logical operation
    uint32_t* source_reg2;      //source register 2 for an integer/logical operation
    uint32_t* destination_reg1; //destination register 1 for an integer/logical operation
    uint32_t* destination_reg2; //destination register 2 for an integer/logical operation
                                //NOTE: destination_reg2 is only used for multiplication
    bool immediate_mode;
    uint32_t ALU_immediate_bits; // only the lower 15 bits of this field are taken from the instruction

    //pointer to table of function pointers representing the opcodes goes here
    opcode_table_t* opcodes;

};

#endif
