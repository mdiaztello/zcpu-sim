

#ifndef __OPCODE_LIST_H_
#define __OPCODE_LIST_H_

enum opcode_t 
{
    //ALU operations
    OPCODE_AND     = (0x00),
    OPCODE_OR      = (0x01),
    OPCODE_NOT     = (0x02),
    OPCODE_XOR     = (0x03),
    OPCODE_ADD     = (0x04),
    OPCODE_SUB     = (0x05),
    OPCODE_MUL     = (0x06),
    OPCODE_DIV     = (0x07),
    OPCODE_COMPARE = (0x08),
    OPCODE_SHIFTL  = (0x09),
    OPCODE_SHIFTR  = (0x0A),

    //Load instructions
    OPCODE_LOAD    = (0x0B),
    OPCODE_LOADR   = (0x0C),
    OPCODE_LOADA   = (0x0D),

    //store instructions
    OPCODE_STORE   = (0x0E),
    OPCODE_STORER  = (0x0F),

    //Jump instructions
    OPCODE_JUMP    = (0x10),

    //Branch instructions
    OPCODE_BRANCH  = (0x11),
};

#endif
