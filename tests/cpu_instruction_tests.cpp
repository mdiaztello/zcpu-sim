
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
    interrupt_controller_t* ic;

    void setup(void)
    {
        // none of these tests are testing the interrupt abilities of the
        // processor, so pass in a bogus address
        const uint32_t BOGUS_INTERRUPT_VECTOR_TABLE_STARTING_ADDRESS = 0x00;
        ic = make_interrupt_controller(BOGUS_INTERRUPT_VECTOR_TABLE_STARTING_ADDRESS);
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

void test_single_instruction( cpu_t* cpu_to_test,
                              memory_bus_t* mock_bus,
                              uint32_t instruction_to_execute,
                              uint8_t destination_reg_name,
                              uint8_t source_reg1_name,
                              uint8_t source_reg2_name,
                              uint32_t source_reg1_value,
                              uint32_t source_reg2_value,
                              uint32_t expected_test_value )
{
    set_register_value(cpu_to_test, source_reg1_name, source_reg1_value);
    set_register_value(cpu_to_test, source_reg2_name, source_reg2_value);

    set_expected_instruction(mock_bus, instruction_to_execute);

    single_step(cpu_to_test, mock_bus);

    LONGS_EQUAL(expected_test_value, get_register_value(cpu_to_test, destination_reg_name));
}

//TESTS FOR ANDING TWO REGISTERS AND WRITING OUTPUT TO A THIRD REG
//FIXME: All of these ALU instruction tests also need to verify the condition code bits

const uint32_t ALL_ONES = ((uint32_t)(~0));

TEST(CPU_INSTRUCTION_TESTS, AND_with_register_value_of_zero_yields_zero)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = ALL_ONES;
    const uint32_t SOURCE_REG2_VALUE = 0x00;
    const uint32_t INSTRUCTION_TO_EXECUTE = (AND(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE & SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, AND_with_register_value_of_all_ones_yields_a_result_identical_to_the_value_of_the_other_register)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = ALL_ONES;
    const uint32_t SOURCE_REG2_VALUE = 0x01234567;
    const uint32_t INSTRUCTION_TO_EXECUTE = (AND(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE & SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, AND_with_bitmask_clears_the_desired_bits)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = ALL_ONES;
    const uint32_t SOURCE_REG2_VALUE = 0xFFFF0FFF;
    const uint32_t INSTRUCTION_TO_EXECUTE = (AND(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE & SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, ANDING_a_register_with_itself_changes_nothing)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R0;
    const uint32_t SOURCE_REG1_VALUE = 0xFEDCBA98;
    const uint32_t SOURCE_REG2_VALUE = 0xFEDCBA98;
    const uint32_t INSTRUCTION_TO_EXECUTE = (AND(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE & SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, result_of_ANDING_two_registers_can_be_written_to_any_register)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R31;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = 0xA5A5A5A5;
    const uint32_t SOURCE_REG2_VALUE = 0x5A5A5A5A;
    const uint32_t INSTRUCTION_TO_EXECUTE = (AND(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE & SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

//TESTS FOR ORING TWO REGISTERS AND WRITING OUTPUT TO A THIRD REG
//FIXME: All of these ALU instruction tests also need to verify the condition code bits

TEST(CPU_INSTRUCTION_TESTS, OR_with_register_value_of_zero_yields_a_result_identical_to_the_value_of_the_other_register)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = 0x00;
    const uint32_t SOURCE_REG2_VALUE = 0x01234567;
    const uint32_t INSTRUCTION_TO_EXECUTE = (OR(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE | SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, OR_with_register_value_of_all_ones_yields_all_ones)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = ALL_ONES;
    const uint32_t SOURCE_REG2_VALUE = 0x00;
    const uint32_t INSTRUCTION_TO_EXECUTE = (OR(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE | SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, OR_with_bitmask_sets_the_desired_bits)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = 0x00;
    const uint32_t SOURCE_REG2_VALUE = 0x0000F000;
    const uint32_t INSTRUCTION_TO_EXECUTE = (OR(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE | SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, ORING_a_register_with_itself_changes_nothing)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R0;
    const uint32_t SOURCE_REG1_VALUE = 0xFEDCBA98;
    const uint32_t SOURCE_REG2_VALUE = 0xFEDCBA98;
    const uint32_t INSTRUCTION_TO_EXECUTE = (OR(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE | SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, result_of_ORING_two_registers_can_be_written_to_any_register)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R31;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = 0xA5A5A5A5;
    const uint32_t SOURCE_REG2_VALUE = 0x5A5A5A5A;
    const uint32_t INSTRUCTION_TO_EXECUTE = (OR(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE | SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}


//TESTS FOR INVERTING THE CONTENTS OF A REGISTER AND WRITING TO A DIFFERENT REGISTER
//FIXME: All of these ALU instruction tests also need to verify the condition code bits

TEST(CPU_INSTRUCTION_TESTS, INVERTING_a_register_with_value_zero_yields_all_ones)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t VALUE_UNUSED = 77;
    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = VALUE_UNUSED;
    const uint32_t SOURCE_REG1_VALUE = 0x00;
    const uint32_t SOURCE_REG2_VALUE = VALUE_UNUSED;
    const uint32_t INSTRUCTION_TO_EXECUTE = (NOT(DEST_REG_NAME, SOURCE_REG1_NAME));
    const uint32_t EXPECTED_TEST_VALUE = ALL_ONES;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, INVERTING_a_register_with_value_all_ones_yields_zero)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t VALUE_UNUSED = 77;
    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = VALUE_UNUSED;
    const uint32_t SOURCE_REG1_VALUE = ALL_ONES;
    const uint32_t SOURCE_REG2_VALUE = VALUE_UNUSED;
    const uint32_t INSTRUCTION_TO_EXECUTE = (NOT(DEST_REG_NAME, SOURCE_REG1_NAME));
    const uint32_t EXPECTED_TEST_VALUE = 0x00;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, INVERTING_works_with_arbitrary_source_and_destination_registers)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t VALUE_UNUSED = 77;
    const uint8_t DEST_REG_NAME = R17;
    const uint8_t SOURCE_REG1_NAME = R31;
    const uint8_t SOURCE_REG2_NAME = VALUE_UNUSED;
    const uint32_t SOURCE_REG1_VALUE = 0xA0B0C0D0;
    const uint32_t SOURCE_REG2_VALUE = VALUE_UNUSED;
    const uint32_t INSTRUCTION_TO_EXECUTE = (NOT(DEST_REG_NAME, SOURCE_REG1_NAME));
    const uint32_t EXPECTED_TEST_VALUE = ~0xA0B0C0D0;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}


//TESTS FOR XOR-ING THE CONTENTS OF TWO REGISTERS AND WRITING THE OUTPUT TO A THIRD REG
//FIXME: All of these ALU instruction tests also need to verify the condition code bits

TEST(CPU_INSTRUCTION_TESTS, XORING_a_register_with_a_value_of_zero_yields_a_result_identical_to_the_value_of_the_other_register)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = 0x00;
    const uint32_t SOURCE_REG2_VALUE = 0x01234567;
    const uint32_t INSTRUCTION_TO_EXECUTE = (XOR(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, XORING_a_register_with_a_value_of_all_ones_is_the_same_as_inverting_the_other_register)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = ALL_ONES;
    const uint32_t SOURCE_REG2_VALUE = 0x01234567;
    const uint32_t INSTRUCTION_TO_EXECUTE = (XOR(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = ~SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, XORING_works_correctly_with_arbitrary_values)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = 0x000FF000;
    const uint32_t SOURCE_REG2_VALUE = 0x01234567;
    const uint32_t INSTRUCTION_TO_EXECUTE = (XOR(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE ^ SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, XORING_works_with_arbitrary_source_and_destination_registers)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R17;
    const uint8_t SOURCE_REG1_NAME = R5;
    const uint8_t SOURCE_REG2_NAME = R29;
    const uint32_t SOURCE_REG1_VALUE = 0x000FF000;
    const uint32_t SOURCE_REG2_VALUE = 0x01234567;
    const uint32_t INSTRUCTION_TO_EXECUTE = (XOR(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE ^ SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

//TESTS FOR ADDING THE CONTENTS OF TWO REGISTERS AND WRITING THE OUTPUT TO A THIRD REG
//FIXME: All of these ALU instruction tests also need to verify the condition code bits

TEST(CPU_INSTRUCTION_TESTS, ADDING_zero_to_a_register_changes_nothing)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = 0x00;
    const uint32_t SOURCE_REG2_VALUE = 0x01234567;
    const uint32_t INSTRUCTION_TO_EXECUTE = (ADD(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG2_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, ADDING_two_values_gives_the_expected_result)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = 0x0EDCBA98;
    const uint32_t SOURCE_REG2_VALUE = 0x01234567;
    const uint32_t INSTRUCTION_TO_EXECUTE = (ADD(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = 0x0FFFFFFF;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, ADDING_one_to_a_value_of_all_ones_overflows_and_yields_zero)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R16;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R23;
    const uint32_t SOURCE_REG1_VALUE = 0xFFFFFFFF;
    const uint32_t SOURCE_REG2_VALUE = 0x01;
    const uint32_t INSTRUCTION_TO_EXECUTE = (ADD(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = 0x00000000;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, ADDING_negative_twos_complement_numbers_to_positive_values_works_properly)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R16;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R23;
    const uint32_t SOURCE_REG1_VALUE = 0xFFFFFFFF; //-1
    const uint32_t SOURCE_REG2_VALUE = 0x00000200;
    const uint32_t INSTRUCTION_TO_EXECUTE = (ADD(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = 0x000001FF; // -1 + 0x200 = 0x1FF

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, ADDING_a_pair_of_twos_complement_negative_numbers_works_correctly)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R16;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R23;
    const uint32_t SOURCE_REG1_VALUE = 0xFFFFFFFF; // -1
    const uint32_t SOURCE_REG2_VALUE = 0xFFFFFE00; // -0x200
    const uint32_t INSTRUCTION_TO_EXECUTE = (ADD(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = 0xFFFFFDFF;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}


//TESTS FOR SUBTRACTING THE CONTENTS OF TWO REGISTERS AND WRITING THE OUTPUT TO A THIRD REG
//FIXME: All of these ALU instruction tests also need to verify the condition code bits

TEST(CPU_INSTRUCTION_TESTS, SUBBING_zero_from_a_register_changes_nothing)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = 0x01234567;
    const uint32_t SOURCE_REG2_VALUE = 0x00;
    const uint32_t INSTRUCTION_TO_EXECUTE = (SUB(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1_VALUE;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, SUBBING_one_from_zero_yields_all_ones)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R0;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R1;
    const uint32_t SOURCE_REG1_VALUE = 0x00;
    const uint32_t SOURCE_REG2_VALUE = 0x01;
    const uint32_t INSTRUCTION_TO_EXECUTE = (SUB(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = ALL_ONES;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, SUBBING_a_register_from_itself_yields_zero)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R16;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R0;
    const uint32_t SOURCE_REG1_VALUE = 0xFFFFFFFF;
    const uint32_t SOURCE_REG2_VALUE = 0xFFFFFFFF;
    const uint32_t INSTRUCTION_TO_EXECUTE = (SUB(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = 0x00000000;

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, SUBBING_a_twos_complement_negative_number_from_a_positive_number_is_the_same_as_adding)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t DEST_REG_NAME = R16;
    const uint8_t SOURCE_REG1_NAME = R0;
    const uint8_t SOURCE_REG2_NAME = R23;
    const uint32_t SOURCE_REG1_VALUE = 0x00000200;
    const uint32_t SOURCE_REG2_VALUE = 0xFFFFFFFF; //-1
    const uint32_t INSTRUCTION_TO_EXECUTE = (SUB(DEST_REG_NAME, SOURCE_REG1_NAME, SOURCE_REG2_NAME));
    const uint32_t EXPECTED_TEST_VALUE = 0x00000201; // 0x200 - (-1) = 0x200

    test_single_instruction( cpu, &mock_bus,
                             INSTRUCTION_TO_EXECUTE,
                             DEST_REG_NAME,
                             SOURCE_REG1_NAME,
                             SOURCE_REG2_NAME,
                             SOURCE_REG1_VALUE,
                             SOURCE_REG2_VALUE,
                             EXPECTED_TEST_VALUE);
}


