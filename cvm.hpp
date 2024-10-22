#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <iomanip>

#include "common.hpp"

class CVM {
private:
    std::vector<uint16_t> registers;

    void printRegisters() const {
        std::cout << "Registers: ";
        for (size_t i = 0; i < registers.size(); ++i) {
            if (registers[i] != 0) {
                std::cout << "R" << i << "=" << registers[i] << " ";
            }
        }
        std::cout << std::endl;
    }

public:
    void execute(const std::vector<uint16_t>& bytecode) {
        registers.clear();
        registers.resize(256, 0);
        size_t i = 0;
        while (i < bytecode.size()) {
            uint16_t inst = bytecode[i++];
            std::cout << "Executing instruction: 0x" << std::hex << inst << std::dec << std::endl;
            switch (inst) {
            case MOV: {
                uint16_t toReg = bytecode[i++];
                uint16_t fromRegOrVal = bytecode[i++];
                if (fromRegOrVal & 0x100) {
                    uint16_t fromReg = fromRegOrVal & 0xFF;
                    std::cout << "MOV R" << toReg << ", R" << fromReg << std::endl;
                    registers[toReg] = registers[fromReg];
                }
                else {
                    std::cout << "MOV R" << toReg << ", " << fromRegOrVal << std::endl;
                    registers[toReg] = fromRegOrVal;
                }
                break;
            }
            case ADD: {
                uint16_t reg1 = bytecode[i++];
                uint16_t reg2 = bytecode[i++];
                uint16_t rreg = bytecode[i++]; // result register
                std::cout << "ADD R" << reg1 << ", R" << reg2 << ", R" << rreg << std::endl;
                registers[rreg] = registers[reg1] + registers[reg2];
                break;
            }
            case SUB: {
                uint16_t reg1 = bytecode[i++];
                uint16_t reg2 = bytecode[i++];
                uint16_t rreg = bytecode[i++];
                std::cout << "SUB R" << reg1 << ", R" << reg2 << ", R" << rreg << std::endl;
                registers[rreg] = registers[reg1] - registers[reg2];
                break;
            }
            case MUL: {
                uint16_t reg1 = bytecode[i++];
                uint16_t reg2 = bytecode[i++];
                uint16_t rreg = bytecode[i++];
                std::cout << "MUL R" << reg1 << ", R" << reg2 << ", R" << rreg << std::endl;
                registers[rreg] = registers[reg1] * registers[reg2];
                break;
            }
            case DIV: {
                uint16_t reg1 = bytecode[i++];
                uint16_t reg2 = bytecode[i++];
                uint16_t rreg = bytecode[i++];
                std::cout << "DIV R" << reg1 << ", R" << reg2 << ", R" << rreg << std::endl;
                registers[rreg] = registers[reg1] / registers[reg2];
                break;
            }
            case MOD: {
                uint16_t reg1 = bytecode[i++];
                uint16_t reg2 = bytecode[i++];
                uint16_t rreg = bytecode[i++];
                std::cout << "MOD R" << reg1 << ", R" << reg2 << ", R" << rreg << std::endl;
                registers[rreg] = static_cast<int>(registers[reg1]) % static_cast<int>(registers[reg2]);
                break;
            }
            case POW: {
                uint16_t breg = bytecode[i++]; // base reg
                uint16_t ereg = bytecode[i++]; // exponent reg
                uint16_t rreg = bytecode[i++]; // result reg
                std::cout << "POW R" << breg << ", R" << ereg << ", R" << rreg << std::endl;
                registers[rreg] = std::pow(registers[breg], registers[ereg]);
                break;
            }
            default:
                throw std::runtime_error("Unknown instruction: " + std::to_string(inst));
            }
            printRegisters();
        }
    }

    uint16_t result() const {
        return registers[0];
    }
};