#pragma once

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <iomanip>

#define MOV      0x01 // move a value to a register
#define LOADV    0x02 // load variable
#define STOREV   0x03 // store variable
#define ADD      0x04
#define SUB      0x05
#define MUL      0x06
#define DIV      0x07
#define MOD      0x08 // modulo
#define POW      0x09 // power
#define NEG      0x0A // negation
#define NOT      0x0B // logical NOT
#define EQ       0x0C // equal
#define NEQ      0x0D // not equal
#define GT       0x0E // greater than
#define LT       0x0F // less than
#define GTE      0x10 // greater than or equal
#define LTE      0x11 // less than or equal
#define AND      0x12 // logical AND
#define OR       0x13 // logical OR

#define LOG(msg) std::cout << "LOG: " << msg << std::endl;

std::string prettyBytecode(const std::vector<uint16_t>& bytecode) {
    std::unordered_map<uint16_t, std::string> instructionMap = {
        {0x01, "MOV"},    {0x02, "LOADV"}, {0x03, "STOREV"},
        {0x04, "ADD"},    {0x05, "SUB"},   {0x06, "MUL"},
        {0x07, "DIV"},    {0x08, "MOD"},   {0x09, "POW"},
        {0x0A, "NEG"},    {0x0B, "NOT"},   {0x0C, "EQ"},
        {0x0D, "NEQ"},    {0x0E, "GT"},    {0x0F, "LT"},
        {0x10, "GTE"},    {0x11, "LTE"},   {0x12, "AND"},
        {0x13, "OR"}
    };

    std::stringstream output;
    size_t i = 0;

    while (i < bytecode.size()) {
        uint16_t inst = bytecode[i++];

        if (instructionMap.find(inst) != instructionMap.end()) {
            output << std::setw(8) << std::left << instructionMap[inst];

            if (inst == MOV || inst == LOADV || inst == STOREV || inst == ADD ||
                inst == SUB || inst == MUL || inst == DIV || inst == MOD || inst == POW) {
                uint16_t reg1 = bytecode[i++];
                uint16_t reg2 = bytecode[i++];
                output << "R" << reg1 << ", R" << reg2;

                if (inst == ADD || inst == SUB || inst == MUL || inst == DIV ||
                    inst == MOD || inst == POW) {
                    uint16_t rreg = bytecode[i++];
                    output << ", R" << rreg;
                }
            }
            else if (inst == NEG || inst == NOT) {
                uint16_t reg = bytecode[i++];
                output << "R" << reg;
            }
            else if (inst == EQ || inst == NEQ || inst == GT || inst == LT ||
                inst == GTE || inst == LTE || inst == AND || inst == OR) {
                uint16_t reg1 = bytecode[i++];
                uint16_t reg2 = bytecode[i++];
                output << "R" << reg1 << ", R" << reg2;
            }
            output << std::endl;
        }
        else {
            output << "Unknown Instruction: " << std::hex << inst << std::endl;
        }
    }

    return output.str();
}
