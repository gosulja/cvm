#include <stdexcept>
#include <vector>
#include <string>
#include <memory>
#include <variant>

// cvm types
#pragma once

enum class Type {
    INT,
    BOOL,
    STRING,
    ARRAY,
    VECTOR,
};

class Value;

class ArrayValue {
public:
    Type element_type;
    std::vector<Value> elements;
    
    ArrayValue(Type type) : element_type(type) {}
    
    void set(size_t index, const Value& value);
    Value get(size_t index) const;
    size_t size() const { return elements.size(); }
};

class VectorValue {
public:
    Type element_type;
    std::vector<Value> elements;
    
    VectorValue(Type type) : element_type(type) {}
    
    void set(size_t index, const Value& value);
    Value get(size_t index) const;
    void push_back(const Value& value);
    size_t size() const { return elements.size(); }
};

// Updated Value class with std::variant
class Value {
public:
    Type type;
    
    using ValueType = std::variant<int, bool, std::shared_ptr<std::string>, 
                                   std::shared_ptr<ArrayValue>, std::shared_ptr<VectorValue>>;
    union {
        int ivalue;
        bool bvalue;
        std::string* svalue;
        ArrayValue* avalue;
        VectorValue* vvalue;
    };

    Value() : type(Type::INT), ivalue(0) {}
    explicit Value(int v) : type(Type::INT), ivalue(v) {}
    explicit Value(bool v) : type(Type::BOOL), bvalue(v) {}
    explicit Value(const std::string& v) : type(Type::STRING), svalue(new std::string(v)) {}
    explicit Value(const ArrayValue& v) : type(Type::ARRAY), avalue(new ArrayValue(v)) {}
    explicit Value(const VectorValue& v) : type(Type::VECTOR), vvalue(new VectorValue(v)) {}

    ~Value() {
        switch (type) {
            case Type::STRING:
                delete svalue;
                break;
            case Type::ARRAY:
                delete avalue;
                break;
            case Type::VECTOR:
                delete vvalue;
                break;
            default:
                break;
        }
    }

    std::string debug_string() const {
        switch (type) {
            case Type::BOOL:
                return std::string("BOOL:") + (bvalue ? "true" : "false");
            case Type::INT:
                return std::string("INT:") + std::to_string(ivalue);
            case Type::STRING:
                return std::string("STRING:\"") + *svalue + "\"";
            case Type::ARRAY:
                return std::string("ARRAY[size=" + std::to_string(avalue->size()) + "]");
            case Type::VECTOR:
                return std::string("VECTOR[size=" + std::to_string(vvalue->size()) + "]");
            default:
                return "UNKNOWN";
        }
    }

    Value(const Value& other) : type(other.type) {
        switch (type) {
            case Type::INT:
                ivalue = other.ivalue;
                break;
            case Type::BOOL:
                bvalue = other.bvalue;
                break;
            case Type::STRING:
                svalue = new std::string(*other.svalue);
                break;
            case Type::ARRAY:
                avalue = new ArrayValue(*other.avalue);
                break;
            case Type::VECTOR:
                vvalue = new VectorValue(*other.vvalue);
                break;
        }
    }

    Value& operator=(const Value& other) {
        if (this != &other) {
            this->~Value();
            type = other.type;
            switch (type) {
                case Type::INT:
                    ivalue = other.ivalue;
                    break;
                case Type::BOOL:
                    bvalue = other.bvalue;
                    break;
                case Type::STRING:
                    svalue = new std::string(*other.svalue);
                    break;
                case Type::ARRAY:
                    avalue = new ArrayValue(*other.avalue);
                    break;
                case Type::VECTOR:
                    vvalue = new VectorValue(*other.vvalue);
                    break;
            }
        }
        return *this;
    }
};

inline void ArrayValue::set(size_t index, const Value& value) {
    if (index >= elements.size()) {
        throw std::runtime_error("[cvm] Array index out of bounds");
    }
    if (value.type != element_type) {
        throw std::runtime_error("[cvm] Type mismatch in array assignment");
    }
    elements[index] = value;
}

inline Value ArrayValue::get(size_t index) const {
    if (index >= elements.size()) {
        throw std::runtime_error("[cvm] Array index out of bounds");
    }
    return elements[index];
}

inline void VectorValue::set(size_t index, const Value& value) {
    if (value.type != element_type) {
        throw std::runtime_error("[cvm] Type mismatch in vector assignment");
    }
    if (index >= elements.size()) {
        elements.resize(index + 1);
    }
    elements[index] = value;
}

inline Value VectorValue::get(size_t index) const {
    if (index >= elements.size()) {
        throw std::runtime_error("[cvm] Vector index out of bounds");
    }
    return elements[index];
}

inline void VectorValue::push_back(const Value& value) {
    if (value.type != element_type) {
        throw std::runtime_error("[cvm] Type mismatch in vector push_back");
    }
    elements.push_back(value);
}
