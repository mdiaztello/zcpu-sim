

//This file defines the cpu module user-facing functions as well
//as any additional functions necessary to implement the CPU internals (except
//for implementing the instructions, which reside in another file).


#include "debug.h"
#include "cpu.h"
#include "cpu_private.h"
#include "cpu_ops.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>



enum cpu_pipeline_stage_t { FETCH1, FETCH2, DECODE, MEMORY1, MEMORY2, EXECUTE };

enum cpu_pipeline_stage_t pipeline_stage = FETCH1;

static void install_opcodes(cpu_t* cpu);
static void update_pc(cpu_t* cpu);
static void fetch1(cpu_t* cpu);
static void fetch2(cpu_t* cpu);
static void decode(cpu_t* cpu);
static void memory1(cpu_t* cpu);
static void memory2(cpu_t* cpu);
static void execute(cpu_t* cpu);
//static void write_back(cpu_t* cpu);

typedef void (*pipeline_stage_t)(cpu_t*);
pipeline_stage_t pipeline_stages[7] = { &fetch1, &fetch2, &decode, &memory1, &memory2, &execute};


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

static uint32_t sign_extend_pc_relative_offset(uint32_t pc_relative_offset)
{
    uint32_t result = pc_relative_offset;
    if(pc_relative_offset >> 20)
    {
        result |= 0xFFE00000;
    }
    return result;
}

static uint32_t sign_extend_base_offset(uint32_t base_register_offset)
{
    uint32_t result = base_register_offset;
    if(base_register_offset >> 15)
    {
        result |= 0xFFFF0000;
    }
    return result;
}

static void install_opcodes(cpu_t* cpu)
{
    cpu->opcodes = get_instruction_table();
}

static uint32_t get_opcode(cpu_t* cpu)
{
    return (cpu->IR & 0xFC000000) >> 26;
}

static uint32_t* get_source_reg1(cpu_t* cpu)
{
    uint32_t reg_name = (cpu->IR & 0x000001E0) >> 5;
    return &cpu->registers[reg_name];
}

static uint32_t* get_source_reg2(cpu_t* cpu)
{
    uint32_t reg_name = (cpu->IR & 0x0000001F);
    return &cpu->registers[reg_name];
}

static uint32_t* get_destination_reg1(cpu_t* cpu)
{
    uint32_t reg_name = (cpu->IR & (0x1F << 21)) >> 21;
    return &cpu->registers[reg_name];
}

static uint32_t* get_destination_reg2(cpu_t* cpu)
{
    uint32_t reg_name = (cpu->IR & (0x1F << 6)) >> 6;
    return &cpu->registers[reg_name];
}

//gets the register to store from for store-type instructions
static uint32_t* get_store_source_reg(cpu_t* cpu)
{
    uint32_t reg_name = (cpu->IR & (0x1F << 21)) >> 21;
    return &cpu->registers[reg_name];
}

static bool get_immediate_mode_flag(cpu_t* cpu)
{
    return (cpu->IR & 0x00000001); //the last bit of the instruction is the immediate mode flag
}

//gets the 15-bit immediate mode operand from the instruction for ALU operations
static uint32_t get_ALU_immediate_bits(cpu_t* cpu)
{
    return (cpu->IR & 0x0000FFFF) >> 1;
}

//gets the 21-bit pc-relative offset encoded in the load/store instruction
static uint32_t get_pc_relative_offset(cpu_t* cpu)
{
    return (cpu->IR & 0x001FFFFF);
}

static uint32_t* get_base_reg(cpu_t* cpu)
{
    uint32_t reg_name = (cpu->IR & (0x1F << 16) ) >> 16;
    return &cpu->registers[reg_name];
}

//get the 16-bit base-register offset for base_reg + offset style instructions
static uint32_t get_base_register_offset(cpu_t* cpu)
{
    return (cpu->IR & 0x0000FFFF);
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
    cpu->ALU_immediate_bits = get_ALU_immediate_bits(cpu);
    cpu->pc_relative_offset_bits = get_pc_relative_offset(cpu);
    cpu->base_reg = get_base_reg(cpu);
    cpu->base_register_offset_bits = get_base_register_offset(cpu);
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
    //implementing loads first
    //FIXME This stuff probably won't work for storing
    pipeline_stage = MEMORY2;
    if(is_pc_relative_instruction(cpu->opcode))
    {
        //FIXME: sign extend offset to handle negative offsets?
        cpu->MAR = cpu->PC + sign_extend_pc_relative_offset(cpu->pc_relative_offset_bits);
        printf("the PC is %08x, the offset is %08x, the MAR is %08x\n", cpu->PC, sign_extend_pc_relative_offset(cpu->pc_relative_offset_bits), cpu->MAR);
    }
    else //base register + offset
    {
        //FIXME: sign extend offset to handle negative offsets?
        cpu->MAR = *cpu->base_reg + sign_extend_base_offset(cpu->base_register_offset_bits);
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
            printf("the mdr is %08x\n", cpu->MDR);
            pipeline_stage = EXECUTE;
        }
        else
        {
            //store instructions don't really have anything to execute, they
            //are purely memory access commands, so we can go back to fetch
            //instead of executing nothing
            pipeline_stage = FETCH1;
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
    pipeline_stage = FETCH1;
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
cpu_t* make_cpu(memory_bus_t* bus)
{
    cpu_t* new_cpu = calloc(1, sizeof(struct cpu));
    new_cpu->bus = bus;
    return new_cpu;
}

void cpu_reset(cpu_t* cpu)
{
    const uint32_t INITIAL_ADDRESS = 0x00;
    const uint32_t INITIAL_VALUE = 0x00;
    memset(cpu->registers, 0x00, sizeof(cpu->registers));
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

    printf("the PC is now %08x\n", cpu->PC);
}

bool cpu_completed_instruction(cpu_t* cpu)
{
    return cpu->instruction_finished;
}
