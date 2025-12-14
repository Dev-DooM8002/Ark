#pragma once
#include <string>
#include <iostream>

enum class TokenType {
    // 1. Palabras Reservadas
    KW_VAR, KW_FUN, KW_GLOBAL, KW_LOCAL,
    KW_IF,          // if
    KW_ELIF,        // elif
    KW_ELSE,        // else
    
    // 2. Tipos de Datos
    TYPE_INT, TYPE_STR, TYPE_CHAR, TYPE_BOOL,

    // 3. Secciones
    SEC_DATA, SEC_SBOX, SEC_START, SEC_END,

    // 4. Literales e Identificadores
    LIT_INT, LIT_STR, LIT_CHAR, ID,

    // 5. Simbolos
    COLON,      // :
    SEMICOLON,  // ;
    LPAREN, RPAREN, // ( )
    LBRACE, RBRACE, // { }
    ARROW,      // ->
    
    // 6. Operadores
    PLUS, MINUS,    // + -
    EQ_EQ,          // ==
    BANG_EQ,        // !=
    LT, GT,         // < >
    
    // 7. Control
    EOF_TOKEN, UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
};

// Helper para debug (Ahora si completo)
inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        // Palabras Clave
        case TokenType::KW_VAR: return "KW_VAR";
        case TokenType::KW_FUN: return "KW_FUN";
        case TokenType::KW_GLOBAL: return "KW_GLOBAL";
        case TokenType::KW_LOCAL: return "KW_LOCAL";
        case TokenType::KW_IF: return "KW_IF";
        case TokenType::KW_ELIF: return "KW_ELIF";
        case TokenType::KW_ELSE: return "KW_ELSE";

        // Tipos
        case TokenType::TYPE_INT: return "TYPE_INT";
        case TokenType::TYPE_STR: return "TYPE_STR";
        case TokenType::TYPE_CHAR: return "TYPE_CHAR";
        case TokenType::TYPE_BOOL: return "TYPE_BOOL";

        // Secciones
        case TokenType::SEC_DATA: return "SEC_DATA";
        case TokenType::SEC_SBOX: return "SEC_SBOX";
        case TokenType::SEC_START: return "SEC_START";
        case TokenType::SEC_END: return "SEC_END";

        // Literales
        case TokenType::LIT_INT: return "LIT_INT";
        case TokenType::LIT_STR: return "LIT_STR";
        case TokenType::LIT_CHAR: return "LIT_CHAR";
        case TokenType::ID: return "ID";

        // Simbolos
        case TokenType::COLON: return "COLON";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::ARROW: return "ARROW";

        // Operadores
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::EQ_EQ: return "EQ_EQ";
        case TokenType::BANG_EQ: return "BANG_EQ";
        case TokenType::LT: return "LT";
        case TokenType::GT: return "GT";

        // Control
        case TokenType::EOF_TOKEN: return "EOF";
        default: return "UNKNOWN";
    }
}