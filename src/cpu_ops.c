
//This submodule implements all of the opcodes for the CPU and sticks them in 
//a table that the cpu structure links to when it is created.

#include "cpu_ops.h"
#include "debug.h"

static cpu_op instruction_table[NUM_INSTRUCTIONS];

//tells us if the instruction touches memory during execution (i.e. load/store)
bool is_memory_instruction(uint8_t opcode)
{
    return ((0x0B <= opcode) && (opcode <= 0x0F));
}

bool is_pc_relative_instruction(uint8_t opcode)
{
    const uint8_t LOAD = 0x0B;
    const uint8_t LOADA = 0x0D;
    const uint8_t STORE = 0x0E;
    return (LOAD == opcode) || (LOADA == opcode) || (STORE == opcode);
}

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
    if(!cpu->immediate_mode)
    {
        *cpu->destination_reg1 = *cpu->source_reg1 & *cpu->source_reg2;
    }
    else
    {
        //FIXME: I may need to change this
        //we will fill the upper bits with 1's for now in the immediate mode
        //operand so that it does no damage to what's in the upper 16-bits of
        //the register
        cpu->ALU_immediate_bits |= 0xFFFF0000 ;
        *cpu->destination_reg1 = *cpu->source_reg1 & cpu->ALU_immediate_bits;

    }
}

//  OR: bitwise ORs the contents of sr1 and sr2 and stores them in dr1
//      opcode = 000001
//      e.g. OR <destination_reg1> <source_reg1> <source_reg2>

void cpu_or(cpu_t* cpu)
{
    //FIXME: do we need to update the CCR?
    if(!cpu->immediate_mode)
    {
        *cpu->destination_reg1 = *cpu->source_reg1 | *cpu->source_reg2;
    }
    else
    {
        //FIXME: I may need to change this
        //we will fill the upper bits with 0's for now in the immediate mode
        //operand so that it does no damage to what's in the upper 16-bits of
        //the register
        cpu->ALU_immediate_bits &= 0x0000FFFF ;
        printf("the immediate-mode value was 0x%08X\n", cpu->ALU_immediate_bits);
        *cpu->destination_reg1 = *cpu->source_reg1 | cpu->ALU_immediate_bits;
    }
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



void cpu_load_pc_relative(cpu_t* cpu)
{
    beacon();
    *cpu->destination_reg1 = cpu->MDR;
}

void cpu_load_base_plus_offset(cpu_t* cpu)
{
    *cpu->destination_reg1 = cpu->MDR;
}

void cpu_nop(cpu_t* cpu)
{
    beacon();
    if(cpu != NULL)
    {
        message(__FUNCTION__);
    }
}

static cpu_op instruction_table[NUM_INSTRUCTIONS] =
{
    &cpu_and, &cpu_or, &cpu_not, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_load_pc_relative, &cpu_load_base_plus_offset, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
};
