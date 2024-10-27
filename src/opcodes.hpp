#pragma once

#include <cstdint>
#include <string>

enum class OpCode : uint8_t {
    PUSH = 0x01, 
    ADD  = 0x02,
    SUB  = 0x03,
    MUL  = 0x04,
    DIV  = 0x05,
    MOD  = 0x06,

    LOAD  = 0x07,
    STORE = 0x08,
    NOT   = 0x09,
    INC   = 0x0A,
    DEC   = 0x0B,
    PUSHK = 0x0C,
    NEG   = 0x0D,

    CONCAT = 0x0E,
    PRINT  = 0x0F,

    MKARR  = 0x20,
    MKVEC  = 0x21,
    APUSH  = 0x22,
    GETIDX = 0x23,
    SETIDX = 0x24,
    ASIZE  = 0x25,
    VBACK  = 0x26,

    JMP    = 0x27,  // unconditional jump
    JMPF   = 0x28,  // jump if false

    GT     = 0x29, // greater than
    LT     = 0x30, // less than
    GTE    = 0x31, // greater than or equal
    LTE    = 0x32, // less than or equal
    EQ     = 0x33, // equal '=='
    NEQ    = 0x34, // not equal '!='

    RET    = 0x35,
    CALL   = 0x36,
    ENTER  = 0x37, // enter functions frame

    HALT = 0x00,
};

inline std::string op_as_string(const OpCode& opcode) {
    switch (opcode) {
        case OpCode::PUSH: return "PUSH";
        case OpCode::ADD: return "ADD";
        case OpCode::SUB: return "SUB";
        case OpCode::MUL: return "MUL";
        case OpCode::DIV: return "DIV";
        case OpCode::MOD: return "MOD";
        case OpCode::HALT: return "HALT";
        case OpCode::LOAD: return "LOAD";
        case OpCode::STORE: return "STORE";
        case OpCode::NOT: return "NOT";
        case OpCode::INC: return "INC";
        case OpCode::DEC: return "DEC";
        case OpCode::PUSHK: return "PUSHK";
        case OpCode::NEG: return "NEG";
        case OpCode::JMP: return "JMP";
        case OpCode::JMPF: return "JMPF";
        default: return "UNKNOWN";
    }
}
