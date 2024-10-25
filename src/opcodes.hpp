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
        default: return "UNKNOWN";
    }
}
