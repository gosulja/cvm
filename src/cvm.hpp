#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <string>
#include <sys/types.h>
#include <vector>
#include <array>
#include <memory>
#include <stdexcept>

#include "ctypes.hpp"
#include "opcodes.hpp"
#include "common.hpp"

class Frame;
class VMStack;

class Error : public std::runtime_error {
public:
    explicit Error(const std::string& msg) : std::runtime_error("[CVM::Error] " + msg) {}
};

class OperandStack {
private:
    static const size_t          MAX_STACK = 256;
    std::array<Value, MAX_STACK> stack;
    int                          top = -1;

public:
    void push(const Value& value) {
        if (top >= static_cast<int>(MAX_STACK) - 1) {
            throw Error("Stack overflow.");
        }

        stack[++top] = value;
    }    

    Value pop() {
        if (top < 0) {
            throw Error("Stack underflow.");
        }

        return stack[top--];
    }

    Value& peek(int distance = 0) {
        int index = top - distance;
        if (index < 0) {
            throw Error("Stack underflow with peek.");
        }

        return stack[index];
    }

    void clear() {
        top = -1;
    }

    bool is_empty() const {
        return top == -1;
    }

    size_t size() const {
        return top + 1;
    }
};

class Frame {
private:
    static const size_t           MAX_LOCALS = 256;
    std::array<Value, MAX_LOCALS> locals;
    OperandStack                  op_stack;
    std::vector<uint8_t>&         bytecode;
    size_t                        ip = 0;

public:
    explicit Frame(std::vector<uint8_t>& bcode) : bytecode(bcode) {}

    uint8_t readByte() {
        if (ip >= bytecode.size()) {
            throw Error("Unexpected end of bytecode.");
        }

        return bytecode[ip++];
    }

    void push(const Value& value) {
        op_stack.push(value);
    }

    Value pop() {
        return op_stack.pop();
    }

    Value& peek(int distance = 0) {
        return op_stack.peek(distance);
    }

    void setLocal(uint16_t index, const Value& value) {
        if (index >= MAX_LOCALS) {
            throw Error("Local variable index out of bounds.");
        }

        locals[index] = value;
    }

    Value getLocal(uint16_t index) {
        if (index >= MAX_LOCALS) {
            throw Error("Local variable index out of bounds.");
        }

        return locals[index];
    }

    size_t getIP() const { return ip; }
    void setIP(size_t v) { ip = v; }
    bool more_insts() const { return ip < bytecode.size(); } // does the bytecode have more instructions??

    Value getResult() {
        if (op_stack.is_empty()) {
            throw Error("No result on stack.");
        }

        return op_stack.peek();
    }
};

class CVM {
private:
    std::vector<uint8_t>    bytecode;
    std::unique_ptr<Frame>  cur_frame;
    
    // debug values
    bool                    debug = false;

    void unary(const OpCode& op) {
        Value a = cur_frame->pop();
        Value result;

        switch (op) {
            case OpCode::NOT: {
                if (debug) {
                    print("NOT operation on: " + a.debug_string());
                }
                
                if (a.type == Type::BOOL) {
                    result = Value(!a.bvalue);
                } else if (a.type == Type::INT) {
                    result = Value(!static_cast<bool>(a.ivalue));
                } else {
                    throw Error("Cannot use unary operator '!' on invalid operand type.");
                }
                
                if (debug) {
                    print("NOT result: " + result.debug_string());
                }
                break;
            }

            case OpCode::INC: {
                if (a.type != Type::INT) {
                    throw Error("Cannot use unary operator '++' on non-integer operand.");
                }
                result = Value(a.ivalue + 1);
                break;
            }
            case OpCode::DEC: {
                if (a.type != Type::INT) {
                    throw Error("Cannot use unary operator '--' on non-integer operand.");
                }
                result = Value(a.ivalue - 1);
                break;
            }
            case OpCode::NEG: {
                if (a.type != Type::INT) {
                    throw Error("Cannot use unary operator '-' on non-integer operand.");
                }
                result = Value(-a.ivalue);
                break;
            }
            default:
                throw Error("Unknown unary operator.");
        }

        cur_frame->push(result);
    }

    void binary(const OpCode& op) {
        Value b = cur_frame->pop();
        Value a = cur_frame->pop();
        
        if (op == OpCode::ADD && (a.type == Type::STRING || b.type == Type::STRING)) {
            std::string result;
            
            if (a.type == Type::STRING) result += *a.svalue;
            else if (a.type == Type::INT) result += std::to_string(a.ivalue);
            else if (a.type == Type::BOOL) result += (a.bvalue ? "true" : "false");
            
            if (b.type == Type::STRING) result += *b.svalue;
            else if (b.type == Type::INT) result += std::to_string(b.ivalue);
            else if (b.type == Type::BOOL) result += (b.bvalue ? "true" : "false");
            
            cur_frame->push(Value(result));
            return;
        }
        
        if (a.type != Type::INT || b.type != Type::INT) {
            throw Error("Binary operation cannot be operated on non-integers types.");
        }

        Value result;
        switch (op) {
            case OpCode::ADD: result = Value(a.ivalue + b.ivalue); break;
            case OpCode::SUB: result = Value(a.ivalue - b.ivalue); break;
            case OpCode::MUL: result = Value(a.ivalue * b.ivalue); break;
            case OpCode::DIV: result = Value(a.ivalue / b.ivalue); break;
            case OpCode::MOD: result = Value(a.ivalue % b.ivalue); break;
            default: throw Error("Unknown binary operator.");
        }

        cur_frame->push(result);
    }

    void debug_stack() {
        if (!debug) return;

        print("stack: [");
        try {
            for (size_t i = 0; i < 4; i++) {
                try {
                    Value val = cur_frame->peek(i);
                    print("     " + std::to_string(val.ivalue) + ",");
                } catch (const Error& e) {
                    break;
                }
            }
        } catch (const std::exception&) {}

        print("]");
    }

    static std::string to_hex(uint8_t val) {
        std::stringstream ss;
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(val);
        return ss.str();
    }

    void print_value(const Value& value) {
        switch (value.type) {
            case Type::INT:
                std::cout << value.ivalue;
                break;
            case Type::BOOL:
                std::cout << (value.bvalue ? "true" : "false");
                break;
            case Type::STRING:
                std::cout << *value.svalue;
                break;
            case Type::ARRAY:
                std::cout << &value.avalue;
                break;
            case Type::VECTOR:
                std::cout << &value.vvalue;
                break;
        }
        std::cout << std::endl;
    }

public:
    CVM(const std::vector<uint8_t>& bcode, bool debug = false)
        : bytecode(bcode), debug(debug) {}

    void execute() {
        cur_frame = std::make_unique<Frame>(bytecode);

        while (cur_frame->more_insts()) {
            debug_stack();

            uint8_t inst = cur_frame->readByte();
            OpCode opc = static_cast<OpCode>(inst);

            try {
                switch (opc) {
                    case OpCode::PUSHK: {
                        uint8_t type_byte = cur_frame->readByte();
                        // "mark" its a string
                        if (type_byte == 0xFF) {
                            if (debug) {
                                print("pushing a string constant");
                            }

                            std::string str;
                            uint8_t c;
                            while ((c = cur_frame->readByte()) != 0) {
                                str += static_cast<char>(c);
                            }
                            cur_frame->push(Value(str));
                        } else {
                            // int consts
                            int value = (type_byte << 24) |
                                      (cur_frame->readByte() << 16) |
                                      (cur_frame->readByte() << 8) |
                                      cur_frame->readByte();
                            cur_frame->push(Value(value));
                        }
                        break;
                    }
                    case OpCode::LOAD: {
                        uint8_t index = cur_frame->readByte();
                        Value value = cur_frame->getLocal(index);
                        cur_frame->push(value);
                        break;
                    }
                    case OpCode::STORE: {
                        uint8_t index = cur_frame->readByte();
                        Value value = cur_frame->peek();
                        cur_frame->setLocal(index, value);
                        break;
                    }
                    case OpCode::PUSH: {
                        uint8_t val = cur_frame->readByte();

                        if (debug) {
                            print("PUSH: raw byte = 0x" + to_hex(val));
                        }
                        
                        if (val & 0x80) {
                            bool bvalue = (val & 0x01) != 0; // extract the lowest bit for the boolean value
                            if (debug) {
                                print("Pushing boolean: " + std::string(bvalue ? "true" : "false"));
                            }
                            cur_frame->push(Value(bvalue));
                        } else {
                            if (debug) {
                                print("Pushing integer: " + std::to_string(val));
                            }
                            cur_frame->push(Value(static_cast<int>(val)));
                        }
                        break;
                    }
                    case OpCode::ADD:
                    case OpCode::SUB:
                    case OpCode::MUL:
                    case OpCode::DIV:
                    case OpCode::MOD:
                        binary(opc);
                        break;
                    case OpCode::NOT:
                    case OpCode::INC:
                    case OpCode::DEC:
                    case OpCode::NEG:
                        unary(opc);
                        break;
                    case OpCode::MKARR: {
                        uint8_t type_byte = cur_frame->readByte();
                        Type e_type = static_cast<Type>(type_byte);
                        ArrayValue arr(e_type);
                        cur_frame->push(Value(arr));
                        break;
                    }
                    case OpCode::MKVEC: {
                        uint8_t type_byte = cur_frame->readByte();
                        Type e_type = static_cast<Type>(type_byte);
                        VectorValue vec(e_type);
                        cur_frame->push(Value(vec));
                        break;
                    }
                    case OpCode::APUSH: {
                        Value elem = cur_frame->pop();
                        Value arr = cur_frame->pop();

                        if (arr.type == Type::ARRAY) {
                            arr.avalue->elements.push_back(elem);
                        } else if (arr.type == Type::VECTOR) {
                            arr.vvalue->elements.push_back(elem);
                        } else {
                            throw Error("Cannot push to non-array type.");
                        }

                        cur_frame->push(arr);
                        break;
                    }
                    case OpCode::GETIDX: {
                        Value idx = cur_frame->pop();
                        Value arr = cur_frame->pop();

                        if (idx.type != Type::INT) {
                            throw Error("Array index must be a numeric literal.");
                        }

                        if (arr.type == Type::ARRAY) {
                            Value elem = arr.avalue->get(idx.ivalue);
                            cur_frame->push(elem);
                        } else if (arr.type == Type::VECTOR) {
                            Value elem = arr.vvalue->get(idx.ivalue);
                            cur_frame->push(elem);
                        } else {
                            print(std::to_string(static_cast<int>(arr.type)));
                            throw Error("Cannot index non-array type.");
                        }

                        break;
                    }
                    case OpCode::SETIDX: {
                        Value value = cur_frame->pop();
                        Value idx = cur_frame->pop();
                        Value arr = cur_frame->pop();
                        
                        if (idx.type != Type::INT) {
                            throw Error("Array index must be a numeric literal.");
                        }
                        
                        if (arr.type == Type::ARRAY) {
                            arr.avalue->set(idx.ivalue, value);
                        } else if (arr.type == Type::VECTOR) {
                            arr.vvalue->set(idx.ivalue, value);
                        } else {
                            throw Error("Cannot index non-array type.");
                        }
                        
                        cur_frame->push(arr);
                        break;
                    }
                    case OpCode::ASIZE: {
                        Value arr = cur_frame->pop();
        
                        if (arr.type == Type::ARRAY) {
                            cur_frame->push(Value(static_cast<int>(arr.avalue->size())));
                        } else if (arr.type == Type::VECTOR) {
                            cur_frame->push(Value(static_cast<int>(arr.vvalue->size())));
                        } else {
                            throw Error("Cannot get size of non-array type.");
                        }
                        break;
                    }
                    case OpCode::VBACK: {
                        print("Warning: back() function is deprecated and should not be used.");
                        // Value value = cur_frame->pop();
                        // Value vec = cur_frame->peek();
                        
                        // if (vec.type != Type::VECTOR) {
                        //     throw Error("back() can only be used with vectors.");
                        // }
                        
                        // vec.vvalue->push_back(value);
                        // cur_frame->push(vec);
                        break;
                    }
                    case OpCode::PRINT: {
                        Value value = cur_frame->peek();
                        print_value(value);
                        cur_frame->pop();
                        break;
                    }
                    case OpCode::HALT:
                        if (debug) {
                            print("cvm halted.");
                        }
                        return;
                    default:
                        throw Error("Unknown opcode: " + std::to_string(inst));
                }
            } catch (const Error& e) {
                print("Runtime error at ip=" + std::to_string(cur_frame->getIP()) + 
                      ": " + std::string(e.what()));
                throw;
            }
        }
    }

    Value getResult() {
        if (!cur_frame) {
            throw Error("No frame available");
        }
        return cur_frame->getResult();
    }

    std::string getResultAsString() {
        if (!cur_frame) {
            throw Error("No frame available");
        }

        Value result = cur_frame->getResult();
        switch (result.type) {
            case Type::INT:
                return std::to_string(result.ivalue);
            case Type::BOOL:
                return result.bvalue ? "true" : "false";
            case Type::STRING:
                print("Found string");
                return *result.svalue;
            default: return "UNKNOWN";
        }
    }
};
