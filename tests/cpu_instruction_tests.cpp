
#include "CppUTest/TestHarness.h"

extern "C"
{
#include <stdio.h>
#include "preprocessor_assembler.h"
#include "memory_bus.h"
#include "interrupt_controller.h"
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

/* ******* STRUCTS FOR MOCKING PURPOSES ******** */
struct memory_bus_t
{
    uint32_t address_lines;
    uint32_t data_lines;
    bus_mode_t bus_mode; //for either read or write ops
    selected_device_t selected_device;
    bool device_ready; //report back the status of devices that take multiple bus cycles
    bool bus_active;
};

/* ******* END OF STRUCTS FOR MOCKING PURPOSES ******** */

cpu_t* build_cpu(memory_bus_t* bus, interrupt_controller_t* ic)
{
    cpu_t* cpu = make_cpu(bus, ic);
    init_cpu(cpu);

    return cpu;
}

//sets the value on the bus to the expected instruction to help fake out the CPU
void set_expected_instruction(memory_bus_t* fake_bus, uint32_t instruction)
{
    fake_bus->data_lines = instruction;
    //make the CPU think that memory is ready and has returned the fetched instruction
    fake_bus->device_ready = true;
}

void single_step(cpu_t* cpu, memory_bus_t* fake_bus)
{
    while(!cpu_completed_instruction(cpu))
    {
        cpu_cycle(cpu);
        bus_cycle(fake_bus);
    }
}

void set_register_value(cpu_t* cpu, uint8_t register_name, uint32_t value)
{
    cpu->registers[register_name] = value;
}

uint32_t get_register_value(cpu_t* cpu, uint8_t register_name)
{
    return cpu->registers[register_name];
}

TEST(CPU_INSTRUCTION_TESTS, cpu_test_helper_functions_working_correctly)
{
    memory_bus_t mock_bus;
    interrupt_controller_t* ic = make_interrupt_controller();
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint32_t instruction = 0xAAAAAAAA; //not a real instruction encoding

    set_register_value(cpu, R0, (uint32_t)(~0));
    LONGS_EQUAL(0xFFFFFFFF, get_register_value(cpu, R0));
    set_register_value(cpu, R1, 0x00);
    LONGS_EQUAL(0x00000000, get_register_value(cpu, R1));

    //There are no peripherals to request interrupts, so the interrupt request
    //status had better be false
    CHECK(interrupt_requested(ic) == false);

    set_expected_instruction(&mock_bus, instruction);
    CHECK(mock_bus.data_lines == instruction);
    CHECK(mock_bus.device_ready == true);
}

//TESTS FOR ANDING TWO REGISTERS AND WRITING OUTPUT TO A THIRD REG
//FIXME: All of these ALU instruction tests also need to verify the condition code bits

TEST(CPU_INSTRUCTION_TESTS, AND_with_register_value_of_zero_yields_zero)
{
    memory_bus_t mock_bus;
    interrupt_controller_t* ic = make_interrupt_controller();
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    set_register_value(cpu, R0, (uint32_t)(~0));
    set_register_value(cpu, R1, 0x00);

    const uint32_t EXPECTED_VALUE = 0x00;
    const uint32_t instruction = (AND(R0, R0, R1));

    set_expected_instruction(&mock_bus, instruction);

    single_step(cpu, &mock_bus);

    LONGS_EQUAL(EXPECTED_VALUE, get_register_value(cpu, R0));
}

TEST(CPU_INSTRUCTION_TESTS, AND_with_register_value_of_all_ones_does_not_change_other_register)
{
    memory_bus_t mock_bus;
    interrupt_controller_t* ic = make_interrupt_controller();
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    set_register_value(cpu, R0, (uint32_t)(~0));
    set_register_value(cpu, R1, 0x01234567);

    const uint32_t EXPECTED_VALUE = 0x01234567;
    const uint32_t instruction = (AND(R0, R0, R1));

    set_expected_instruction(&mock_bus, instruction);

    single_step(cpu, &mock_bus);

    LONGS_EQUAL(EXPECTED_VALUE, get_register_value(cpu, R0));
}

TEST(CPU_INSTRUCTION_TESTS, AND_with_bitmask_clears_the_desired_bits)
{
    memory_bus_t mock_bus;
    interrupt_controller_t* ic = make_interrupt_controller();
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    set_register_value(cpu, R0, (uint32_t)(~0));
    set_register_value(cpu, R1, 0xFFFF0FFF);

    const uint32_t EXPECTED_VALUE = 0xFFFF0FFF;
    const uint32_t instruction = (AND(R0, R0, R1));

    set_expected_instruction(&mock_bus, instruction);

    single_step(cpu, &mock_bus);

    LONGS_EQUAL(EXPECTED_VALUE, get_register_value(cpu, R0));
}

TEST(CPU_INSTRUCTION_TESTS, ANDING_a_register_with_itself_changes_nothing)
{
    memory_bus_t mock_bus;
    interrupt_controller_t* ic = make_interrupt_controller();
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    set_register_value(cpu, R0, 0xFEDCBA98);

    const uint32_t EXPECTED_VALUE = 0xFEDCBA98;
    const uint32_t instruction = (AND(R0, R0, R0));

    set_expected_instruction(&mock_bus, instruction);

    single_step(cpu, &mock_bus);

    LONGS_EQUAL(EXPECTED_VALUE, get_register_value(cpu, R0));
}

TEST(CPU_INSTRUCTION_TESTS, result_of_ANDING_two_registers_can_be_written_to_any_register)
{
    memory_bus_t mock_bus;
    interrupt_controller_t* ic = make_interrupt_controller();
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    set_register_value(cpu, R0, 0xA5A5A5A5);
    set_register_value(cpu, R1, 0x5A5A5A5A);

    const uint32_t EXPECTED_VALUE = 0;
    const uint32_t instruction = (AND(R31, R0, R1));

    set_expected_instruction(&mock_bus, instruction);

    single_step(cpu, &mock_bus);

    LONGS_EQUAL(EXPECTED_VALUE, get_register_value(cpu, R31));
}
