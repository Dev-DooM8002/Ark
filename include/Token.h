#pragma once
#include <string>
#include <iostream>

enum class TokenType {
    // Palabras Reservadas
    KW_VAR, KW_FUN, KW_GLOBAL, KW_LOCAL,
    KW_IF, KW_ELIF, KW_ELSE,
    KW_LOOP, KW_ULOOP, KW_BREAK, KW_JUMP,
    KW_PIL, KW_CIN,
    
    // Logicos (Keywords)
    KW_AND, KW_OR, KW_NOT,

    // Tipos
    TYPE_INT, TYPE_STR, TYPE_CHAR, TYPE_BOOL,

    // Secciones
    SEC_DATA, SEC_SBOX, SEC_START, SEC_END,

    // Literales / ID
    LIT_INT, LIT_STR, LIT_CHAR, ID,

    // Simbolos
    COLON, SEMICOLON, LPAREN, RPAREN, LBRACE, RBRACE, ARROW, COMMA,
    
    // Operadores Matematicos
    PLUS, MINUS, STAR, SLASH, // + - * /
    
    // Operadores Logicos / Comparacion / Varios
    EQ_EQ, BANG_EQ, // == !=
    LT, GT, LTE, GTE, // < > <= >=
    
    AMPERSAND, PIPE, BANG, // & | !
    
    LSHIFT, // << (Concatenacion)
    
    EOF_TOKEN, UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
};

inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::LSHIFT: return "LSHIFT (<<)";
        case TokenType::LTE: return "LTE (<=)";
        case TokenType::GTE: return "GTE (>=)";
        default: return "TOKEN";
    }
}