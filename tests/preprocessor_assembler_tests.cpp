
#include "CppUTest/TestHarness.h"

extern "C"
{
#include <stdio.h>
#include "preprocessor_assembler.h"
}

//
// The tests here are meant to test my preprocessor "assembler", which is a
// quick-and-dirty way of writing assembly language programs for the processor
// without having to write a separate assembler program. The drawback to this
// method is that, although it is quick, it lacks the ability to use labels,
// which means that writing big programs with subroutines and loops will be
// painful (since all of the offsets have to be computed manually). That's ok
// though, because I am only going to use it with short enough programs to
// verify that the processor is properly executing individual instructions and
// short sequences of instructions. In the mean time, I still need to make sure
// that the instructions are being encoded properly by the "assembler", else I
// will end up going on a wild goose chase when I see a bug that turns out to
// be an improperly encoded instruction.
//

TEST_GROUP(PREPROCESSOR_ASSEMBLER_TESTS)
{
    void setup(void)
    {
    }

    void teardown(void)
    {
    }
};

// LOAD instruction tests

TEST(PREPROCESSOR_ASSEMBLER_TESTS, load_instruction_encoded_no_offset_correctly)
{
    CHECK( LOAD(R0, 0) == 0x2C000000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, load_instruction_encoded_negative_one_offset_correctly)
{
    CHECK( LOAD(R0, -1) == 0x2C1FFFFF );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, load_instruction_encoded_max_negative_offset_correctly)
{
    //the pc-relative offset is 21-bits, so the most negative offset is -2^20 or -1048576
    CHECK( LOAD(R0, -1048576) == 0x2C100000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, load_instruction_encoding_truncates_the_last_21_bits)
{
    //the pc-relative offset is 21-bits, so the most negative offset is -2^20 or -1048576 or 0xFFF00000
    CHECK( LOAD(R0, 0xFFD00000) == 0x2C100000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, load_instruction_encoded_positive_one_offset_correctly)
{
    CHECK( LOAD(R0, 1) == 0x2C000001 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, load_instruction_encoded_max_positive_offset_correctly)
{
    //the pc-relative offset is 21-bits, so the most positive offset is 2^20-1 or 1048575
    CHECK( LOAD(R0, 1048575) == 0x2C0FFFFF );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, load_instruction_encoded_destination_register_correctly)
{
    CHECK( LOAD(R31, 0) == 0x2FE00000 );
    CHECK( LOAD(R16, 0) == 0x2E000000 );
    CHECK( LOAD(R1, 0) == 0x2C200000 );
    CHECK( LOAD(R0, 0) == 0x2C000000 );
}

//LOADR instruction tests

TEST(PREPROCESSOR_ASSEMBLER_TESTS, loadr_instruction_encoded_destination_register_correctly)
{
    CHECK( LOADR(R31, R0, 0) == 0x33E00000 );
    CHECK( LOADR(R16, R0, 0) == 0x32000000 );
    CHECK( LOADR(R1, R0, 0) == 0x30200000 );
    CHECK( LOADR(R0, R0, 0) == 0x30000000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, loadr_instruction_encoded_base_register_correctly)
{
    CHECK( LOADR(R0, R31, 0) == 0x301F0000 );
    CHECK( LOADR(R0, R16, 0) == 0x30100000 );
    CHECK( LOADR(R0, R1, 0) == 0x30010000 );
    CHECK( LOADR(R0, R0, 0) == 0x30000000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, loadr_instruction_encoded_zero_offset_correctly)
{
    CHECK( LOADR(R0, R0, 0) == 0x30000000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, loadr_instruction_encoded_negative_one_offset_correctly)
{
    CHECK( LOADR(R0, R0, -1) == 0x3000FFFF );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, loadr_instruction_encoded_max_negative_offset_correctly)
{
    //offset for base+offset instructions is 16 bits, so most negative value is -32768
    CHECK( LOADR(R0, R0, -32768) == 0x30008000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, loadr_instruction_encoded_positive_one_offset_correctly)
{
    //offset for base+offset instructions is 16 bits, so most negative value is -32768
    CHECK( LOADR(R0, R0, 1) == 0x30000001 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, loadr_instruction_encoded_max_positive_offset_correctly)
{
    //offset for base+offset instructions is 16 bits, so most negative value is 32767
    CHECK( LOADR(R0, R0, 32767) == 0x30007FFF );
}

//LOADA instruction tests
//this instruction uses the same format as LOAD and therefore the same macro to
//encode it, so we'll just do some abbreviated testing
TEST(PREPROCESSOR_ASSEMBLER_TESTS, loada_instruction_encoded_offsets_correctly)
{
    //no offset
    CHECK( LOADA(R0, 0) == 0x34000000 );

    //offset = -1
    CHECK( LOADA(R0, -1) == 0x341FFFFF );

    //most negative offset
    CHECK( LOADA(R0, -1048576) == 0x34100000 );

    //offsets only use bottom 21 bits of constant
    CHECK( LOADA(R0, 0xFFD00000) == 0x34100000 );

    //offset = +1
    CHECK( LOADA(R0, 1) == 0x34000001 );

    //most positive offset
    CHECK( LOADA(R0, 1048575) == 0x340FFFFF );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, loada_instruction_encoded_destination_register_correctly)
{
    CHECK( LOADA(R31, 0) == 0x37E00000 );
    CHECK( LOADA(R16, 0) == 0x36000000 );
    CHECK( LOADA(R1, 0) == 0x34200000 );
    CHECK( LOADA(R0, 0) == 0x34000000 );
}


// STORE instruction tests

TEST(PREPROCESSOR_ASSEMBLER_TESTS, store_instruction_encoded_no_offset_correctly)
{
    CHECK( STORE(R0, 0) == 0x38000000);
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, store_instruction_encoded_negative_one_offset_correctly)
{
    CHECK( STORE(R0, -1) == 0x381FFFFF );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, store_instruction_encoded_max_negative_offset_correctly)
{
    //the pc-relative offset is 21-bits, so the most negative offset is -2^20 or -1048576
    CHECK( STORE(R0, -1048576) == 0x38100000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, store_instruction_encoding_truncates_the_last_21_bits)
{
    //the pc-relative offset is 21-bits, so the most negative offset is -2^20 or -1048576 or 0xFFF00000
    CHECK( STORE(R0, 0xFFD00000) == 0x38100000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, store_instruction_encoded_positive_one_offset_correctly)
{
    CHECK( STORE(R0, 1) == 0x38000001 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, store_instruction_encoded_max_positive_offset_correctly)
{
    //the pc-relative offset is 21-bits, so the most positive offset is 2^20-1 or 1048575
    CHECK( STORE(R0, 1048575) == 0x380FFFFF );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, store_instruction_encoded_destination_register_correctly)
{
    CHECK( STORE(R31, 0) == 0x3BE00000 );
    CHECK( STORE(R16, 0) == 0x3A000000 );
    CHECK( STORE(R1, 0) == 0x38200000 );
    CHECK( STORE(R0, 0) == 0x38000000 );
}

//STORER instruction tests

TEST(PREPROCESSOR_ASSEMBLER_TESTS, storer_instruction_encoded_destination_register_correctly)
{
    CHECK( STORER(R31, R0, 0) == 0x3FE00000 );
    CHECK( STORER(R16, R0, 0) == 0x3E000000 );
    CHECK( STORER(R1, R0, 0) == 0x3C200000 );
    CHECK( STORER(R0, R0, 0) == 0x3C000000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, storer_instruction_encoded_base_register_correctly)
{
    CHECK( STORER(R0, R31, 0) == 0x3C1F0000 );
    CHECK( STORER(R0, R16, 0) == 0x3C100000 );
    CHECK( STORER(R0, R1, 0) == 0x3C010000 );
    CHECK( STORER(R0, R0, 0) == 0x3C000000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, storer_instruction_encoded_zero_offset_correctly)
{
    CHECK( STORER(R0, R0, 0) == 0x3C000000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, storer_instruction_encoded_negative_one_offset_correctly)
{
    CHECK( STORER(R0, R0, -1) == 0x3C00FFFF );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, storer_instruction_encoded_max_negative_offset_correctly)
{
    //offset for base+offset instructions is 16 bits, so most negative value is -32768
    CHECK( STORER(R0, R0, -32768) == 0x3C008000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, storer_instruction_encoded_positive_one_offset_correctly)
{
    //offset for base+offset instructions is 16 bits, so most negative value is -32768
    CHECK( STORER(R0, R0, 1) == 0x3C000001 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, storer_instruction_encoded_max_positive_offset_correctly)
{
    //offset for base+offset instructions is 16 bits, so most negative value is 32767
    CHECK( STORER(R0, R0, 32767) == 0x3C007FFF );
}


//AND instruction tests

TEST(PREPROCESSOR_ASSEMBLER_TESTS, and_instruction_encoded_destination_register_correctly)
{
    CHECK( AND(R0, R0, R0) == 0x00000000 );
    CHECK( AND(R1, R0, R0) == 0x00200000 );
    CHECK( AND(R16, R0, R0) == 0x02000000 );
    CHECK( AND(R31, R0, R0) == 0x03E00000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, and_instruction_encoded_first_source_register_correctly)
{
    CHECK( AND(R0, R0, R0) == 0x00000000 );
    CHECK( AND(R0, R1, R0) == 0x00010000 );
    CHECK( AND(R0, R16, R0) == 0x00100000 );
    CHECK( AND(R0, R31, R0) == 0x001F0000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, and_instruction_encoded_second_source_register_correctly)
{
    CHECK( AND(R0, R0, R0) == 0x00000000 );
    CHECK( AND(R0, R0, R1) == 0x00000800 );
    CHECK( AND(R0, R0, R16) == 0x00008000 );
    CHECK( AND(R0, R0, R31) == 0x0000F800 );
}

//AND_IMMEDIATE instruction tests

TEST(PREPROCESSOR_ASSEMBLER_TESTS, and_immediate_instruction_encoded_destination_register_correctly)
{
    CHECK( AND_IMMEDIATE(R0, R0, 0) == 0x00000001 );
    CHECK( AND_IMMEDIATE(R1, R0, 0) == 0x00200001 );
    CHECK( AND_IMMEDIATE(R16, R0, 0) == 0x02000001 );
    CHECK( AND_IMMEDIATE(R31, R0, 0) == 0x03E00001 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, and_immediate_instruction_encoded_first_source_register_correctly)
{
    CHECK( AND_IMMEDIATE(R0, R0, 0) == 0x00000001 );
    CHECK( AND_IMMEDIATE(R0, R1, 0) == 0x00010001 );
    CHECK( AND_IMMEDIATE(R0, R16, 0) == 0x00100001 );
    CHECK( AND_IMMEDIATE(R0, R31, 0) == 0x001F0001 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, and_immediate_instruction_encoded_immediate_value_correctly)
{
    //immediate 0 value
    CHECK( AND_IMMEDIATE(R0, R0, 0) == 0x00000001 );
    //immediate -1 value
    CHECK( AND_IMMEDIATE(R0, R0, -1) == 0x0000FFFF );
    //immediate most negative
    CHECK( AND_IMMEDIATE(R0, R0, -16384) == 0x00008001 );
    //immediate most positive
    CHECK( AND_IMMEDIATE(R0, R0, 16383) == 0x00007FFF );
    //only use 15 bit immediate value
    CHECK( AND_IMMEDIATE(R0, R0, 0x0FFF4000) == 0x00008001 );
}

//OR instruction tests

TEST(PREPROCESSOR_ASSEMBLER_TESTS, or_instruction_encoded_destination_register_correctly)
{
    CHECK( OR(R0, R0, R0) == 0x04000000 );
    CHECK( OR(R1, R0, R0) == 0x04200000 );
    CHECK( OR(R16, R0, R0) == 0x06000000 );
    CHECK( OR(R31, R0, R0) == 0x07E00000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, or_instruction_encoded_first_source_register_correctly)
{
    CHECK( OR(R0, R0, R0) == 0x04000000 );
    CHECK( OR(R0, R1, R0) == 0x04010000 );
    CHECK( OR(R0, R16, R0) == 0x04100000 );
    CHECK( OR(R0, R31, R0) == 0x041F0000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, or_instruction_encoded_second_source_register_correctly)
{
    CHECK( OR(R0, R0, R0) == 0x04000000 );
    CHECK( OR(R0, R0, R1) == 0x04000800 );
    CHECK( OR(R0, R0, R16) == 0x04008000 );
    CHECK( OR(R0, R0, R31) == 0x0400F800 );
}

//OR_IMMEDIATE instruction tests

TEST(PREPROCESSOR_ASSEMBLER_TESTS, or_immediate_instruction_encoded_destination_register_correctly)
{
    CHECK( OR_IMMEDIATE(R0, R0, 0) == 0x04000001 );
    CHECK( OR_IMMEDIATE(R1, R0, 0) == 0x04200001 );
    CHECK( OR_IMMEDIATE(R16, R0, 0) == 0x06000001 );
    CHECK( OR_IMMEDIATE(R31, R0, 0) == 0x07E00001 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, or_immediate_instruction_encoded_first_source_register_correctly)
{
    CHECK( OR_IMMEDIATE(R0, R0, 0) == 0x04000001 );
    CHECK( OR_IMMEDIATE(R0, R1, 0) == 0x04010001 );
    CHECK( OR_IMMEDIATE(R0, R16, 0) == 0x04100001 );
    CHECK( OR_IMMEDIATE(R0, R31, 0) == 0x041F0001 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, or_immediate_instruction_encoded_immediate_value_correctly)
{
    //immediate 0 value
    CHECK( OR_IMMEDIATE(R0, R0, 0) == 0x04000001 );
    //immediate -1 value
    CHECK( OR_IMMEDIATE(R0, R0, -1) == 0x0400FFFF );
    //immediate most negative
    CHECK( OR_IMMEDIATE(R0, R0, -16384) == 0x04008001 );
    //immediate most positive
    CHECK( OR_IMMEDIATE(R0, R0, 16383) == 0x04007FFF );
    //only use 15 bit immediate value
    CHECK( OR_IMMEDIATE(R0, R0, 0x0FFF4000) == 0x04008001 );
}

//NOT instruction tests

TEST(PREPROCESSOR_ASSEMBLER_TESTS, not_instruction_encoded_destination_register_correctly)
{
    CHECK( NOT(R0, R0) == 0x08000000 );
    CHECK( NOT(R1, R0) == 0x08200000 );
    CHECK( NOT(R16, R0) == 0x0A000000 );
    CHECK( NOT(R31, R0) == 0x0BE00000 );
}

TEST(PREPROCESSOR_ASSEMBLER_TESTS, not_instruction_encoded_source_register_correctly)
{
    CHECK( NOT(R0, R0) == 0x08000000 );
    CHECK( NOT(R0, R1) == 0x08010000 );
    CHECK( NOT(R0, R16) == 0x08100000 );
    CHECK( NOT(R0, R31) == 0x081F0000 );
}