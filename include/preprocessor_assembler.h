
#ifndef __PREPROCESSOR_ASSEMBLER_H_
#define __PREPROCESSOR_ASSEMBLER_H_


//This module is a convenience module so that I can write small programs in
//assembly language without needing to write a full-blown separate assembler
//for testing purposes. To be sure, there will be an assembler later (and a
//compiler), but for now, this will help me write test cases more easily.

//The way to use this "assembler" is to invoke the function-like macros with the name of the instruction that you want, feeding it the proper arguments according to what the instruction's normal args should be, and it will generate the appropriate constant representing the encoded instruction for you.
//
//e.g.: NOT R0  --> NOT(R0) --> 0x08000000


//HELPER MACROS
#define GET_BOTTOM_BITS(value, number)  ((value) & (((1u) << number) - 1))
#define REGISTER_OP(op, dest, sr1, sr2)     ((op << 26) | (dest << 21) | (sr1 << 16) | (sr2 << 11))
#define IMMEDIATE_OP(op, dest, sr, imm15)   ((op << 26) | (dest << 21) | (sr << 16) | (GET_BOTTOM_BITS(imm15, 15) << 1) | IMMEDIATE_MODE)

//OPCODES
#define OPCODE_AND  (0x00)
#define OPCODE_OR   (0x01)
#define OPCODE_NOT  (0x01)

//AUXILIARY BITS: Additional bits that are needed for proper instruction encoding
//e.g. immediate mode flag bits, branch condition bits, &c.

#define IMMEDIATE_MODE (0x01)   //indicates whether to use immediate bits in instruction encoding
#define REGISTER_MODE  (0x00)   //indicates whether to use only registers for the operation

//REGISTER NAMES
#define R0  (0x00)
#define R1  (0x01)
#define R2  (0x02)
#define R3  (0x03)
#define R4  (0x04)
#define R5  (0x05)
#define R6  (0x06)
#define R7  (0x07)
#define R8  (0x08)
#define R9  (0x09)
#define R10 (0x0A)
#define R11 (0x0B)
#define R12 (0x0C)
#define R13 (0x0D)
#define R14 (0x0E)
#define R15 (0x0F)
#define R16 (0x10)
#define R17 (0x11)
#define R18 (0x12)
#define R19 (0x13)
#define R20 (0x14)
#define R21 (0x15)
#define R22 (0x16)
#define R23 (0x17)
#define R24 (0x18)
#define R25 (0x19)
#define R26 (0x1A)
#define R27 (0x1B)
#define R28 (0x1C)
#define R29 (0x1D)
#define R30 (0x1E)
#define R31 (0x1F)

//INSTRUCTIONS

#define AND(destination_reg, source_reg1, source_reg2)                      REGISTER_OP(OPCODE_AND, destination_reg, source_reg1, source_reg2)
#define AND_IMMEDIATE(destination_reg, source_reg, immediate_value_15_bits) IMMEDIATE_OP(OPCODE_AND, destination_reg, source_reg, immediate_value_15_bits)
#define OR(destination_reg, source_reg1, source_reg2)                       REGISTER_OP(OPCODE_OR, destination_reg, source_reg1, source_reg2)
#define OR_IMMEDIATE(destination_reg, source_reg, immediate_value_15_bits)  IMMEDIATE_OP(OPCODE_OR, destination_reg, source_reg, immediate_value_15_bits)
//the bottom 16 bits of "NOT" are not used, so we don't care about their value
#define NOT(destination_reg, source_reg)                                    IMMEDIATE_OP(OPCODE_NOT, destination_reg, source_reg, 0x00)  


#endif // __PREPROCESSOR_ASSEMBLER_H_
