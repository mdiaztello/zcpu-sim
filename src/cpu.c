

//This file defines the cpu module user-facing functions as well
//as any additional functions necessary to implement the CPU internals (except
//for implementing the instructions, which reside in another file).


#include "cpu.h"
#include "cpu_private.h"
#include "cpu_ops.h"
#include "memory.h"
#include <string.h>
#include <stdlib.h>

#define NUM_MEM_LOCATIONS 1024



static void install_memory(cpu_t* cpu, memory_t* RAM);
static void install_opcodes(cpu_t* cpu);
static void update_pc(cpu_t* cpu);
static void fetch(cpu_t* cpu);
static void decode(cpu_t* cpu);
static void execute(cpu_t* cpu);
//static void write_back(cpu_t* cpu);

static uint32_t* get_source_reg1(cpu_t* cpu);
static uint32_t* get_source_reg2(cpu_t* cpu);
static uint32_t* get_destination_reg1(cpu_t* cpu);
static uint32_t* get_destination_reg2(cpu_t* cpu);
static bool get_immediate_mode_flag(cpu_t* cpu);
static uint32_t get_ALU_immediate_bits(cpu_t* cpu);
static uint32_t get_opcode(cpu_t* cpu);
static cpu_op get_instruction(cpu_t* cpu);

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

static bool get_immediate_mode_flag(cpu_t* cpu)
{
    return (cpu->IR & 0x00000001); //the last bit of the instruction is the immediate mode flag
}

//gets the 15-bit immediate mode operand from the instruction for ALU operations
static uint32_t get_ALU_immediate_bits(cpu_t* cpu)
{
    return (cpu->IR & 0x0000FFFF) >> 1;
}

static void install_memory(cpu_t* cpu, memory_t* RAM)
{
    cpu->RAM = RAM;
}

static void fetch(cpu_t* cpu)
{
    cpu->MAR = cpu->PC;
    cpu->MDR = memory_get(cpu->RAM, cpu->MAR);
    cpu->IR = cpu->MDR;
    update_pc(cpu);
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
    cpu->immediate_mode = get_immediate_mode_flag(cpu);
    cpu->ALU_immediate_bits = get_ALU_immediate_bits(cpu);


}

static void execute(cpu_t* cpu)
{
    cpu_op instruction = get_instruction(cpu);
    instruction(cpu);
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



cpu_t* make_cpu(void)
{
    cpu_t* new_cpu = calloc(1, sizeof(struct cpu));
    return new_cpu;
}

void init_cpu(cpu_t* cpu)
{
    const uint32_t INITIAL_ADDRESS = 0x00;
    const uint32_t INITIAL_VALUE = 0x00;
    memset(cpu, 0x00, sizeof(struct cpu));
    cpu->PC = INITIAL_ADDRESS;
    cpu->CCR = INITIAL_VALUE;
    cpu->IR = INITIAL_VALUE;
    cpu->MDR = INITIAL_VALUE;
    cpu->MAR = INITIAL_VALUE;
    install_memory(cpu, make_memory(NUM_MEM_LOCATIONS));
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

void dump_memory(cpu_t* cpu, size_t starting_address, size_t ending_address)
{
    memory_print(cpu->RAM, starting_address, ending_address);
}


void cpu_cycle(cpu_t* cpu)
{
    fetch(cpu);
    decode(cpu);
    execute(cpu);
    //write_back(cpu);

    //dump_cpu_state(cpu);

}


void cpu_load_program(cpu_t* cpu, uint32_t program[], size_t program_length)
{

    for(size_t i = 0; i < program_length; i++)
    {
        memory_set(cpu->RAM, i, program[i]);
    }

}
