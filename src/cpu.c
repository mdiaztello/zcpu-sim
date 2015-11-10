

//This file defines the cpu module user-facing functions as well
//as any additional functions necessary to implement the CPU internals (except
//for implementing the instructions, which reside in another file).


#include "debug.h"
#include "cpu.h"
#include "cpu_private.h"
#include "cpu_ops.h"
#include "bit_twiddling.h"
#include "interrupt_controller.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


struct cpu backup_cpu; //holds backups of our cpu's registers, etc while in interrupt mode

enum cpu_pipeline_stage_t { INTERRUPT, FETCH1, FETCH2, DECODE, MEMORY1, MEMORY2, EXECUTE };

enum cpu_pipeline_stage_t pipeline_stage = INTERRUPT;

static void install_opcodes(cpu_t* cpu);
static void update_pc(cpu_t* cpu);
static void interrupt(cpu_t* cpu);
static void fetch1(cpu_t* cpu);
static void fetch2(cpu_t* cpu);
static void decode(cpu_t* cpu);
static void memory1(cpu_t* cpu);
static void memory2(cpu_t* cpu);
static void execute(cpu_t* cpu);
//static void write_back(cpu_t* cpu);

typedef void (*pipeline_stage_t)(cpu_t*);
pipeline_stage_t pipeline_stages[] = { &interrupt, &fetch1, &fetch2, &decode, &memory1, &memory2, &execute};


static uint32_t* get_source_reg1(cpu_t* cpu);
static uint32_t* get_source_reg2(cpu_t* cpu);
static uint32_t* get_destination_reg1(cpu_t* cpu);
static uint32_t* get_destination_reg2(cpu_t* cpu);
static bool get_immediate_mode_flag(cpu_t* cpu);
static uint32_t get_ALU_immediate_bits(cpu_t* cpu);
static uint32_t get_opcode(cpu_t* cpu);
static cpu_op get_instruction(cpu_t* cpu);
static uint32_t get_pc_relative_offset(cpu_t* cpu);
static uint32_t* get_base_reg(cpu_t* cpu);
static uint8_t get_condition_code_bits(cpu_t* cpu);


//extracts the encoded condition code bits from the instruction
static uint8_t get_condition_code_bits(cpu_t* cpu)
{
    return GET_BITS_IN_RANGE(cpu->IR, 23 ,25);
}


//as the name implies, it sign-extends the given value, of the given width, to 32-bits
static uint32_t sign_extend(uint32_t original_value, uint8_t original_value_width)
{
    uint32_t result = original_value;
    uint32_t MASK = (0xFFFFFFFF);
    if(original_value >> (original_value_width -1))
    {
        MASK = MASK - ((1 << original_value_width) -1);
        result = MASK | result;
    }
    return result;
}

static uint32_t sign_extend_ALU_immediate_bits(uint32_t ALU_bits)
{
    return sign_extend(ALU_bits, 15);
}

//sign-extend the 26-bit pc-relative offset for jump instructions
static uint32_t sign_extend_jump_pc_relative_offset(uint32_t pc_relative_offset)
{
    return sign_extend(pc_relative_offset, 26);
}

//sign-extend the 21-bit pc-relative offset for load/store instructions
static uint32_t sign_extend_pc_relative_offset(uint32_t pc_relative_offset)
{
    return sign_extend(pc_relative_offset, 21);
}

//sign-extend the 16-bit offset for base + register type instructions
static uint32_t sign_extend_base_offset(uint32_t base_register_offset)
{
    return sign_extend(base_register_offset, 16);
}

//sign-extend the 23-bit pc-relative offset for branches
static uint32_t sign_extend_branch_pc_relative_offset(uint32_t pc_relative_offset)
{
    return sign_extend(pc_relative_offset, 23);
}

static void install_opcodes(cpu_t* cpu)
{
    cpu->opcodes = get_instruction_table();
}

static uint32_t get_opcode(cpu_t* cpu)
{
    return GET_BITS_IN_RANGE(cpu->IR, 26, 31);
}

#warning FIXME!! the source and destination register encodings in the instructions might prove problematic when we add multiplication! Think about how to fix this!
static uint32_t* get_source_reg1(cpu_t* cpu)
{
    uint32_t reg_name = GET_BITS_IN_RANGE(cpu->IR, 16, 20);
    return &cpu->registers[reg_name];
}

static uint32_t* get_source_reg2(cpu_t* cpu)
{
    uint32_t reg_name = GET_BITS_IN_RANGE(cpu->IR, 11, 15);
    return &cpu->registers[reg_name];
}

static uint32_t* get_destination_reg1(cpu_t* cpu)
{
    uint32_t reg_name = GET_BITS_IN_RANGE(cpu->IR, 21, 25);
    return &cpu->registers[reg_name];
}

static uint32_t* get_destination_reg2(cpu_t* cpu)
{
    //FIXME: this destination register 2 encoding is now broken! figure out what I want to do about instruction encoding and multiplication
    uint32_t reg_name = GET_BITS_IN_RANGE(cpu->IR, 6, 10);
    return &cpu->registers[reg_name];
}

//gets the register to store from for store-type instructions
static uint32_t* get_store_source_reg(cpu_t* cpu)
{
    uint32_t reg_name = GET_BITS_IN_RANGE(cpu->IR, 21, 25);
    return &cpu->registers[reg_name];
}

static bool get_immediate_mode_flag(cpu_t* cpu)
{
    //the last bit of the instruction is the immediate mode flag
    const uint8_t IMMEDIATE_MODE_BIT = 0;
    return CHECK_BIT_SET(cpu->IR, IMMEDIATE_MODE_BIT);
}

//gets the 15-bit immediate mode operand from the instruction for ALU operations
static uint32_t get_ALU_immediate_bits(cpu_t* cpu)
{
    return GET_BITS_IN_RANGE(cpu->IR, 1, 15);
}

//gets the 21-bit pc-relative offset encoded in the load/store instruction
static uint32_t get_pc_relative_offset(cpu_t* cpu)
{
    return GET_BITS_IN_RANGE(cpu->IR, 0, 20);
}

static uint32_t* get_base_reg(cpu_t* cpu)
{
    uint32_t reg_name = GET_BITS_IN_RANGE(cpu->IR, 16, 20);
    return &cpu->registers[reg_name];
}

//get the 16-bit base-register offset for base_reg + offset style instructions
static uint32_t get_base_register_offset(cpu_t* cpu)
{
    return GET_BITS_IN_RANGE(cpu->IR, 0, 15);
}

//get the 26-bit pc-relative offset for the jump instruction
static uint32_t get_jump_pc_offset(cpu_t* cpu)
{
    return GET_BITS_IN_RANGE(cpu->IR, 0, 25);
}

static uint32_t get_branch_pc_offset(cpu_t* cpu)
{
    return GET_BITS_IN_RANGE(cpu->IR, 0, 22);
}

static void interrupt(cpu_t* cpu)
{
    if(interrupt_requested(cpu->ic) && !interrupt_in_process(cpu))
    {
        set_interrupt_in_process_status(cpu, true);
        beacon();
        exit(-1);
        backup_cpu = *cpu;
        cpu->PC = get_interrupt_vector_table_starting_address(cpu->ic) + get_interrupt_source(cpu->ic);
    }

    pipeline_stage = FETCH1;
}

static void fetch1(cpu_t* cpu)
{
    pipeline_stage = FETCH2;
    cpu->MAR = cpu->PC;
    update_pc(cpu);
    bus_enable(cpu->bus);
    bus_set_address_lines(cpu->bus, cpu->MAR);
    bus_set_read_operation(cpu->bus);
}

static void fetch2(cpu_t* cpu)
{
    if(bus_is_device_ready(cpu->bus))
    {
        pipeline_stage = DECODE;
        cpu->MDR = bus_get_data_lines(cpu->bus);
        cpu->IR = cpu->MDR;
        bus_clear_device_ready(cpu->bus);
        bus_disable(cpu->bus);
    }
    else
    {
        pipeline_stage = FETCH2;
    }

}

static void decode(cpu_t* cpu)
{
    cpu->opcode = get_opcode(cpu);
    //we don't know what kind of instruction we have yet, but we can speculatively 
    //gather other information like source registers and what not because during
    //execution, the instructions will only reference what they need, effectively discarding the
    //garbage information in the other registers
    cpu->source_reg1 = get_source_reg1(cpu);
    cpu->source_reg2 = get_source_reg2(cpu);
    cpu->destination_reg1 = get_destination_reg1(cpu);
    cpu->destination_reg2 = get_destination_reg2(cpu);
    cpu->store_source_reg = get_store_source_reg(cpu);
    cpu->immediate_mode = get_immediate_mode_flag(cpu);
    cpu->instruction_condition_codes = get_condition_code_bits(cpu);
    cpu->ALU_immediate_bits = sign_extend_ALU_immediate_bits(get_ALU_immediate_bits(cpu));
    cpu->load_pc_relative_offset_bits = sign_extend_pc_relative_offset(get_pc_relative_offset(cpu));
    cpu->base_reg = get_base_reg(cpu);
    cpu->base_register_offset_bits = sign_extend_base_offset(get_base_register_offset(cpu));
    cpu->jump_pc_relative_offset_bits = sign_extend_jump_pc_relative_offset(get_jump_pc_offset(cpu));
    cpu->branch_pc_relative_offset_bits = sign_extend_branch_pc_relative_offset(get_branch_pc_offset(cpu));
    //load/store instructions get special treatment in our FSM
    if(is_memory_instruction(cpu->opcode))
    {
        if(is_load_effective_address_instruction(cpu->opcode))
        {
            //This instruction doesn't actually access memory, it just loads an
            //address into a register, so skip the memory accesses
            pipeline_stage = EXECUTE;
        }
        else
        {
            pipeline_stage = MEMORY1;
        }
    }
    else
    {
        pipeline_stage = EXECUTE;
    }
}

static void memory1(cpu_t* cpu)
{
    pipeline_stage = MEMORY2;
    if(is_pc_relative_instruction(cpu->opcode))
    {
        cpu->MAR = cpu->PC + cpu->load_pc_relative_offset_bits;
    }
    else //base register + offset
    {
        cpu->MAR = *cpu->base_reg + cpu->base_register_offset_bits;
    }

    bus_enable(cpu->bus);
    bus_set_address_lines(cpu->bus, cpu->MAR);
    if(is_load_instruction(cpu->opcode))
    {
        bus_set_read_operation(cpu->bus);
    }
    else //store instruction
    {
        bus_set_write_operation(cpu->bus);
        cpu->MDR = *cpu->store_source_reg;
        bus_set_data_lines(cpu->bus, cpu->MDR);
    }
}

static void memory2(cpu_t* cpu)
{
    if(bus_is_device_ready(cpu->bus))
    {
        bus_clear_device_ready(cpu->bus);
        bus_disable(cpu->bus);

        if(is_load_instruction(cpu->opcode))
        {
            cpu->MDR = bus_get_data_lines(cpu->bus);
            pipeline_stage = EXECUTE;
        }
        else
        {
            //store instructions don't really have anything to execute, they
            //are purely memory access commands, so we can go back to fetch
            //instead of executing nothing
            pipeline_stage = INTERRUPT;
        }
    }
    else
    {
        pipeline_stage = MEMORY2;
    }
}

static void execute(cpu_t* cpu)
{
    cpu_op instruction = get_instruction(cpu);
    instruction(cpu);
    pipeline_stage = INTERRUPT;
}

static cpu_op get_instruction(cpu_t* cpu)
{
    return (*cpu->opcodes)[cpu->opcode];
}

#if 0
static void write_back(cpu_t* cpu)
{

}
#endif

static void update_pc(cpu_t* cpu)
{
    cpu->PC += 1; //updates 1 word at a time, but each word is 32-bits
}

//This just makes the raw cpu object, it doesn't do any of the work to "build" it
//FIXME: add cache parameter later
cpu_t* make_cpu(memory_bus_t* bus, interrupt_controller_t* ic)
{
    cpu_t* new_cpu = calloc(1, sizeof(struct cpu));
    new_cpu->bus = bus;
    new_cpu->ic = ic;
    return new_cpu;
}

void cpu_reset(cpu_t* cpu)
{
    const uint32_t INITIAL_ADDRESS = 0x00;
    const uint32_t INITIAL_VALUE = 0x00;
    memset(cpu->registers, INITIAL_VALUE, sizeof(cpu->registers));
    cpu->PC = INITIAL_ADDRESS;
    cpu->CCR = INITIAL_VALUE;
    cpu->IR = INITIAL_VALUE;
    cpu->MDR = INITIAL_VALUE;
    cpu->MAR = INITIAL_ADDRESS;
    cpu->opcode = INITIAL_VALUE;
    cpu->source_reg1 = NULL;
    cpu->source_reg2 = NULL;
    cpu->destination_reg1 = NULL;
    cpu->destination_reg2 = NULL;
    cpu->immediate_mode = false;
    cpu->ALU_immediate_bits = INITIAL_VALUE;
}

void init_cpu(cpu_t* cpu)
{
    cpu_reset(cpu);
    install_opcodes(cpu);
}

void destroy_cpu(cpu_t* cpu)
{
    free(cpu);
}

void dump_cpu_state(cpu_t* cpu)
{
    printf("\n\n**** CPU STATE ****\n\n");
    printf("PC = 0x%08X \t IR = 0x%08X\n", cpu->PC, cpu->IR);
    printf("CCR = 0x%08X\n", cpu->CCR);
    printf("MDR = 0x%08X \t MAR = 0x%08X\n\n", cpu->MDR, cpu->MAR);

    for (int i = 0; i < NUM_REGISTERS; i+=2)
    {
        printf("R%02d = 0x%08X \t R%02d = 0x%08X\n", i, cpu->registers[i], i+1, cpu->registers[i+1]);
    }
}

void cpu_cycle(cpu_t* cpu)
{
    pipeline_stage_t stage = pipeline_stages[pipeline_stage];
    stage(cpu);

    //if we just finished executing then we've completed the instruction 
    //(FIXME: this won't be true when we add the memory write stage)
    if(stage == execute)
        cpu->instruction_finished = true;
    else
    {
        cpu->instruction_finished = false;
    }

}

bool cpu_completed_instruction(cpu_t* cpu)
{
    return cpu->instruction_finished;
}
