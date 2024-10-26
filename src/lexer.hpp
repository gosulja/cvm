#include "cvm.hpp"
#include <cctype>
#include <cstddef>
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
    // size_t line;
    // size_t col;
} Token;

class Lexer {
private:
    std::string source;
    size_t position;
    char current;

public:
    Lexer(const std::string& source) : source(source), position(0), current(source.empty() ? '\0' : source[0]) {}

    std::vector<Token> generate() {
        std::vector<Token> tokens;
        
        while (not_end()) {
            if (std::isspace(current)) {
                advance();
                continue;
            }
            
            else {
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

                    case '*':
                    case '/':
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

                    case '!':
                        tokens.emplace_back(nt(TokenType::PREFIX, "!"));
                        advance();
                        break;

                    case '=':
                        tokens.emplace_back(nt(TokenType::EQUALS, "="));
                        advance();
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
                            
                            if (value == "true") {
                                tokens.emplace_back(nt(TokenType::TRUE, value));
                            } else if (value == "false") {
                                tokens.emplace_back(nt(TokenType::FALSE, value));
                            } else if (value == "int" || value == "bool" || value == "string") {
                                tokens.emplace_back(nt(TokenType::TYPE, value));
                            } else {
                                tokens.emplace_back(nt(TokenType::IDENTIFIER, value));
                            }
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
        return Token{.type = type, .value = value};
    }

    bool not_end() {
        return position < source.size();
    }

    void advance() {
        position++;
        current = position < source.size() ? source[position] : '\0';
    }
};