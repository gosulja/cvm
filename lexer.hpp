#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cctype>
#include <unordered_map>

#define OPERATOR    0x01
#define NUMBER      0x02
#define IDENTIFIER  0x03
#define SEMI        0x04
#define ASSIGN      0x05
#define KEYWORD     0x06
#define LPAREN      0x07
#define RPAREN      0x08
#define EOS         0x00

struct Token {
    uint16_t    type;
    std::string lexeme;
    Token(uint16_t t, const std::string& v) : type(t), lexeme(v) {}
};

class Lexer {
private:
    std::string source;
    size_t position;
    char current;
    std::unordered_map<std::string, uint16_t> keywords;

public:
    Lexer(const std::string& source) : source(source), position(0), current(source[position]) {
        keywords = {
            {"true", KEYWORD},
            {"false", KEYWORD},
        };
    }

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (not_end()) {
            if (current == ' ' || current == '\n' || current == '\t') {
                advance();
            }
            else if (contains(current, "+-*/%^")) {
                tokens.emplace_back(Token{ OPERATOR, std::string(1, current) });
                advance();
            }
            else if (current == '(') {
                tokens.emplace_back(Token{ LPAREN, "(" });
                advance();
            }
            else if (current == ')') {
                tokens.emplace_back(Token{ RPAREN, ")" });
                advance();
            }
            else if (current == '=') {
                if (peek() == '=') {
                    tokens.emplace_back(Token{ OPERATOR, "==" });
                    advance();
                    advance();
                }
                else {
                    tokens.emplace_back(Token{ ASSIGN, "=" });
                    advance();
                }
            }
            else if (current == '!') {
                if (peek() == '=') {
                    tokens.emplace_back(Token{ OPERATOR, "!=" });
                    advance();
                    advance();
                }
                else {
                    tokens.emplace_back(Token{ OPERATOR, "!" });
                    advance();
                }
            }
            else if (current == '>') {
                if (peek() == '=') {
                    tokens.emplace_back(Token{ OPERATOR, ">=" });
                    advance();
                    advance();
                }
                else {
                    tokens.emplace_back(Token{ OPERATOR, ">" });
                    advance();
                }
            }
            else if (current == '<') {
                if (peek() == '=') {
                    tokens.emplace_back(Token{ OPERATOR, "<=" });
                    advance();
                    advance();
                }
                else {
                    tokens.emplace_back(Token{ OPERATOR, "<" });
                    advance();
                }
            }
            else if (current == '&' && peek() == '&') {
                tokens.emplace_back(Token{ OPERATOR, "&&" });
                advance();
                advance();
            }
            else if (current == '|' && peek() == '|') {
                tokens.emplace_back(Token{ OPERATOR, "||" });
                advance();
                advance();
            }
            else if (current == ';') {
                tokens.emplace_back(Token{ SEMI, ";" });
                advance();
            }
            else if (std::isdigit(current)) {
                tokens.emplace_back(number());
            }
            else if (std::isalpha(current) || current == '_') {
                tokens.emplace_back(identifier());
            }
            else {
                advance();
            }
        }
        tokens.emplace_back(Token{ EOS, "" });
        return tokens;
    }

    bool contains(char c, const char* candidate) {
        for (const char* p = candidate; *p != '\0'; ++p) {
            if (*p == c) return true;
        }
        return false;
    }

    bool not_end() {
        return position < source.size();
    }

    void advance() {
        position++;
        if (not_end()) {
            current = source[position];
        }
        else {
            current = '\0';
        }
    }

    char peek() {
        if (position + 1 < source.size()) {
            return source[position + 1];
        }
        return '\0';
    }

    Token number() {
        std::string value;
        while (not_end() && std::isdigit(current)) {
            value.push_back(current);
            advance();
        }

        if (current == '.' && std::isdigit(peek())) {
            value.push_back(current);
            advance();
            while (not_end() && std::isdigit(current)) {
                value.push_back(current);
                advance();
            }
        }
        return Token{ NUMBER, value };
    }

    Token identifier() {
        std::string value;
        while (not_end() && (std::isalpha(current) || current == '_')) {
            value.push_back(current);
            advance();
        }
        auto it = keywords.find(value);
        if (it != keywords.end()) {
            return Token{ it->second, value };
        }
        return Token{ IDENTIFIER, value };
    }
};