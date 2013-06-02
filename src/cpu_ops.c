
//This submodule implements all of the opcodes for the CPU and sticks them in 
//a table that the cpu structure links to when it is created.

#include "cpu_ops.h"
#include "debug.h"

static cpu_op instruction_table[NUM_INSTRUCTIONS];



static void message(const char* msg)
{
    printf("\nexecuting %s\n", msg);
}



opcode_table_t* get_instruction_table(void)
{
    return &instruction_table;
}

//  AND: bitwise ANDs the contents of sr1 and sr2 and stores them in dr1
//      opcode = 000000
//      e.g. AND <destination_reg1> <source_reg1> <source_reg2>
void cpu_and(cpu_t* cpu)
{
    //FIXME: do we need to update the CCR?
    *cpu->destination_reg1 = *cpu->source_reg1 & *cpu->source_reg2;
}

//  OR: bitwise ORs the contents of sr1 and sr2 and stores them in dr1
//      opcode = 000001
//      e.g. OR <destination_reg1> <source_reg1> <source_reg2>

void cpu_or(cpu_t* cpu)
{
    //FIXME: do we need to update the CCR?
    *cpu->destination_reg1 = *cpu->source_reg1 | *cpu->source_reg2;
}

//    NOT: negates contents of sr1 and puts them in dr1
//        opcode = 000010
//        e.g. NOT <destination_reg1> <source_reg1>
void cpu_not(cpu_t* cpu)
{
    //FIXME: do we need to update the CCR?
    //message(__FUNCTION__);
    *cpu->destination_reg1 = ~(*cpu->source_reg1);
    //printf("source reg = 0x%08X\n", *cpu->source_reg1);
    //printf("destination reg = 0x%08X\n", *cpu->destination_reg1);
}


void cpu_nop(cpu_t* cpu)
{
    if(cpu != NULL)
    {
        message(__FUNCTION__);
    }
}

static cpu_op instruction_table[NUM_INSTRUCTIONS] =
{
    &cpu_and, &cpu_or, &cpu_not, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
};
