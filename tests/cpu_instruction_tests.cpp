
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

struct zcpu_register_t
{
    uint8_t name;
    uint32_t value;
};

typedef struct zcpu_register_t zcpu_register_t;



void test_single_instruction2( cpu_t* cpu_to_test,
                              memory_bus_t* mock_bus,
                              const zcpu_register_t destination_register,
                              const zcpu_register_t source_register1,
                              const zcpu_register_t source_register2,
                              uint32_t instruction_to_execute,
                              uint32_t expected_test_value )
{
    set_register_value(cpu_to_test, source_register1.name, source_register1.value);
    set_register_value(cpu_to_test, source_register2.name, source_register2.value);

    set_expected_instruction(mock_bus, instruction_to_execute);

    single_step(cpu_to_test, mock_bus);

    LONGS_EQUAL(expected_test_value, get_register_value(cpu_to_test, destination_register.name));
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

//constant values for test data
const uint32_t INVALID_DATA = 0x44444444;
const uint32_t ALL_ONES = ((uint32_t)(~0));

TEST(CPU_INSTRUCTION_TESTS, AND_with_register_value_of_zero_yields_zero)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = ALL_ONES };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x00 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (AND(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value & SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, AND_with_register_value_of_all_ones_yields_a_result_identical_to_the_value_of_the_other_register)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = ALL_ONES };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x01234567 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (AND(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value & SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, AND_with_bitmask_clears_the_desired_bits)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = ALL_ONES };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0xFFFF0FFF };
    const uint32_t INSTRUCTION_TO_EXECUTE = (AND(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value & SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, ANDING_a_register_with_itself_changes_nothing)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0xFEDCBA98 };
    const zcpu_register_t SOURCE_REG2 = { .name = R0, .value = 0xFEDCBA98 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (AND(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value & SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, result_of_ANDING_two_registers_can_be_written_to_any_register)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R31, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0xA5A5A5A5 };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x5A5A5A5A };
    const uint32_t INSTRUCTION_TO_EXECUTE = (AND(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value & SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

//TESTS FOR ORING TWO REGISTERS AND WRITING OUTPUT TO A THIRD REG
//FIXME: All of these ALU instruction tests also need to verify the condition code bits

TEST(CPU_INSTRUCTION_TESTS, OR_with_register_value_of_zero_yields_a_result_identical_to_the_value_of_the_other_register)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0x00 };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x01234567 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (OR(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value | SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, OR_with_register_value_of_all_ones_yields_all_ones)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = ALL_ONES };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x00 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (OR(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value | SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, OR_with_bitmask_sets_the_desired_bits)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0x00 };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x0000F000 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (OR(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value | SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, ORING_a_register_with_itself_changes_nothing)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0xFEDCBA98 };
    const zcpu_register_t SOURCE_REG2 = { .name = R0, .value = 0xFEDCBA98 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (OR(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value | SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, result_of_ORING_two_registers_can_be_written_to_any_register)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R31, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0xA5A5A5A5 };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x5A5A5A5A };
    const uint32_t INSTRUCTION_TO_EXECUTE = (OR(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value | SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}


//TESTS FOR INVERTING THE CONTENTS OF A REGISTER AND WRITING TO A DIFFERENT REGISTER
//FIXME: All of these ALU instruction tests also need to verify the condition code bits

TEST(CPU_INSTRUCTION_TESTS, INVERTING_a_register_with_value_zero_yields_all_ones)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t VALUE_UNUSED = 77;
    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0x00 };
    const zcpu_register_t SOURCE_REG2 = { .name = VALUE_UNUSED, .value = VALUE_UNUSED };
    const uint32_t INSTRUCTION_TO_EXECUTE = (NOT(DEST_REG.name, SOURCE_REG1.name));
    const uint32_t EXPECTED_TEST_VALUE = ALL_ONES;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, INVERTING_a_register_with_value_all_ones_yields_zero)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t VALUE_UNUSED = 77;
    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = ALL_ONES };
    const zcpu_register_t SOURCE_REG2 = { .name = VALUE_UNUSED, .value = VALUE_UNUSED };
    const uint32_t INSTRUCTION_TO_EXECUTE = (NOT(DEST_REG.name, SOURCE_REG1.name));
    const uint32_t EXPECTED_TEST_VALUE = 0x00;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, INVERTING_works_with_arbitrary_source_and_destination_registers)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint8_t VALUE_UNUSED = 77;
    const zcpu_register_t DEST_REG    = { .name = R17, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R31, .value = 0xA0B0C0D0 };
    const zcpu_register_t SOURCE_REG2 = { .name = VALUE_UNUSED, .value = VALUE_UNUSED };
    const uint32_t INSTRUCTION_TO_EXECUTE = (NOT(DEST_REG.name, SOURCE_REG1.name));
    const uint32_t EXPECTED_TEST_VALUE = ~0xA0B0C0D0;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}


//TESTS FOR XOR-ING THE CONTENTS OF TWO REGISTERS AND WRITING THE OUTPUT TO A THIRD REG
//FIXME: All of these ALU instruction tests also need to verify the condition code bits

TEST(CPU_INSTRUCTION_TESTS, XORING_a_register_with_a_value_of_zero_yields_a_result_identical_to_the_value_of_the_other_register)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0x00 };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x01234567 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (XOR(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, XORING_a_register_with_a_value_of_all_ones_is_the_same_as_inverting_the_other_register)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = ALL_ONES };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x01234567 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (XOR(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = ~SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, XORING_works_correctly_with_arbitrary_values)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0x000FF000 };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x01234567 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (XOR(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value ^ SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, XORING_works_with_arbitrary_source_and_destination_registers)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R17, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R5, .value = 0x000FF000 };
    const zcpu_register_t SOURCE_REG2 = { .name = R29, .value = 0x01234567 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (XOR(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value ^ SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

//TESTS FOR ADDING THE CONTENTS OF TWO REGISTERS AND WRITING THE OUTPUT TO A THIRD REG
//FIXME: All of these ALU instruction tests also need to verify the condition code bits

TEST(CPU_INSTRUCTION_TESTS, ADDING_zero_to_a_register_changes_nothing)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0x00 };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x01234567 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (ADD(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG2.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, ADDING_two_values_gives_the_expected_result)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0x0EDCBA98 };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x01234567 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (ADD(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = 0x0FFFFFFF;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, ADDING_one_to_a_value_of_all_ones_overflows_and_yields_zero)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R16, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0xFFFFFFFF };
    const zcpu_register_t SOURCE_REG2 = { .name = R23, .value = 0x01 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (ADD(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = 0x00000000;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, ADDING_negative_twos_complement_numbers_to_positive_values_works_properly)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R16, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0xFFFFFFFF };
    const zcpu_register_t SOURCE_REG2 = { .name = R23, .value = 0x00000200 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (ADD(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = 0x000001FF; // -1 + 0x200 = 0x1FF

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, ADDING_a_pair_of_twos_complement_negative_numbers_works_correctly)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R16, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0xFFFFFFFF };
    const zcpu_register_t SOURCE_REG2 = { .name = R23, .value = 0xFFFFFE00 }; // -0x200
    const uint32_t INSTRUCTION_TO_EXECUTE = (ADD(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = 0xFFFFFDFF;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}


//TESTS FOR SUBTRACTING THE CONTENTS OF TWO REGISTERS AND WRITING THE OUTPUT TO A THIRD REG
//FIXME: All of these ALU instruction tests also need to verify the condition code bits

TEST(CPU_INSTRUCTION_TESTS, SUBBING_zero_from_a_register_changes_nothing)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0x01234567 };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x00 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (SUB(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = SOURCE_REG1.value;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, SUBBING_one_from_zero_yields_all_ones)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R0, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0x00 };
    const zcpu_register_t SOURCE_REG2 = { .name = R1, .value = 0x01 };
    const uint32_t INSTRUCTION_TO_EXECUTE = (SUB(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = ALL_ONES;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, SUBBING_a_register_from_itself_yields_zero)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R16, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = ALL_ONES };
    const zcpu_register_t SOURCE_REG2 = { .name = R0, .value = ALL_ONES };
    const uint32_t INSTRUCTION_TO_EXECUTE = (SUB(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = 0x00000000;

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

TEST(CPU_INSTRUCTION_TESTS, SUBBING_a_twos_complement_negative_number_from_a_positive_number_is_the_same_as_adding)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const zcpu_register_t DEST_REG    = { .name = R16, .value = INVALID_DATA };
    const zcpu_register_t SOURCE_REG1 = { .name = R0, .value = 0x00000200 };
    const zcpu_register_t SOURCE_REG2 = { .name = R23, .value = ALL_ONES };
    const uint32_t INSTRUCTION_TO_EXECUTE = (SUB(DEST_REG.name, SOURCE_REG1.name, SOURCE_REG2.name));
    const uint32_t EXPECTED_TEST_VALUE = 0x00000201; // 0x200 - (-1) = 0x201

    test_single_instruction2( cpu, &mock_bus,
                             DEST_REG,
                             SOURCE_REG1,
                             SOURCE_REG2,
                             INSTRUCTION_TO_EXECUTE,
                             EXPECTED_TEST_VALUE);
}

static void set_PC(cpu_t* cpu, uint32_t address)
{
    cpu->PC = address;
}

static uint32_t get_PC(cpu_t* cpu)
{
    return cpu->PC;
}

static void test_JUMP_instruction(cpu_t* cpu_to_test,
                                  memory_bus_t* mock_bus,
                                  uint32_t instruction_to_execute,
                                  const uint32_t starting_address,
                                  const uint32_t expected_ending_address)
{
    set_PC(cpu_to_test, starting_address);
    set_expected_instruction(mock_bus, instruction_to_execute);
    single_step(cpu_to_test, mock_bus);
    LONGS_EQUAL(expected_ending_address, get_PC(cpu_to_test));
}

#define GET_LARGEST_POSITIVE_TWOS_COMPLEMENT_VALUE_THAT_FITS_IN_NUM_BITS(num_bits) ((1 << (num_bits-1)) - 1)
#define GET_LARGEST_NEGATIVE_TWOS_COMPLEMENT_VALUE_THAT_FITS_IN_NUM_BITS(num_bits) (-(1 << (num_bits-1)))

//JUMP instruction tests
TEST(CPU_INSTRUCTION_TESTS, JUMP_instructions_properly_handle_large_negative_offsets)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint32_t starting_addr = 0x000000FF;
    const uint32_t NUM_OFFSET_BITS_FOR_JUMP_INSTRUCTION = 26;
    const int32_t JUMP_OFFSET = GET_LARGEST_NEGATIVE_TWOS_COMPLEMENT_VALUE_THAT_FITS_IN_NUM_BITS(NUM_OFFSET_BITS_FOR_JUMP_INSTRUCTION);
    const uint32_t expected_ending_address = (starting_addr + 1 + JUMP_OFFSET);
    const uint32_t instruction = (JUMP(JUMP_OFFSET));

    test_JUMP_instruction(cpu, &mock_bus, instruction, starting_addr, expected_ending_address);
}

TEST(CPU_INSTRUCTION_TESTS, JUMP_instruction_with_offset_of_zero_leaves_the_program_counter_at_PC_plus_one)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint32_t starting_addr = 0x000000FF;
    const int32_t JUMP_OFFSET = 0;
    const uint32_t expected_ending_address = (starting_addr + 1 + JUMP_OFFSET);
    const uint32_t instruction = (JUMP(JUMP_OFFSET));

    test_JUMP_instruction(cpu, &mock_bus, instruction, starting_addr, expected_ending_address);
}

TEST(CPU_INSTRUCTION_TESTS, JUMP_instructions_properly_handle_large_positive_offsets)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint32_t starting_addr = 0x000000FF;
    const uint32_t NUM_OFFSET_BITS_FOR_JUMP_INSTRUCTION = 26;
    const int32_t JUMP_OFFSET = GET_LARGEST_POSITIVE_TWOS_COMPLEMENT_VALUE_THAT_FITS_IN_NUM_BITS(NUM_OFFSET_BITS_FOR_JUMP_INSTRUCTION);
    const uint32_t expected_ending_address = (starting_addr + 1 + JUMP_OFFSET);
    const uint32_t instruction = (JUMP(JUMP_OFFSET));

    test_JUMP_instruction(cpu, &mock_bus, instruction, starting_addr, expected_ending_address);
}

//JUMPR instruction tests

static void test_JUMPR_instruction(cpu_t* cpu_to_test,
                                  memory_bus_t* mock_bus,
                                  uint32_t instruction_to_execute,
                                  uint32_t base_reg_name,
                                  uint32_t base_reg_value,
                                  const uint32_t starting_address,
                                  const uint32_t expected_ending_address)
{
    set_register_value(cpu_to_test, base_reg_name, base_reg_value);
    set_PC(cpu_to_test, starting_address);
    set_expected_instruction(mock_bus, instruction_to_execute);
    single_step(cpu_to_test, mock_bus);
    LONGS_EQUAL(expected_ending_address, get_PC(cpu_to_test));
}

TEST(CPU_INSTRUCTION_TESTS, JUMPR_instructions_properly_handle_large_negative_offsets)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint32_t starting_addr = 0x000000FF;
    const uint32_t NUM_OFFSET_BITS_FOR_JUMPR_INSTRUCTION = 16;
    const int32_t JUMPR_OFFSET = GET_LARGEST_NEGATIVE_TWOS_COMPLEMENT_VALUE_THAT_FITS_IN_NUM_BITS(NUM_OFFSET_BITS_FOR_JUMPR_INSTRUCTION);
    const uint8_t base_reg_name = R0;
    const uint32_t base_reg_value = 0;
    const uint32_t instruction = (JUMPR(base_reg_name, JUMPR_OFFSET));
    const uint32_t expected_ending_address = ( base_reg_value + JUMPR_OFFSET);

    test_JUMPR_instruction(cpu, &mock_bus, instruction, base_reg_name, base_reg_value, starting_addr, expected_ending_address);
}

TEST(CPU_INSTRUCTION_TESTS, JUMPR_instruction_with_offset_of_zero_just_jumps_to_the_base_register_address)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint32_t starting_addr = 0x000000FF;
    const int32_t JUMPR_OFFSET = 0;
    const uint8_t base_reg_name = R0;
    const uint32_t base_reg_value = 0xDEADBEEF;
    const uint32_t instruction = (JUMPR(base_reg_name, JUMPR_OFFSET));
    const uint32_t expected_ending_address = base_reg_value;

    test_JUMPR_instruction(cpu, &mock_bus, instruction, base_reg_name, base_reg_value, starting_addr, expected_ending_address);
}

TEST(CPU_INSTRUCTION_TESTS, JUMPR_instructions_properly_handle_large_positive_offsets)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint32_t starting_addr = 0x000000FF;
    const uint32_t NUM_OFFSET_BITS_FOR_JUMPR_INSTRUCTION = 16;
    const int32_t JUMPR_OFFSET = GET_LARGEST_POSITIVE_TWOS_COMPLEMENT_VALUE_THAT_FITS_IN_NUM_BITS(NUM_OFFSET_BITS_FOR_JUMPR_INSTRUCTION);
    const uint8_t base_reg_name = R0;
    const uint32_t base_reg_value = 0;
    const uint32_t instruction = (JUMPR(base_reg_name, JUMPR_OFFSET));
    const uint32_t expected_ending_address = ( base_reg_value + JUMPR_OFFSET);

    test_JUMPR_instruction(cpu, &mock_bus, instruction, base_reg_name, base_reg_value, starting_addr, expected_ending_address);
}

TEST(CPU_INSTRUCTION_TESTS, JUMPR_instructions_works_with_other_registers)
{
    memory_bus_t mock_bus;
    cpu_t* cpu = build_cpu(&mock_bus, ic);

    const uint32_t starting_addr = 0x000000FF;
    const int32_t JUMPR_OFFSET = 0x0004110;
    const uint8_t base_reg_name = R31;
    const uint32_t base_reg_value = 0xDEADBEEF;
    const uint32_t instruction = (JUMPR(base_reg_name, JUMPR_OFFSET));
    const uint32_t expected_ending_address = 0xDEADFFFF;

    test_JUMPR_instruction(cpu, &mock_bus, instruction, base_reg_name, base_reg_value, starting_addr, expected_ending_address);
}
