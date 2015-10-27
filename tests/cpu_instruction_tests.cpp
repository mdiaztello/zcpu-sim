
#include "CppUTest/TestHarness.h"

extern "C"
{
#include <stdio.h>
#include "preprocessor_assembler.h"
#include "memory_bus.h"
#include "cpu.h"
#include "cpu_private.h"
}

//This file contains tests that will demonstrate that the CPU is executing
//instructions correctly

TEST_GROUP(CPU_INSTRUCTION_TESTS)
{
    void setup(void)
    {
    }

    void teardown(void)
    {
    }
};

#if 0
cpu_t* build_cpu(memory_bus_t* bus, interrupt_controller_t* ic)
{
    cpu_t* cpu = make_cpu(bus, ic);
    init_cpu(cpu);

    return cpu;
}
#endif

TEST(CPU_INSTRUCTION_TESTS, first_test)
{
#if 0
    memory_bus_t* bus  = NULL;
    interrupt_controller_t* ic = NULL;
    cpu_t* cpu = build_cpu(bus, ic);
#endif
    CHECK(false);
}
