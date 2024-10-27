#include "cvm.hpp"
#include <cctype>
#include <cstddef>
#include <unordered_map>
#include <vector>
#include <string>

enum class TokenType {
    IDENTIFIER,
    OPERATOR,
    STRING,
    NUMBER,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    FUNCTION,
    RETURN,
    IF,
    ELSE,
    COMMA,
    PREFIX,
    POSTFIX,
    EQUALS,
    FALSE,
    SEMI,
    TYPE,
    TRUE,
    EOS,
};

typedef struct Token {
    TokenType type;
    std::string value;
    size_t line, col;

    Token(const TokenType& type, const std::string& value, size_t line, size_t col)
        : type(type), value(value), line(line), col(col) {}
} Token;

class Lexer {
private:
    std::string source;
    size_t position, l = 0, c;  // l = line, c = column
    char current;

    std::unordered_map<std::string, TokenType> keywords;

public:
    Lexer(const std::string& source) : source(source), position(0), current(source.empty() ? '\0' : source[0]) {
        keywords = {
            {"true", TokenType::TRUE},
            {"false", TokenType::FALSE},
            {"null", TokenType::TYPE},
            {"void", TokenType::TYPE},
            {"int", TokenType::TYPE},
            {"bool", TokenType::TYPE},
            {"string", TokenType::TYPE},
            {"if", TokenType::IF},
            {"else", TokenType::ELSE},
            {"fn", TokenType::FUNCTION},
            {"return", TokenType::RETURN},
        };
    }

    std::vector<Token> generate() {
        std::vector<Token> tokens;
        
        while (not_end()) {
            if (std::isspace(current)) {
                advance();
                continue;
            } else {
                switch (current) {
                    case '+':
                        advance();
                        if (current == '+') {
                            tokens.emplace_back(nt(TokenType::PREFIX, "++"));
                            advance();
                        } else {
                            tokens.emplace_back(nt(TokenType::OPERATOR, "+"));
                        }
                        break;

                    case '-':
                        advance();
                        if (current == '-') {
                            tokens.emplace_back(nt(TokenType::PREFIX, "--"));
                            advance();
                        } else {
                            tokens.emplace_back(nt(TokenType::OPERATOR, "-"));
                        }
                        break;

                    case '/': {
                        advance();
                        if (current == '/') {
                            while (not_end() && current != '\n') {
                                advance();
                            }

                            if (current == '\n') { 
                                l++; 
                                c = 1; 
                                advance(); 
                            }
                        } else {
                            tokens.emplace_back(nt(TokenType::OPERATOR, "/"));
                        }
                        break;
                    }

                    case '*':
                    case '%':
                        tokens.emplace_back(nt(TokenType::OPERATOR, std::string(1, current)));
                        advance();
                        break;

                    case '(':
                        tokens.emplace_back(nt(TokenType::LPAREN, "("));
                        advance();
                        break;

                    case ')':
                        tokens.emplace_back(nt(TokenType::RPAREN, ")"));
                        advance();
                        break;

                    case ';':
                        tokens.emplace_back(nt(TokenType::SEMI, ";"));
                        advance();
                        break;

                    case '[':
                        tokens.emplace_back(nt(TokenType::LBRACKET, "["));
                        advance();
                        break;

                    case ']':
                        tokens.emplace_back(nt(TokenType::RBRACKET, "]"));
                        advance();
                        break;

                    case '{':
                        tokens.emplace_back(nt(TokenType::LBRACE, "{"));
                        advance();
                        break;

                    case ',':
                        tokens.emplace_back(nt(TokenType::COMMA, ","));
                        advance();
                        break;

                    case '}':
                        tokens.emplace_back(nt(TokenType::RBRACE, "}"));
                        advance();
                        break;

                    case '<':
                        advance();
                        if (current == '=') {
                            tokens.emplace_back(nt(TokenType::OPERATOR, "<="));
                            advance();
                        } else {
                            tokens.emplace_back(nt(TokenType::OPERATOR, "<"));
                        }
                        break;

                    case '>':
                        advance();
                        if (current == '=') {
                            tokens.emplace_back(nt(TokenType::OPERATOR, ">="));
                            advance();
                        } else {
                            tokens.emplace_back(nt(TokenType::OPERATOR, ">"));
                        }
                        break;

                    case '!':
                        advance();
                        if (current == '=') {
                            tokens.emplace_back(nt(TokenType::OPERATOR, "!="));
                            advance();
                        } else {
                            tokens.emplace_back(nt(TokenType::PREFIX, "!"));
                        }
                        break;

                    case '=':
                        advance();
                        if (current == '=') {
                            tokens.emplace_back(nt(TokenType::OPERATOR, "=="));
                            advance();
                        } else {
                            tokens.emplace_back(nt(TokenType::EQUALS, "="));
                        }
                        break;

                    case '"': {
                        advance();
                        std::string value;
                        while (not_end() && current != '"') {
                            if (current == '\\') {
                                advance();
                                switch (current) {
                                    case 'n': value += '\n'; break;
                                    case 't': value += '\t'; break;
                                    case 'r': value += '\r'; break;
                                    case '"': value += '"'; break;
                                    case '\\': value += '\\'; break;
                                    default: value += current;
                                }
                            } else {
                                value += current;
                            }
                            advance();
                        }

                        if (current == '"') {
                            advance();
                            tokens.emplace_back(nt(TokenType::STRING, value));
                        } else {
                            throw Error("Unterminated string literal.");
                        }
                        
                        break;
                    }

                    default:
                        if (std::isdigit(current)) {
                            std::string value;
                            while (not_end() && std::isdigit(current)) {
                                value.push_back(current);
                                advance();
                            }
                            tokens.emplace_back(nt(TokenType::NUMBER, value));
                        } else if (std::isalpha(current) || current == '_') {
                            std::string value;
                            while (not_end() && (std::isalpha(current) || current == '_')) {
                                value.push_back(current);
                                advance();
                            }
                            
                            auto keyword = keywords.find(value);
                            if (keyword != keywords.end()) {
                                tokens.emplace_back(nt(keyword->second, value));
                            } else {
                                tokens.emplace_back(nt(TokenType::IDENTIFIER, value));
                            }
                        } else {
                            throw Error("Unknown character at '" + std::string(1, current) + "' (ln " + std::to_string(static_cast<int>(l)) + ", col " + std::to_string(static_cast<int>(c)) + ")");
                        }

                        break;
                }
            }
        }
        
        tokens.emplace_back(nt(TokenType::EOS, ""));
        return tokens;
    }

private:
    Token nt(const TokenType& type, const std::string& value) const {
        return Token(type, value, l, c);
    }

    bool not_end() {
        return position < source.size();
    }

    void advance() {
        if (current == '\n') {
            l++; 
            c = 1;
        } else {
            c++;
        }
        position++;
        current = position < source.size() ? source[position] : '\0';
    }
};