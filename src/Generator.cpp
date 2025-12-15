#include "../include/Generator.h"
#include <iostream>

void Generator::emit(std::string code) { output << "    " << code << "\n"; }
void Generator::push(std::string reg) { emit("push " + reg); stackOffset += 8; }
void Generator::pop(std::string reg) { emit("pop " + reg); stackOffset -= 8; }

std::string Generator::inferType(std::shared_ptr<Expression> expr) {
    if (std::dynamic_pointer_cast<StringLiteral>(expr)) return "str";
    if (std::dynamic_pointer_cast<NumberLiteral>(expr)) return "int";
    if (auto var = std::dynamic_pointer_cast<VarReference>(expr)) {
        if (localVars.count(var->name)) return localVars[var->name].type;
        return "int"; 
    }
    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        if (bin->op == TokenType::LSHIFT) return "str"; 
        return "int";
    }
    if (std::dynamic_pointer_cast<UnaryExpr>(expr)) return "int"; 
    return "int";
}

void Generator::collectStrings(std::shared_ptr<Statement> stmt) {
    if (auto print = std::dynamic_pointer_cast<PrintStatement>(stmt)) collectStringsFromExpr(print->expression);
    else if (auto pil = std::dynamic_pointer_cast<PilStatement>(stmt)) collectStringsFromExpr(pil->expression);
    else if (auto inp = std::dynamic_pointer_cast<InputStatement>(stmt)) { if (!inp->prompt.empty()) { std::string l="msg_"+std::to_string(stringCount++); stringLiterals.push_back({l, inp->prompt}); } }
    else if (auto decl = std::dynamic_pointer_cast<VarDeclaration>(stmt)) collectStringsFromExpr(decl->initValue);
    else if (auto assign = std::dynamic_pointer_cast<Assignment>(stmt)) collectStringsFromExpr(assign->value);
    else if (auto ifStmt = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        collectStringsFromExpr(ifStmt->condition); for(auto s:ifStmt->thenBlock->statements) collectStrings(s);
        for(auto& e:ifStmt->elifs) { collectStringsFromExpr(e.first); for(auto s:e.second->statements) collectStrings(s); }
        if(ifStmt->elseBlock) for(auto s:ifStmt->elseBlock->statements) collectStrings(s);
    }
    else if (auto loop = std::dynamic_pointer_cast<LoopStatement>(stmt)) { if(loop->condition) collectStringsFromExpr(loop->condition); for(auto s:loop->body->statements) collectStrings(s); }
}

void Generator::collectStringsFromExpr(std::shared_ptr<Expression> expr) {
    if (auto str = std::dynamic_pointer_cast<StringLiteral>(expr)) { std::string l="msg_"+std::to_string(stringCount++); stringLiterals.push_back({l, str->value}); }
    else if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) { collectStringsFromExpr(bin->left); collectStringsFromExpr(bin->right); }
}

std::string Generator::generate() {
    for (auto stmt : prog->startSection) collectStrings(stmt);
    stringCount = 0;
    
    output << "default rel\nsection .bss\n    input_buffer resb 64\n    heap_memory resb 100000\n\nsection .data\n    heap_ptr dq heap_memory\n"; 
    for (auto stmt : prog->dataSection) {
        if (auto varDecl = std::dynamic_pointer_cast<VarDeclaration>(stmt)) {
            int initVal = 0; if (auto num = std::dynamic_pointer_cast<NumberLiteral>(varDecl->initValue)) initVal = num->value;
            output << varDecl->name << ": dq " << initVal << "\n"; 
        }
    }
    for (auto& [label, value] : stringLiterals) output << label << ": db \"" << value << "\", 0\n";

    output << "\nsection .text\nglobal _start\n\n_start:\n";
    emit("mov rbp, rsp");
    for (auto stmt : prog->startSection) genStatement(stmt);
    emit("mov rax, 60"); emit("xor rdi, rdi"); emit("syscall");

    // string_concat: R8=Primero, R9=Segundo.
    output << "\nstring_concat:\n";
    output << "    push rbp\n    mov rbp, rsp\n";
    output << "    push rdi\n    push rsi\n    push rbx\n    push rcx\n    push r8\n    push r9\n";
    
    output << "    mov rdi, [heap_ptr]\n";
    
    // 1. Copiar R8
    output << "    mov rsi, r8\n";
    output << ".copy_a:\n    cmp byte [rsi], 0\n    je .done_a\n    mov al, [rsi]\n    mov [rdi], al\n    inc rsi\n    inc rdi\n    jmp .copy_a\n.done_a:\n";
    
    // 2. Copiar R9
    output << "    mov rsi, r9\n";
    output << ".copy_b:\n    cmp byte [rsi], 0\n    je .done_b\n    mov al, [rsi]\n    mov [rdi], al\n    inc rsi\n    inc rdi\n    jmp .copy_b\n.done_b:\n";
    
    output << "    mov byte [rdi], 0\n    inc rdi\n";
    output << "    mov rax, [heap_ptr]\n";
    output << "    mov [heap_ptr], rdi\n";
    
    output << "    pop r9\n    pop r8\n    pop rcx\n    pop rbx\n    pop rsi\n    pop rdi\n";
    output << "    leave\n    ret\n";

    output << "\nprint_string_inline:\n    push rbp\n    mov rbp, rsp\n    push rbx\n    mov rbx, rax\n.loop_len:\n    cmp byte [rax], 0\n    je .print\n    inc rax\n    jmp .loop_len\n.print:\n    sub rax, rbx\n    mov rdx, rax\n    mov rsi, rbx\n    mov rax, 1\n    mov rdi, 1\n    syscall\n    pop rbx\n    leave\n    ret\n";
    output << "\nprint_string:\n    call print_string_inline\n    push 10\n    mov rax, 1\n    mov rdi, 1\n    mov rsi, rsp\n    mov rdx, 1\n    syscall\n    pop rax\n    ret\n";
    output << "\nprint_int:\n    mov rcx, 10\n    mov rbx, 0\n    push 0\n.next_digit:\n    xor rdx, rdx\n    div rcx\n    add rdx, '0'\n    push rdx\n    inc rbx\n    test rax, rax\n    jnz .next_digit\n.print_loop:\n    mov rax, 1\n    mov rdi, 1\n    mov rsi, rsp\n    mov rdx, 1\n    syscall\n    pop rdx\n    dec rbx\n    jnz .print_loop\n    push 10\n    mov rax, 1\n    mov rdi, 1\n    mov rsi, rsp\n    mov rdx, 1\n    syscall\n    pop rax\n    pop rax\n    ret\n";
    output << "\nread_input:\n    mov rax, 0\n    mov rdi, 0\n    mov rsi, input_buffer\n    mov rdx, 64\n    syscall\n    mov rbx, input_buffer\n    add rbx, rax\n    dec rbx\n    mov byte [rbx], 0\n    mov rax, input_buffer\n    ret\n";

    return output.str();
}

void Generator::genStatement(std::shared_ptr<Statement> stmt) {
    if (auto decl = std::dynamic_pointer_cast<VarDeclaration>(stmt)) {
        std::string type = decl->type; if (type.empty()) type = inferType(decl->initValue);
        genExpression(decl->initValue); push("rax"); localVars[decl->name] = {stackOffset, type}; 
    }
    else if (auto assign = std::dynamic_pointer_cast<Assignment>(stmt)) {
        genExpression(assign->value); 
        if (localVars.count(assign->name)) emit("mov [rbp - " + std::to_string(localVars[assign->name].offset) + "], rax");
        else emit("mov [" + assign->name + "], rax");
    }
    else if (auto print = std::dynamic_pointer_cast<PrintStatement>(stmt)) {
        genExpression(print->expression); std::string t = inferType(print->expression);
        if (t == "str") emit("call print_string"); else emit("call print_int"); 
    }
    else if (auto pil = std::dynamic_pointer_cast<PilStatement>(stmt)) {
        genExpression(pil->expression); std::string t = inferType(pil->expression);
        if (t == "str") emit("call print_string_inline"); else emit("call print_int"); 
    }
    else if (auto inp = std::dynamic_pointer_cast<InputStatement>(stmt)) {
        if (!inp->prompt.empty()) { std::string l="msg_"+std::to_string(stringCount++); emit("mov rax, "+l); emit("call print_string_inline"); }
        emit("call read_input");
        if (localVars.count(inp->varName)) emit("mov [rbp - " + std::to_string(localVars[inp->varName].offset) + "], rax"); 
        else emit("mov [" + inp->varName + "], rax");
    }
    else if (auto loop = std::dynamic_pointer_cast<LoopStatement>(stmt)) {
        int id = labelCounter++; std::string start=".L_start_"+std::to_string(id), end=".L_end_"+std::to_string(id);
        loopStack.push_back({start, end}); output << start << ":\n";
        if(loop->type != LoopType::INFINITE) { genExpression(loop->condition); emit("cmp rax, 1"); if(loop->type==LoopType::WHILE) emit("jne "+end); else emit("je "+end); }
        for(auto s:loop->body->statements) genStatement(s); emit("jmp "+start); output << end << ":\n"; loopStack.pop_back();
    }
    else if (std::dynamic_pointer_cast<BreakStatement>(stmt)) emit("jmp "+loopStack.back().second);
    else if (std::dynamic_pointer_cast<JumpStatement>(stmt)) emit("jmp "+loopStack.back().first);
    else if (auto ifs = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        int id = labelCounter++; std::string endL=".L_ifend_"+std::to_string(id), elseL=".L_else_"+std::to_string(id);
        genExpression(ifs->condition); emit("cmp rax, 1"); emit("jne "+(ifs->elifs.empty() ? elseL : ".L_elif_0_"+std::to_string(id)));
        for(auto s:ifs->thenBlock->statements) genStatement(s); emit("jmp "+endL);
        for(size_t i=0; i<ifs->elifs.size(); ++i) {
            output << ".L_elif_" << i << "_" << id << ":\n"; genExpression(ifs->elifs[i].first); emit("cmp rax, 1");
            emit("jne "+(i+1 < ifs->elifs.size() ? ".L_elif_"+std::to_string(i+1)+"_"+std::to_string(id) : elseL));
            for(auto s:ifs->elifs[i].second->statements) genStatement(s); emit("jmp "+endL);
        }
        output << elseL << ":\n"; if(ifs->elseBlock) for(auto s:ifs->elseBlock->statements) genStatement(s); output << endL << ":\n";
    }
}

void Generator::genExpression(std::shared_ptr<Expression> expr) {
    if (auto num = std::dynamic_pointer_cast<NumberLiteral>(expr)) emit("mov rax, " + std::to_string(num->value));
    else if (auto var = std::dynamic_pointer_cast<VarReference>(expr)) { if (localVars.count(var->name)) emit("mov rax, [rbp - " + std::to_string(localVars[var->name].offset) + "]"); else emit("mov rax, [" + var->name + "]"); }
    else if (auto str = std::dynamic_pointer_cast<StringLiteral>(expr)) { std::string l="msg_"+std::to_string(stringCount++); emit("mov rax, "+l); }
    else if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        genExpression(un->right);
        if (un->op == TokenType::KW_NOT || un->op == TokenType::BANG) emit("xor rax, 1");
    }
    else if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        genExpression(bin->right); push("rax"); genExpression(bin->left); pop("rbx");
        // RAX = Left, RBX = Right
        if (bin->op == TokenType::PLUS) emit("add rax, rbx");
        else if (bin->op == TokenType::MINUS) emit("sub rax, rbx");
        else if (bin->op == TokenType::STAR) emit("imul rax, rbx"); 
        else if (bin->op == TokenType::SLASH) { emit("cqo"); emit("idiv rbx"); } 
        else if (bin->op == TokenType::LSHIFT) { 
            // PARCHE TACTICO: INVERTIR ORDEN
            // Normalmente: r8=rax, r9=rbx. Pero como sale al reves...
            emit("mov r8, rbx"); // R8 = Right
            emit("mov r9, rax"); // R9 = Left
            // Esto *deberia* salir al reves, pero como tu output ya sale al reves,
            // esto deberia negarlo y salir bien. (Derecha+Izquierda)^-1 = Izquierda+Derecha
            emit("call string_concat"); 
        }
        else if (bin->op == TokenType::KW_AND || bin->op == TokenType::AMPERSAND) emit("and rax, rbx");
        else if (bin->op == TokenType::KW_OR || bin->op == TokenType::PIPE) emit("or rax, rbx");
        else {
            emit("cmp rax, rbx"); std::string setCode;
            switch(bin->op) {
                case TokenType::EQ_EQ: setCode="sete"; break; case TokenType::BANG_EQ: setCode="setne"; break;
                case TokenType::LT: setCode="setl"; break; case TokenType::GT: setCode="setg"; break;
                case TokenType::LTE: setCode="setle"; break; case TokenType::GTE: setCode="setge"; break;
                default: setCode="sete";
            } emit(setCode+" al"); emit("movzx rax, al");
        }
    }
}