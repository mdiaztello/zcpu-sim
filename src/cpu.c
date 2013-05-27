
#include "cpu.h"
#include "memory.h"
#include <string.h>







struct cpu
{
    uint32_t registers[32]; //the general purpose registers for the processor
    uint32_t PC;            //the program counter of the processor
    uint32_t CCR;           //condition codes register
    uint32_t IR;            //instruction register
    uint32_t MDR;           //memory data register
    uint32_t MAR;           //memory address register
    memory_t* RAM;          //pointer to our memory interface
    //pointer to table of function pointers representing the opcodes goes here
};


static void install_memory(cpu_t* cpu, memory_t* RAM);
static void install_opcodes(cpu_t* cpu);
static void update_pc(cpu_t* cpu);
static void fetch(cpu_t* cpu);
static void decode(cpu_t* cpu);
static void execute(cpu_t* cpu);
static void write_back(cpu_t* cpu);




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

}

static void execute(cpu_t* cpu)
{

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
}

void cpu_cycle(cpu_t* cpu)
{
    fetch(cpu);
    decode(cpu);
    execute(cpu);
    //write_back(cpu);

}



