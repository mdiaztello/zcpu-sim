
//This submodule implements all of the opcodes for the CPU and sticks them in 
//a table that the cpu structure links to when it is created.

#include "bit_twiddling.h"
#include "cpu_ops.h"
#include "opcode_list.h"
#include "debug.h"
#include <stdlib.h> //for exit()

struct cpu backup_cpu; //holds backups of our cpu's registers, etc while in interrupt mode

static cpu_op instruction_table[NUM_INSTRUCTIONS];

enum condition_code_register_bit_position_t { POSITIVE_BIT = 0, ZERO_BIT = 1, NEGATIVE_BIT = 2 };

static const uint8_t INTERRUPT_IN_PROCESS_BIT = 0;

static void set_interrupt_in_process_status(cpu_t* cpu, bool interrupt_in_process);
//sets the condition code bits according to the result of the last ALU operation
//FIXME: Do I want or need additional condition codes?
void update_condition_code_bits(cpu_t* cpu, uint32_t result)
{
    cpu->CCR = 0; //condition code bits are mutually exclusive
    const uint8_t SIGN_BIT = 31;
    if(0 == result)
    {
        BIT_SET(cpu->CCR, ZERO_BIT);
    }
    else if(CHECK_BIT_SET(result, SIGN_BIT)) //negative result b/c of sign bit
    {
        BIT_SET(cpu->CCR, NEGATIVE_BIT);
    }
    else
    {
        BIT_SET(cpu->CCR, POSITIVE_BIT);
    }
}


bool interrupt_in_process(cpu_t* cpu)
{
    return CHECK_BIT_SET(cpu->process_status_reg, INTERRUPT_IN_PROCESS_BIT);
}


void set_interrupt_in_process_status(cpu_t* cpu, bool interrupt_in_process)
{
    if(interrupt_in_process)
    {
        BIT_SET(cpu->process_status_reg, INTERRUPT_IN_PROCESS_BIT);
    }
    else
    {
        BIT_CLEAR(cpu->process_status_reg, INTERRUPT_IN_PROCESS_BIT);
    }
}

static bool is_operating_system_scheduler_interrupt(interrupt_controller_t* ic)
{
    //OS scheduler interrupt service routines must manually save software
    //context because the whole point of the scheduler is that it will not
    //return to the previous context after exiting the ISR, and
    //saving/restoring the machine state automatically would defeat this.
    const uint8_t PREEMPTIVE_SCHEDULER_IRQ = IRQ_0;
    const uint8_t COOPERATIVE_SCHEDULER_IRQ = IRQ_128;
    uint8_t interrupt_source = get_interrupt_source(ic);
    return (PREEMPTIVE_SCHEDULER_IRQ == interrupt_source || COOPERATIVE_SCHEDULER_IRQ == interrupt_source);
}

static void backup_machine_state(cpu_t* cpu)
{
    backup_cpu = *cpu;
}

static void restore_machine_state(cpu_t* cpu)
{
    *cpu = backup_cpu;
}

void enter_interrupt_mode(cpu_t* cpu)
{
    if(!is_operating_system_scheduler_interrupt(cpu->ic))
    {
        backup_machine_state(cpu);
    }

    set_interrupt_in_process_status(cpu, true);

    beacon();       //DEBUG
    exit(-1);       //DEBUG

    cpu->PC = get_interrupt_vector_table_starting_address(cpu->ic) + get_interrupt_source(cpu->ic);
}



void exit_interrupt_mode(cpu_t* cpu)
{
    if(!is_operating_system_scheduler_interrupt(cpu->ic))
    {
        restore_machine_state(cpu);
    }
    set_interrupt_in_process_status(cpu, false);
    beacon();
}


//tells us if the instruction touches memory during execution (i.e. load/store)
bool is_memory_instruction(uint8_t opcode)
{
    return ((OPCODE_LOAD <= opcode) && (opcode <= OPCODE_STORER));
}

bool is_load_instruction(uint8_t opcode)
{
    return ((OPCODE_LOAD == opcode) || (OPCODE_LOADR == opcode) || (OPCODE_LOADA == opcode));
}

bool is_pc_relative_instruction(uint8_t opcode)
{
    return (OPCODE_LOAD == opcode) || (OPCODE_LOADA == opcode) || (OPCODE_STORE == opcode);
}

bool is_load_effective_address_instruction(uint8_t opcode)
{
    return  OPCODE_LOADA == opcode;
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
    if(!cpu->immediate_mode)
    {
        *cpu->destination_reg1 = *cpu->source_reg1 ^ *cpu->source_reg2;
    }
    else
    {
        //FIXME: I may need to change this
        //we will fill the upper bits with 0's for now in the immediate mode
        //operand so that it does no damage to what's in the upper 16-bits of
        //the register
        cpu->ALU_immediate_bits &= 0x0000FFFF ;
        *cpu->destination_reg1 = *cpu->source_reg1 ^ cpu->ALU_immediate_bits;
    }

    update_condition_code_bits(cpu, *cpu->destination_reg1);
}


void cpu_load_pc_relative(cpu_t* cpu)
{
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

//  JUMPR (base register + Offset)
//      opcode = 010100
//      e.g. JUMPR <unused-bits> <base_reg> <base_reg_offset>
//          6-bits + 5-bits-unused + 5-bits + 16-bit-offset
void cpu_jump_base_plus_offset(cpu_t* cpu)
{
    cpu->PC = *cpu->base_reg + cpu->base_register_offset_bits;
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
    uint32_t N = CHECK_BIT_SET(cpu->instruction_condition_codes, NEGATIVE_BIT);
    uint32_t Z = CHECK_BIT_SET(cpu->instruction_condition_codes, ZERO_BIT);
    uint32_t P = CHECK_BIT_SET(cpu->instruction_condition_codes, POSITIVE_BIT);

    if((cpu->CCR & N) || (cpu->CCR & Z) || (cpu->CCR & P))
    {
        cpu->PC = cpu->PC + cpu->branch_pc_relative_offset_bits;
    }
}


//  ADD:
//      opcode = 000100
//      e.g. ADD <destination_reg1> <source_reg1> <source_reg2>
//           6-bits + 5-bits + 5-bits + 5 bits + 11-unused-bits
//  ADD_IMMEDIATE:
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

//  SUB:
//      opcode = 000101
//      e.g. SUB <destination_reg1> <source_reg1> <source_reg2>
//           6-bits + 5-bits + 5-bits + 5 bits + 11-unused-bits
//  SUB_IMMEDIATE:
//      opcode = 000101
//      e.g. SUB <destination_reg1> <source_reg1> <immediatel_val> <immediate-flag>
//           6-bits + 5-bits + 5-bits + 15-bit-immediate + 1-bit-flag

void cpu_sub(cpu_t* cpu)
{
    if(!cpu->immediate_mode)
    {
        *cpu->destination_reg1 = *cpu->source_reg1 - *cpu->source_reg2;
    }
    else
    {
        *cpu->destination_reg1 = *cpu->source_reg1 - cpu->ALU_immediate_bits;
    }

    update_condition_code_bits(cpu, *cpu->destination_reg1);
}

//  CALL (pc-relative)
//      opcode = 010010
//      e.g. CALL <pc-relative-offset>
//          6-bits + 26-bits

void cpu_call(cpu_t* cpu)
{
    //R30 on our machine will be the return address to come back to for subroutines
    cpu->registers[30] = cpu->PC;
    cpu_jump_pc_relative(cpu);
}

void cpu_callr(cpu_t* cpu)
{
    cpu->registers[30] = cpu->PC;
    cpu_jump_base_plus_offset(cpu);
}

//  TRAP/SWI/SYSCALL
//      OPCODE = 010101
//      e.g. TRAP <trap-vector-register> <unused-bits>
//          6-bits + 5-bits-unused + 21-bits-unused
void cpu_swi(cpu_t* cpu)
{
    //our system has 256 possible interrupt sources, and the last half of them
    //are dedicated to software interrupt sources
    const uint8_t SOFTWARE_INTERRUPT_VECTOR_TABLE_STARTING_OFFSET = 128;
    uint8_t software_irq_number = GET_BITS_IN_RANGE(*cpu->trap_vector_register, 0, 6) + SOFTWARE_INTERRUPT_VECTOR_TABLE_STARTING_OFFSET;
    request_interrupt(cpu->ic, software_irq_number);
}


//  RETURNI/RFI/SYSCALL_EXIT
//      opcode = 010110
//      e.g. RETURNI
//          "RETURNI 0x0000000"
//          6-bits + 26-bits-unused
void cpu_rfi(cpu_t* cpu)
{
    exit_interrupt_mode(cpu);
}


void cpu_nop(cpu_t* cpu)
{
    if(cpu != NULL)
    {
        //message(__FUNCTION__);
    }
}

static cpu_op instruction_table[NUM_INSTRUCTIONS] =
{
    &cpu_and, &cpu_or, &cpu_not, &cpu_xor, &cpu_add, &cpu_sub, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_load_pc_relative, &cpu_load_base_plus_offset, &cpu_load_effective_address, &cpu_nop, &cpu_nop,
    &cpu_jump_pc_relative, &cpu_branch, &cpu_call, &cpu_callr, &cpu_jump_base_plus_offset, &cpu_swi, &cpu_rfi, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
    &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop, &cpu_nop,
};
