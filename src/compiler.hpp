#pragma once

#include <cstdint>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>

#include "ctypes.hpp"
#include "lexer.hpp"
#include "opcodes.hpp"

class Compiler {
private:
    std::vector<Token>   tokens;
    size_t               current;
    std::vector<uint8_t> bytecode;

    std::unordered_map<std::string, size_t> variables;
    size_t                                  var_count = 0;

    Token peek() const {
        return tokens[current];
    }

    Token previous() const {
        return tokens[current - 1];
    }

    Token advance() {
        if (!is_at_end()) current++;
        return previous();
    }

    bool is_at_end() const {
        return peek().type == TokenType::EOS;
    }

    bool check(const TokenType& type) const {
        if (is_at_end()) return false;
        return peek().type == type;
    }

    bool match(const TokenType& type) {
        if (check(type)) {
            advance();
            return true;
        }

        return false;
    }

    bool is_keyword(const std::string& w) {
        return w == "int" || w == "string" || w == "bool" || w == "true" || w == "false";
    }

    void emitByte(uint8_t byte) {
        bytecode.push_back(byte);
    }

    void emitBytes(uint8_t byte1, uint8_t byte2) {
        emitByte(byte1);
        emitByte(byte2);
    }

    void emitConstant(int value) {
        if (value <= 255) {
            emitByte(static_cast<uint8_t>(OpCode::PUSH));
            emitByte(static_cast<uint8_t>(value));
        } else {
            emitByte(static_cast<uint8_t>(OpCode::PUSHK));
            emitByte(static_cast<uint8_t>((value >> 24) & 0xFF));
            emitByte(static_cast<uint8_t>((value >> 16) & 0xFF));
            emitByte(static_cast<uint8_t>((value >> 8) & 0xFF));
            emitByte(static_cast<uint8_t>(value & 0xFF));
        }
    }

    void array(bool is_vec) {
        if (!match(TokenType::EQUALS)) {
            throw std::runtime_error("Expected '=' after array declaration.");
        }

        if (!match(TokenType::LBRACE)) {
            throw std::runtime_error("Expected '{' to start array literal.");
        }

        std::string type_str = tokens[current - 6].value;
        Type e_type;

        if (type_str == "int") e_type = Type::INT;
        else if (type_str == "bool") e_type = Type::BOOL;
        else if (type_str == "string") e_type = Type::STRING;
        else throw std::runtime_error("Invalid array element type.");

        emitByte(is_vec ? static_cast<uint8_t>(OpCode::MKVEC)
                        : static_cast<uint8_t>(OpCode::MKARR));
        emitByte(static_cast<uint8_t>(e_type));

        do {
            if (peek().type == TokenType::RBRACE) break;

            expression();
            emitByte(static_cast<uint8_t>(OpCode::APUSH));
        } while (match(TokenType::COMMA));

        if (!match(TokenType::RBRACE)) {
            throw std::runtime_error("Expected '}' after array elements.");
        }
    }

    void array_index() {
        std::string sym = previous().value;

        if (!match(TokenType::LBRACKET))
            throw std::runtime_error("Expected '[' after array name.");

        expression();

        if (!match(TokenType::RBRACKET))
            throw std::runtime_error("Expected ']' after index.");

        if (match(TokenType::EQUALS)) {
            expression();
            emitByte(static_cast<uint8_t>(OpCode::SETIDX)); // set index
        } else {
            emitByte(static_cast<uint8_t>(OpCode::GETIDX)); // get index
        }
    }

    void call() {
        std::string func_name = previous().value;

        if (!match(TokenType::LPAREN)) {
            throw std::runtime_error("Expected '(' after function name.");
        }

        if (func_name == "print") {
            if (!match(TokenType::RPAREN)) {
                expression();

                if (!match(TokenType::RPAREN)) {
                    throw std::runtime_error("Expected ')' to finish expression in function call.");
                }
            }

            emitByte(static_cast<uint8_t>(OpCode::PRINT));
        } else if (func_name == "size") {
            if (!match(TokenType::RPAREN)) {
                expression();

                if (!match(TokenType::RPAREN)) {
                    throw std::runtime_error("Expected ')' to finish expression in function call.");
                }
            }

            emitByte(static_cast<uint8_t>(OpCode::ASIZE));
        } else if (func_name == "back") {
            if (!match(TokenType::RPAREN)) {
                expression();   // first arg

                if (!match(TokenType::COMMA)) {
                    throw std::runtime_error("Expected ',' between arguments in function call.");
                }

                expression();   // second arg

                if (!match(TokenType::RPAREN)) {
                    throw std::runtime_error("Expected ')' to finish arguments in 'back' function.");
                }
            }

            emitByte(static_cast<uint8_t>(OpCode::VBACK));
        } else {
            throw std::runtime_error("Unknown function '" + func_name + "'");
        }
    }

    void number() {
        int value = std::stoi(previous().value);
        emitConstant(value);
    }

    void boolean() {
        uint8_t value = 0x80 | (previous().type == TokenType::TRUE ? 0x01 : 0x00);
        emitByte(static_cast<uint8_t>(OpCode::PUSH));
        emitByte(value);
    }

    void string() {
        std::string value = previous().value;
        emitByte(static_cast<uint8_t>(OpCode::PUSHK));
        emitByte(0xFF);
        for (char c : value) {
            emitByte(static_cast<uint8_t>(c));
        }
        emitByte(0);
    }

    void unary() {
        Token op = previous();
        expression();

        if (op.value == "!") emitByte(static_cast<uint8_t>(OpCode::NOT));
        else if (op.value == "++") emitByte(static_cast<uint8_t>(OpCode::INC));
        else if (op.value == "--") emitByte(static_cast<uint8_t>(OpCode::DEC));
        else if (op.value == "-") emitByte(static_cast<uint8_t>(OpCode::NEG));
    }

    void variable() {
        std::string name = previous().value;
        if (variables.find(name) == variables.end()) {
            throw std::runtime_error("Undefined variable '" + name + "'");
        }

        emitByte(static_cast<uint8_t>(OpCode::LOAD));
        emitByte(static_cast<uint8_t>(variables[name]));
    }

    void declaration() {
        if (!match(TokenType::TYPE)) {
            throw std::runtime_error("Expected type declaration.");
        }

        std::string type = previous().value;

        bool is_arr = false;
        bool is_vec = false;

        if (match(TokenType::LBRACKET)) {
            is_arr = true;
            if (!match(TokenType::RBRACKET)) {
                throw std::runtime_error("Expected ']' after '[' in array declaration.");
            }
        } else if (match(TokenType::LBRACE)) {
            is_vec = true;
            if (!match(TokenType::RBRACE)) {
                throw std::runtime_error("Expected '}' after '{' in vector declaration.");
            }
        }

        if (!match(TokenType::IDENTIFIER)) {
            throw std::runtime_error("Expected variable name, got type " + std::to_string(static_cast<int>(peek().type)));
        }

        std::string name = previous().value;

        if (variables.find(name) != variables.end()) {
            throw std::runtime_error("Variable '" + name + "' already declared.");
        }

        variables[name] = var_count++;

        if (is_arr || is_vec) {
            array(is_vec);
        } else if (match(TokenType::EQUALS)) {
            expression();
        } else {
            emitByte(static_cast<uint8_t>(OpCode::PUSH));
            emitByte(0);
        }

        emitByte(static_cast<uint8_t>(OpCode::STORE));
        emitByte(static_cast<uint8_t>(variables[name]));

        if (!match(TokenType::SEMI)) {
            throw std::runtime_error("Expected ';' after variable declaration.");
        }
    }

    void binary() {
        std::string op = previous().value;
        expression(); // compile right-oper

        if (op == "+") emitByte(static_cast<uint8_t>(OpCode::ADD));
        else if (op == "-") emitByte(static_cast<uint8_t>(OpCode::SUB));
        else if (op == "*") emitByte(static_cast<uint8_t>(OpCode::MUL));
        else if (op == "/") emitByte(static_cast<uint8_t>(OpCode::DIV));
        else if (op == "%") emitByte(static_cast<uint8_t>(OpCode::MOD));
    }

    void grouping() {
        expression();
        if (!match(TokenType::RPAREN))
            throw std::runtime_error("Expected ')' after grouped expression.");
    }

    void expression() {
        if (match(TokenType::NUMBER)) number();
        else if (match(TokenType::STRING)) string();
        else if (match(TokenType::TRUE) || match(TokenType::FALSE)) boolean();
        else if (match(TokenType::IDENTIFIER)) {
            std::string name = previous().value;
            if (peek().type == TokenType::LPAREN) {
                call();
            } else if (peek().type == TokenType::LBRACKET) {
                array_index();
            } else {
                variable();
            }
        }
        else if (match(TokenType::PREFIX)) unary();
        else if (match(TokenType::LPAREN)) grouping();
        else throw std::runtime_error("Expected expression.");

        while (match(TokenType::OPERATOR) || match(TokenType::POSTFIX))
            binary();
    }

public:
    Compiler(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}
    
    std::vector<uint8_t> compile() {
        bytecode.clear();

        while (!is_at_end()) {
            if (match(TokenType::TYPE)) {
                current--;
                declaration();
            } else {
                expression();

                if (!match(TokenType::SEMI)) 
                    throw std::runtime_error("Expected ';' after expression.");
            }

        }

        emitByte(static_cast<uint8_t>(OpCode::HALT));

        return bytecode;
    }
};