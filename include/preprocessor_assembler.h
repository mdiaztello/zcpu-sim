
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
#define GET_BOTTOM_BITS(value, number)          ((value) & (((1u) << (number)) - 1))
#define REGISTER_OP(op, dest, sr1, sr2)         (((op) << 26) | ((dest) << 21) | ((sr1) << 16) | ((sr2) << 11))
#define IMMEDIATE_OP(op, dest, sr, imm15)       (((op) << 26) | ((dest) << 21) | ((sr) << 16) | (GET_BOTTOM_BITS((imm15), 15) << 1) | IMMEDIATE_MODE)
#define PC_RELATIVE(op, reg, imm21)             (((op) << 26) | ((reg) << 21) | (GET_BOTTOM_BITS((imm21), 21)))
#define BASE_PLUS_OFFSET(op, reg, base, imm16)  (((op) << 26) | ((reg) << 21) | ((base) << 16) | (GET_BOTTOM_BITS((imm16), 16)))
#define JUMP_PC_RELATIVE(op, pc_rel_offset26)   ((op << 26) | (GET_BOTTOM_BITS((pc_rel_offset26), 26)))
#define BRANCH_PC_RELATIVE(op, N, Z, P, pc_rel_offset23) ((op << 26) | (N << 25) | (Z << 24) | (P << 23) | (GET_BOTTOM_BITS((pc_rel_offset23), 23)))

#include "opcode_list.h"

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

//INSTRUCTIONS: USE THESE MACROS TO WRITE YOUR ASM PROGRAMS

//ALU INSTRUCTIONS

#define AND(destination_reg, source_reg1, source_reg2)                      REGISTER_OP(OPCODE_AND, destination_reg, source_reg1, source_reg2)
#define AND_IMMEDIATE(destination_reg, source_reg, immediate_value_15_bits) IMMEDIATE_OP(OPCODE_AND, destination_reg, source_reg, immediate_value_15_bits)

#define OR(destination_reg, source_reg1, source_reg2)                       REGISTER_OP(OPCODE_OR, destination_reg, source_reg1, source_reg2)
#define OR_IMMEDIATE(destination_reg, source_reg, immediate_value_15_bits)  IMMEDIATE_OP(OPCODE_OR, destination_reg, source_reg, immediate_value_15_bits)

//the bottom 16 bits of "NOT" are not used, so we don't care about their value
#define NOT(destination_reg, source_reg)                                    REGISTER_OP(OPCODE_NOT, destination_reg, source_reg, 0x00)

#define XOR(destination_reg, source_reg1, source_reg2)                      REGISTER_OP(OPCODE_XOR, destination_reg, source_reg1, source_reg2)
#define XOR_IMMEDIATE(destination_reg, source_reg, immediate_value_15_bits) IMMEDIATE_OP(OPCODE_XOR, destination_reg, source_reg, immediate_value_15_bits)
//a useful mnemonic to have for clearing a register
#define CLEAR(reg)                                                          XOR(reg, reg, reg)

#define ADD(destination_reg, source_reg1, source_reg2)                      REGISTER_OP(OPCODE_ADD, destination_reg, source_reg1, source_reg2)
#define ADD_IMMEDIATE(destination_reg, source_reg, immediate_value_15_bits) IMMEDIATE_OP(OPCODE_ADD, destination_reg, source_reg, immediate_value_15_bits)

#define SUB(destination_reg, source_reg1, source_reg2)                      REGISTER_OP(OPCODE_SUB, destination_reg, source_reg1, source_reg2)
#define SUB_IMMEDIATE(destination_reg, source_reg, immediate_value_15_bits) IMMEDIATE_OP(OPCODE_SUB, destination_reg, source_reg, immediate_value_15_bits)

#define SHIFTL(destination_reg, source_reg1, source_reg2)                   REGISTER_OP(OPCODE_SHIFTL, destination_reg, source_reg1, source_reg2)
#define SHIFTL_IMMEDIATE(destination_reg, source_reg, immediate_value_15_bits)         IMMEDIATE_OP(OPCODE_SHIFTL, destination_reg, source_reg, immediate_value_15_bits)

#define ASHIFTR(destination_reg, source_reg1, source_reg2)                   REGISTER_OP(OPCODE_ASHIFTR, destination_reg, source_reg1, source_reg2)
#define ASHIFTR_IMMEDIATE(destination_reg, source_reg, immediate_value_15_bits)         IMMEDIATE_OP(OPCODE_ASHIFTR, destination_reg, source_reg, immediate_value_15_bits)

//LOAD INSTRUCTIONS

#define LOAD(destination_reg, pc_relative_offset)                           PC_RELATIVE(OPCODE_LOAD, destination_reg, pc_relative_offset)
#define LOADR(destination_reg, base_reg, offset)                            BASE_PLUS_OFFSET(OPCODE_LOADR, destination_reg, base_reg, offset)
#define LOADA(destination_reg, pc_relative_offset)                          PC_RELATIVE(OPCODE_LOADA, destination_reg, pc_relative_offset)

//STORE INSTRUCTIONS

#define STORE(source_reg, pc_relative_offset)                               PC_RELATIVE(OPCODE_STORE, source_reg, pc_relative_offset)
#define STORER(source_reg, base_reg, offset)                                BASE_PLUS_OFFSET(OPCODE_STORER, source_reg, base_reg, offset)

//JUMP INSTRUCTIONS

#define JUMP(pc_relative_offset)                                            JUMP_PC_RELATIVE(OPCODE_JUMP, pc_relative_offset)
//The venerable Halt-catch-fire instruction!
#define HCF                                                                 JUMP(-1)    //spin forever
#define JUMPR(base_reg, offset)                                             BASE_PLUS_OFFSET(OPCODE_JUMPR, 0x00, base_reg, offset)
#define RETURN                                                              JUMPR(R30, 0)

//BRANCH INSTRUCTIONS
#define BRNZP(pc_relative_offset)                                           BRANCH_PC_RELATIVE(OPCODE_BRANCH, 1, 1, 1, pc_relative_offset)
#define BRNZ(pc_relative_offset)                                            BRANCH_PC_RELATIVE(OPCODE_BRANCH, 1, 1, 0, pc_relative_offset)
#define BRZP(pc_relative_offset)                                            BRANCH_PC_RELATIVE(OPCODE_BRANCH, 0, 1, 1, pc_relative_offset)
#define BRNP(pc_relative_offset)                                            BRANCH_PC_RELATIVE(OPCODE_BRANCH, 1, 0, 1, pc_relative_offset)
#define BRN(pc_relative_offset)                                             BRANCH_PC_RELATIVE(OPCODE_BRANCH, 1, 0, 0, pc_relative_offset)
#define BRZ(pc_relative_offset)                                             BRANCH_PC_RELATIVE(OPCODE_BRANCH, 0, 1, 0, pc_relative_offset)
#define BRP(pc_relative_offset)                                             BRANCH_PC_RELATIVE(OPCODE_BRANCH, 0, 0, 1, pc_relative_offset)
//BNV is useless b/c it never branches, but the encoding must be handled since it implicitly exists
#define BNV(pc_relative_offset)                                             BRANCH_PC_RELATIVE(OPCODE_BRANCH, 0, 0, 0, pc_relative_offset)
//BRANCH INSTRUCTION ALIASES (for convenience purposes)
#define BRA                                                                 BRNZP
#define BR                                                                  BRNZP

//CALL INSTRUCTIONS
#define CALL(pc_relative_offset)                                            JUMP_PC_RELATIVE(OPCODE_CALL, pc_relative_offset)
#define CALLR(base_reg, offset)                                             BASE_PLUS_OFFSET(OPCODE_CALLR, 0x00, base_reg, offset)

//TRAP INSTRUCTION AND ALIASES
#define TRAP(trap_vector_register)                                          REGISTER_OP(OPCODE_TRAP, trap_vector_register, 0x00, 0x00)
#define SWI(trap_vector_register)                                           TRAP(trap_vector_register)
#define SYSCALL(trap_vector_register)                                       TRAP(trap_vector_register)

//RETURNI (RETURN FROM INTERRUPT INSTRUCTION)
#define RETURNI                                                             REGISTER_OP(OPCODE_RETURNI, 0x00, 0x00, 0x00)
#define RFI                                                                 RETURNI
#define SYSCALL_EXIT                                                        RETURNI



#endif // __PREPROCESSOR_ASSEMBLER_H_
