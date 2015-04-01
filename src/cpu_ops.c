
//This submodule implements all of the opcodes for the CPU and sticks them in 
//a table that the cpu structure links to when it is created.

#include "cpu_ops.h"
#include "debug.h"

static cpu_op instruction_table[NUM_INSTRUCTIONS];

//sets the condition code bits according to the result of the last ALU operation
//FIXME: Do I want or need additional condition codes?
void update_condition_code_bits(cpu_t* cpu, uint32_t result)
{
    if(0 == result)
    {
        cpu->CCR = (0x00000002); //set zero bit of CCR
    }
    else if(result >> 31) //negative result b/c of sign bit
    {
        cpu->CCR = 0x00000004; //set negative bit of CCR
    }
    else
    {
        cpu->CCR = 0x00000001; //set positive bit of CCR
    }

}

//tells us if the instruction touches memory during execution (i.e. load/store)
bool is_memory_instruction(uint8_t opcode)
{
    return ((0x0B <= opcode) && (opcode <= 0x0F));
}

bool is_load_instruction(uint8_t opcode)
{
    const uint8_t LOAD = 0x0B;
    const uint8_t LOADR = 0x0C;
    const uint8_t LOADA = 0x0D;

    return ((LOAD == opcode) || (LOADR == opcode) || (LOADA == opcode));
}

bool is_pc_relative_instruction(uint8_t opcode)
{
    const uint8_t LOAD = 0x0B;
    const uint8_t LOADA = 0x0D;
    const uint8_t STORE = 0x0E;
    return (LOAD == opcode) || (LOADA == opcode) || (STORE == opcode);
}

bool is_load_effective_address_instruction(uint8_t opcode)
{
    const uint8_t LOADA = 0x0D;
    return  LOADA == opcode;
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

    update_condition_code_bits(cpu, *cpu->destination_reg1);
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
        *cpu->destination_reg1 = *cpu->source_reg1 | cpu->ALU_immediate_bits;
    }

    update_condition_code_bits(cpu, *cpu->destination_reg1);
}

//    NOT: negates contents of sr1 and puts them in dr1
//        opcode = 000010
//        e.g. NOT <destination_reg1> <source_reg1>
void cpu_not(cpu_t* cpu)
{
    //FIXME: do we need to update the CCR?
    *cpu->destination_reg1 = ~(*cpu->source_reg1);
    update_condition_code_bits(cpu, *cpu->destination_reg1);
}

void cpu_xor(cpu_t* cpu)
{
    //FIXME: stub for now
    cpu_nop(cpu);
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

void cpu_load_effective_address(cpu_t* cpu)
{
    *cpu->destination_reg1 = cpu->PC + cpu->load_pc_relative_offset_bits;
}

void cpu_jump_pc_relative(cpu_t* cpu)
{
    cpu->PC = cpu->PC + cpu->jump_pc_relative_offset_bits;
}

// <BRANCH_OPCODE> <condition-flags-to-check> <pc-relative-offset>
//     6-bits + 3-bits + 23-bits
// BRANCH_OPCODE = 010001
// Branching works by comparing the 3 condition codes embedded in the
// instruction against the condition codes set in the CCR, if any of the
// condition codes are set, and the corresponding condition code is set in the
// instruction, the branch will be taken, otherwise the PC will be left in its
// current (incremented) state
void cpu_branch(cpu_t* cpu)
{
    uint32_t N = (1u << 2) & cpu->instruction_condition_codes;
    uint32_t Z = (1u << 1) & cpu->instruction_condition_codes;
    uint32_t P = (1u << 0) & cpu->instruction_condition_codes;

    if((cpu->CCR & N) || (cpu->CCR & Z) || (cpu->CCR & P))
    {
        cpu->PC = cpu->PC + cpu->branch_pc_relative_offset_bits;
    }
}


//  ADD:
//      opcode = 000100
//      e.g. ADD <destination_reg1> <source_reg1> <source_reg2>
//           6-bits + 5-bits + 5-bits + 5 bits + 11-unused-bits
//  ADD_IMMEDIATE
//      opcode = 000100
//      e.g. ADD <destination_reg1> <source_reg1> <immediatel_val> <immediate-flag>
//           6-bits + 5-bits + 5-bits + 15-bit-immediate + 1-bit-flag

void cpu_add(cpu_t* cpu)
{
    if(!cpu->immediate_mode)
    {

        *cpu->destination_reg1 = *cpu->source_reg1 + *cpu->source_reg2;
    }
    else
    {
        *cpu->destination_reg1 = *cpu->source_reg1 + cpu->ALU_immediate_bits;
    }

    update_condition_code_bits(cpu, *cpu->destination_reg1);
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
    &cpu_and, &cpu_or, &cpu_not, &cpu_xor, &cpu_add, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_load_pc_relative, &cpu_load_base_plus_offset, &cpu_load_effective_address, &cpu_nop, &cpu_nop,
    &cpu_jump_pc_relative, &cpu_branch, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
};
