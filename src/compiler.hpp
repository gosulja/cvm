#pragma once

#include <cstddef>
#include <cstdint>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>

#include "ctypes.hpp"
#include "lexer.hpp"
#include "opcodes.hpp"

struct Parameter {
    std::string symbol;
    Type type;
};

struct Function {
    std::string name;
    Type return_type;
    std::vector<Parameter> params;
    size_t bytecode_offset;
    size_t local_count;
};

class Compiler {
private:
    std::vector<Token>   tokens;
    size_t               current;
    std::vector<uint8_t> bytecode;

    std::unordered_map<std::string, size_t> variables;
    size_t                                  var_count = 0;

    std::unordered_map<std::string, Function> functions;
    Function*                                 current_function = nullptr;
    Type                                      current_ret_type = Type::VOID;
    bool                                      has_returned = false;

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

    uint8_t emitJump(OpCode inst) {
        emitByte(static_cast<uint8_t>(inst));
        // 0xff placeholders for jump offset.
        emitByte(0xFF);
        emitByte(0xFF);
        return bytecode.size() - 2;
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

    void function() {
        if (!match(TokenType::IDENTIFIER)) {
            throw std::runtime_error("Expected function name after 'fn' keyword.");
        }

        std::string func_name = previous().value;

        if (functions.find(func_name) != functions.end()) {
            throw std::runtime_error("Function '" + func_name + "' already declared.");
        }

        if (!match(TokenType::LPAREN)) {
            throw std::runtime_error("Expected '(' after function name.");
        }

        Function func;
        func.name = func_name;

        functions[func_name] = func;

        if (!check(TokenType::RPAREN)) {
            do {
                if (!match(TokenType::TYPE)) {
                    throw std::runtime_error("Expected parameter type.");
                }

                std::string param_type = previous().value;
                Type type;

                if (param_type == "int") type = Type::INT;
                else if (param_type == "string") type = Type::STRING;
                else if (param_type == "bool") type = Type::BOOL;
                else throw std::runtime_error("Invalid parameter type.");

                if (!match(TokenType::IDENTIFIER)) {
                    throw std::runtime_error("Expected parameter name.");
                }

                std::string param_name = previous().value;
                func.params.push_back({param_name, type});
            } while (match(TokenType::COMMA));
        }
    
        if (!match(TokenType::RPAREN)) {
            throw std::runtime_error("Expected ')' after parameters.");
        }

        if (!match(TokenType::TYPE)) {
            throw std::runtime_error("Expected return type.");
        }

        std::string return_type = previous().value;
        if (return_type == "void") func.return_type = Type::VOID;
        else if (return_type == "int") func.return_type = Type::INT;
        else if (return_type == "string") func.return_type = Type::STRING;
        else if (return_type == "bool") func.return_type = Type::BOOL;
        else throw std::runtime_error("Invalid return type.");

        func.bytecode_offset = bytecode.size();

        current_function = &functions[func_name];
        current_ret_type = func.return_type;
        has_returned = false;

        if (!match(TokenType::LBRACE)) {
            throw std::runtime_error("Expected '{' before function body.");
        }

        emitByte(static_cast<uint8_t>(OpCode::ENTER));
        size_t locals_pos = bytecode.size();
        emitByte(0x0);

        var_count = 0;
        variables.clear();
        for (const auto& param : func.params) {
            variables[param.symbol] = var_count++;
        }

        while (!check(TokenType::RBRACE) && !is_at_end()) {
            statement();
        }

        bytecode[locals_pos] = static_cast<uint8_t>(var_count);

        if (!has_returned && current_ret_type != Type::VOID) {
            throw std::runtime_error("Function '" + func_name + "' must return a value.");
        }

        if (!has_returned) {
            emitByte(static_cast<uint8_t>(OpCode::PUSH));
            emitByte(0x0); // for void
            emitByte(static_cast<uint8_t>(OpCode::RET));
        }

        if (!match(TokenType::RBRACE)) {
            throw std::runtime_error("Expected '}' after function body.");
        }

        current_function = nullptr;
        current_ret_type = Type::VOID;
        has_returned = false;
    }

    void return_statement() {
        if (current_function == nullptr) {
            throw std::runtime_error("Cannot return from global scope.");
        }

        if (current_ret_type != Type::VOID) {
            expression();

            Type type;
            if (peek().value == "int") type = Type::INT;
            else if (peek().value == "bool") type = Type::BOOL;
            else if (peek().value == "string") type = Type::STRING;
            else throw std::runtime_error("Unknown return type.");

            if (type != current_ret_type) {
                throw std::runtime_error("Return value type doesn't match function return type.");
            }
        }

        if (!match(TokenType::SEMI)) {
            throw std::runtime_error("Expected ';' after return value.");
        }

        emitByte(static_cast<uint8_t>(OpCode::RET));
        has_returned = true;
    }

    void call() {
        std::string func_name = previous().value;
        
        if (func_name == "print" || func_name == "size") {
            if (!match(TokenType::LPAREN)) {
                throw std::runtime_error("Expected '(' after function name.");
            }

            if (func_name == "print") {
                size_t arg_count = 0;
                if (!check(TokenType::RPAREN)) {
                    do {
                        expression();
                        arg_count++;
                    } while (match(TokenType::COMMA));
                }

                if (!match(TokenType::RPAREN)) {
                    throw std::runtime_error("Expected ')' after print arguments.");
                }

                emitByte(static_cast<uint8_t>(OpCode::PRINT));
                emitByte(static_cast<uint8_t>(arg_count));
                return;
            }
            
            if (func_name == "size") {
                if (check(TokenType::RPAREN)) {
                    throw std::runtime_error("size() requires one array argument.");
                }

                expression();

                if (!match(TokenType::RPAREN)) {
                    throw std::runtime_error("Expected ')' after size argument.");
                }

                emitByte(static_cast<uint8_t>(OpCode::ASIZE));
                return;
            }
        }

        if (functions.find(func_name) == functions.end()) {
            throw std::runtime_error("Undefined function '" + func_name + "'");
        }

        Function& func = functions[func_name];

        if (!match(TokenType::LPAREN)) {
            throw std::runtime_error("Expected '(' after function name.");
        }

        size_t arg_count = 0;
        if (!check(TokenType::RPAREN)) {
            do {
                if (arg_count >= func.params.size()) {
                    throw std::runtime_error("Too many arguments to function '" + func_name + "'");
                }

                expression();
                
                Type type;
                if (peek().value == "int") type = Type::INT;
                else if (peek().value == "bool") type = Type::BOOL;
                else if (peek().value == "string") type = Type::STRING;
                else throw std::runtime_error("Unknown return type: " + std::to_string(static_cast<int>(type)));

                if (type != func.params[arg_count].type) {
                    throw std::runtime_error("Argument type mismatch.");
                }

                arg_count++;
            } while (match(TokenType::COMMA));
        }

        if (arg_count != func.params.size()) {
            throw std::runtime_error("Wrong number of arguments to function '" + func_name + "'");
        }

        if (!match(TokenType::RPAREN)) {
            throw std::runtime_error("Expected ')' after arguments.");
        }

        emitByte(static_cast<uint8_t>(OpCode::CALL));

        // function offset
        emitByte(static_cast<uint8_t>((func.bytecode_offset >> 24) & 0xFF));
        emitByte(static_cast<uint8_t>((func.bytecode_offset >> 16) & 0xFF));
        emitByte(static_cast<uint8_t>((func.bytecode_offset >> 8) & 0xFF));
        emitByte(static_cast<uint8_t>(func.bytecode_offset & 0xFF));
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
        else if (op == ">") emitByte(static_cast<uint8_t>(OpCode::GT));
        else if (op == "<") emitByte(static_cast<uint8_t>(OpCode::LT));
        else if (op == ">=") emitByte(static_cast<uint8_t>(OpCode::GTE));
        else if (op == "<=") emitByte(static_cast<uint8_t>(OpCode::LTE));
        else if (op == "==") emitByte(static_cast<uint8_t>(OpCode::EQ));
        else if (op == "!=") emitByte(static_cast<uint8_t>(OpCode::NEQ));
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

    // patch jump
    void patch(size_t offset) {
        size_t current_pos = bytecode.size();
        size_t jump_amt = current_pos - offset;

        if (jump_amt > 0xFFFF) {
            throw std::runtime_error("Jump offset too large.");
        }

        bytecode[offset] = static_cast<uint8_t>((jump_amt >> 8) & 0xFF);
        bytecode[offset + 1] = static_cast<uint8_t>(jump_amt & 0xFF);
    }

    void block() {
        if (!match(TokenType::LBRACE)) {
            throw std::runtime_error("Expected '{' before block.");
        }

        while (!check(TokenType::RBRACE) && !is_at_end()) {
            statement();
        }

        if (!match(TokenType::RBRACE)) {
            throw std::runtime_error("Expected '}' after block.");
        }
    }

    void if_statement() {
        expression();

        size_t then_jump = emitJump(OpCode::JMPF);

        block();

        size_t else_jump = 0;
        if (match(TokenType::ELSE)) {
            else_jump = emitJump(OpCode::JMP);

            patch(then_jump);
            block();
            patch(else_jump);
        } else {
            patch(then_jump);
        }
    }

    void statement() {
        if (match(TokenType::FUNCTION)) {
            function();
        } else if (match(TokenType::RETURN)) {
            return_statement();
        } else if (match(TokenType::IF)) {
            if_statement();
        } else if (match(TokenType::TYPE)) {
            current--;
            declaration();
        } else {
            expression();

            if (!match(TokenType::SEMI)) {
                throw std::runtime_error("Expected ';' after statement.");
            }
        }
    }

public:
    Compiler(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}
    
    std::vector<uint8_t> compile() {
        bytecode.clear();

        while (!is_at_end()) {
            statement();
        }

        emitByte(static_cast<uint8_t>(OpCode::HALT));

        return bytecode;
    }
};