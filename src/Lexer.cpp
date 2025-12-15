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
        if (c == '/' && peek(1) == '/') { while (peek() != '\n' && !isAtEnd()) advance(); continue; }
        // Bloques begin/end
        if (c == '<' && src.substr(pos, 7) == "<begin>") { 
            for(int i=0;i<7;i++) advance(); 
            while(!isAtEnd()){ if(peek()=='<' && src.substr(pos,5)=="<end>"){ for(int i=0;i<5;i++) advance(); break;} advance(); } 
            continue; 
        }
        // Secciones
        if (c == '.') { 
            advance(); std::string s=""; while(isalnum(peek())) s+=advance(); 
            if(s=="data") tokens.push_back(makeToken(TokenType::SEC_DATA,".data")); 
            else if(s=="start") tokens.push_back(makeToken(TokenType::SEC_START,".start")); 
            else if(s=="end") tokens.push_back(makeToken(TokenType::SEC_END,".end")); 
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
            case ',': tokens.push_back(makeToken(TokenType::COMMA, ",")); break;
            case '{': tokens.push_back(makeToken(TokenType::LBRACE, "{")); break;
            case '}': tokens.push_back(makeToken(TokenType::RBRACE, "}")); break;
            
            // Matematicas
            case '+': tokens.push_back(makeToken(TokenType::PLUS, "+")); break;
            case '-': tokens.push_back(makeToken(TokenType::MINUS, "-")); break;
            case '*': tokens.push_back(makeToken(TokenType::STAR, "*")); break;
            case '/': tokens.push_back(makeToken(TokenType::SLASH, "/")); break;

            // Logica Single Char
            case '&': tokens.push_back(makeToken(TokenType::AMPERSAND, "&")); break;
            case '|': tokens.push_back(makeToken(TokenType::PIPE, "|")); break;
            
            // Comparacion y Concatenacion Compleja
            case '=': 
                if (peek() == '=') { advance(); tokens.push_back(makeToken(TokenType::EQ_EQ, "==")); } 
                else std::cerr << "Error Lexer: '=' invalido. Usa ':' para asignar.\n"; 
                break;
            
            case '!': 
                if (peek() == '=') { advance(); tokens.push_back(makeToken(TokenType::BANG_EQ, "!=")); } 
                else tokens.push_back(makeToken(TokenType::BANG, "!")); // NOT
                break;
            
            case '<': 
                if (peek() == '<') { advance(); tokens.push_back(makeToken(TokenType::LSHIFT, "<<")); } // CONCAT
                else if (peek() == '=') { advance(); tokens.push_back(makeToken(TokenType::LTE, "<=")); }
                else tokens.push_back(makeToken(TokenType::LT, "<")); 
                break;
            
            case '>': 
                if (peek() == '=') { advance(); tokens.push_back(makeToken(TokenType::GTE, ">=")); }
                else tokens.push_back(makeToken(TokenType::GT, ">")); 
                break;

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
        {"int", TokenType::TYPE_INT}, {"str", TokenType::TYPE_STR},
        {"global", TokenType::KW_GLOBAL}, {"if", TokenType::KW_IF},
        {"elif", TokenType::KW_ELIF}, {"else", TokenType::KW_ELSE},
        {"loop", TokenType::KW_LOOP}, {"uloop", TokenType::KW_ULOOP},
        {"break", TokenType::KW_BREAK}, {"jump", TokenType::KW_JUMP},
        {"pil", TokenType::KW_PIL}, {"cin", TokenType::KW_CIN},
        // Nuevos
        {"and", TokenType::KW_AND}, {"or", TokenType::KW_OR}, {"not", TokenType::KW_NOT}
    };

    if (keywords.count(text)) return makeToken(keywords[text], text);
    return makeToken(TokenType::ID, text);
}

Token Lexer::number() { std::string t=""; while(isdigit(peek())) t+=advance(); return makeToken(TokenType::LIT_INT, t); }
Token Lexer::stringLit() { advance(); std::string t=""; while(peek()!='"'&&!isAtEnd()) t+=advance(); if(!isAtEnd()) advance(); return makeToken(TokenType::LIT_STR, t); }