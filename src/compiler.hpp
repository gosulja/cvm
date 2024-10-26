#pragma once

#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include "lexer.hpp"
#include "common.hpp"

class Compiler {
private:
    std::vector<uint16_t> bytecode;
    std::unordered_map<std::string, uint16_t> variables;
    uint16_t next_reg = 1;

    void emitByte(uint16_t byte) {
        bytecode.push_back(byte);
        //std::cout << "Emitted byte: 0x" << std::hex << byte << std::dec << std::endl;
    }

    void emitBytes(uint16_t byte1, uint16_t byte2) {
        emitByte(byte1);
        emitByte(byte2);
    }

    void emitBytes(uint16_t byte1, uint16_t byte2, uint16_t byte3) {
        emitByte(byte1);
        emitByte(byte2);
        emitByte(byte3);
    }

    void emitBytes(uint16_t byte1, uint16_t byte2, uint16_t byte3, uint16_t byte4) {
        emitByte(byte1);
        emitByte(byte2);
        emitByte(byte3);
        emitByte(byte4);
    }

    void emitLDR(uint16_t fromReg, uint16_t toReg) {
        emitByte(LDR);
        emitByte(toReg);
        emitByte(0x100 | fromReg);  // indicate it's a register, not a value
    }

    uint16_t allocRegister() {
        if (next_reg >= 256) {
            throw std::runtime_error("Register overflow.");
        }
        //std::cout << "Allocated register: " << next_reg << std::endl;
        return next_reg++;
    }

    uint16_t expression(const std::vector<Token>& tokens, size_t& index) {
        if (index >= tokens.size()) {
            throw std::runtime_error("Unexpected end of input.");
        }

        const Token& token = tokens[index];
        uint16_t result_reg;

        switch (token.type) {
        case NUMBER: {
            uint16_t value = std::stoi(token.lexeme);
            result_reg = allocRegister();
            emitBytes(LDR, result_reg, value);
            //std::cout << "Compiled number: " << value << " to register " << result_reg << std::endl;
            index++;
            break;
        }
        case IDENTIFIER: {
            auto it = variables.find(token.lexeme);
            if (it == variables.end()) {
                throw std::runtime_error("Undefined variable '" + token.lexeme + "'");
            }
            result_reg = it->second;
            index++;
            break;
        }
        case LPAREN: {
            index++; // skip (
            result_reg = expression(tokens, index);
            if (index >= tokens.size() || tokens[index].type != RPAREN) {
                throw std::runtime_error("Expected ')'");
            }
            index++; // skip )
            break;
        }
        default:
            throw std::runtime_error("Unexpected token: " + token.lexeme);
        }

        while (index < tokens.size() && tokens[index].type == OPERATOR) {
            const std::string& op = tokens[index].lexeme;
            index++;

            uint16_t right_reg = expression(tokens, index);
            uint16_t new_result_reg = allocRegister();

            //std::cout << "Compiling operation: " << op << std::endl;

            if (op == "+") {
                emitBytes(ADD, result_reg, right_reg, new_result_reg);
            }
            else if (op == "-") {
                emitBytes(SUB, result_reg, right_reg, new_result_reg);
            }
            else if (op == "*") {
                emitBytes(MUL, result_reg, right_reg, new_result_reg);
            }
            else if (op == "/") {
                emitBytes(DIV, result_reg, right_reg, new_result_reg);
            }
            else if (op == "%") {
                emitBytes(MOD, result_reg, right_reg, new_result_reg);
            }
            else {
                throw std::runtime_error("Unsupported operator: " + op);
            }

            result_reg = new_result_reg;
        }

        return result_reg;
    }

public:
    std::vector<uint16_t> compile(const std::vector<Token>& tokens) {
        bytecode.clear();
        variables.clear();
        next_reg = 1;

        //std::cout << "Starting compilation" << std::endl;
        size_t index = 0;
        uint16_t finalResultReg = expression(tokens, index);

        emitLDR(finalResultReg, 0);

        //std::cout << "Compilation finished. Bytecode size: " << bytecode.size() << std::endl;
        return bytecode;
    }

    void print_bytecode() const {
        std::cout << "Bytecode:" << std::endl;
        std::cout << prettyBytecode(bytecode);
    }
};
