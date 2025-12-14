#include "../include/Lexer.h"
#include <cctype>
#include <iostream>
#include <map>

char Lexer::peek(int offset) const {
    if (pos + offset >= src.length()) return '\0';
    return src[pos + offset];
}

char Lexer::advance() {
    char current = src[pos];
    pos++;
    if (current == '\n') line++;
    return current;
}

bool Lexer::isAtEnd() const {
    return pos >= src.length();
}

Token Lexer::makeToken(TokenType type, std::string value) {
    return {type, value, line};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        char c = peek();

        if (isspace(c)) { advance(); continue; }

        // Comentarios simples
        if (c == '/' && peek(1) == '/') {
            while (peek() != '\n' && !isAtEnd()) advance();
            continue;
        }
        // Comentarios multi-linea <begin>...<end>
        if (c == '<' && src.substr(pos, 7) == "<begin>") {
            for(int i=0; i<7; i++) advance(); 
            while (!isAtEnd()) {
                if (peek() == '<' && src.substr(pos, 5) == "<end>") {
                    for(int i=0; i<5; i++) advance();
                    break;
                }
                advance();
            }
            continue; 
        }

        // Secciones (.data, .start, .end)
        if (c == '.') {
            advance();
            std::string sectionName = "";
            while (isalnum(peek())) sectionName += advance();
            if (sectionName == "data") tokens.push_back(makeToken(TokenType::SEC_DATA, ".data"));
            else if (sectionName == "start") tokens.push_back(makeToken(TokenType::SEC_START, ".start"));
            else if (sectionName == "end") tokens.push_back(makeToken(TokenType::SEC_END, ".end"));
            continue;
        }

        if (isalpha(c) || c == '_') { tokens.push_back(identifier()); continue; }
        if (isdigit(c)) { tokens.push_back(number()); continue; }
        if (c == '"') { tokens.push_back(stringLit()); continue; }

        advance(); 
        switch (c) {
            case ':': tokens.push_back(makeToken(TokenType::COLON, ":")); break;
            case ';': tokens.push_back(makeToken(TokenType::SEMICOLON, ";")); break;
            case '(': tokens.push_back(makeToken(TokenType::LPAREN, "(")); break;
            case ')': tokens.push_back(makeToken(TokenType::RPAREN, ")")); break;
            case '{': tokens.push_back(makeToken(TokenType::LBRACE, "{")); break;
            case '}': tokens.push_back(makeToken(TokenType::RBRACE, "}")); break;
            case '+': tokens.push_back(makeToken(TokenType::PLUS, "+")); break;
            case '-': tokens.push_back(makeToken(TokenType::MINUS, "-")); break;
            
            // Comparadores y Asignacion
            case '=': 
                if (peek() == '=') { // ==
                    advance(); 
                    tokens.push_back(makeToken(TokenType::EQ_EQ, "=="));
                } else {
                    std::cerr << "Error Lexer: '=' no es valido solo. Usa ':' para asignar." << std::endl;
                }
                break;
            case '!':
                if (peek() == '=') { // !=
                    advance();
                    tokens.push_back(makeToken(TokenType::BANG_EQ, "!="));
                }
                break;
            case '<': tokens.push_back(makeToken(TokenType::LT, "<")); break;
            case '>': tokens.push_back(makeToken(TokenType::GT, ">")); break;

            default: break;
        }
    }
    tokens.push_back(makeToken(TokenType::EOF_TOKEN, "EOF"));
    return tokens;
}

Token Lexer::identifier() {
    std::string text = "";
    while (isalnum(peek()) || peek() == '_') text += advance();

    static std::map<std::string, TokenType> keywords = {
        {"var", TokenType::KW_VAR},
        {"int", TokenType::TYPE_INT},
        {"str", TokenType::TYPE_STR},
        {"global", TokenType::KW_GLOBAL},
        {"if", TokenType::KW_IF},
        {"elif", TokenType::KW_ELIF},
        {"else", TokenType::KW_ELSE},
    };

    if (keywords.count(text)) return makeToken(keywords[text], text);
    return makeToken(TokenType::ID, text);
}

Token Lexer::number() {
    std::string text = "";
    while (isdigit(peek())) text += advance();
    return makeToken(TokenType::LIT_INT, text);
}

Token Lexer::stringLit() {
    advance();
    std::string text = "";
    while (peek() != '"' && !isAtEnd()) text += advance();
    if (!isAtEnd()) advance();
    return makeToken(TokenType::LIT_STR, text);
}