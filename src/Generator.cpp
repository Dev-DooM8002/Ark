#include "../include/Generator.h"
#include <iostream>

void Generator::emit(std::string code) {
    output << "    " << code << "\n";
}

void Generator::push(std::string reg) {
    emit("push " + reg);
    stackOffset += 8; 
}

void Generator::pop(std::string reg) {
    emit("pop " + reg);
    stackOffset -= 8;
}

std::string Generator::generate() {
    output << "default rel\nsection .data\n";
    for (auto stmt : prog->dataSection) {
        if (auto varDecl = std::dynamic_pointer_cast<VarDeclaration>(stmt)) {
            int initVal = 0;
            if (auto num = std::dynamic_pointer_cast<NumberLiteral>(varDecl->initValue)) initVal = num->value;
            output << varDecl->name << ": dq " << initVal << "\n"; 
        }
    }

    output << "\nsection .text\nglobal _start\n\n_start:\n";
    emit("mov rbp, rsp");

    for (auto stmt : prog->startSection) genStatement(stmt);

    emit("mov rax, 60");
    emit("xor rdi, rdi"); 
    emit("syscall");

    // Rutinas Helper ASM
    output << "\nprint_string:\n    push rbp\n    mov rbp, rsp\n    push rbx\n    mov rbx, rax\n.loop_len:\n    cmp byte [rax], 0\n    je .print\n    inc rax\n    jmp .loop_len\n.print:\n    sub rax, rbx\n    mov rdx, rax\n    mov rsi, rbx\n    mov rax, 1\n    mov rdi, 1\n    syscall\n    push 10\n    mov rax, 1\n    mov rdi, 1\n    mov rsi, rsp\n    mov rdx, 1\n    syscall\n    pop rax\n    pop rbx\n    leave\n    ret\n";
    output << "\nprint_int:\n    mov rcx, 10\n    mov rbx, 0\n    push 0\n.next_digit:\n    xor rdx, rdx\n    div rcx\n    add rdx, '0'\n    push rdx\n    inc rbx\n    test rax, rax\n    jnz .next_digit\n.print_loop:\n    mov rax, 1\n    mov rdi, 1\n    mov rsi, rsp\n    mov rdx, 1\n    syscall\n    pop rdx\n    dec rbx\n    jnz .print_loop\n    push 10\n    mov rax, 1\n    mov rdi, 1\n    mov rsi, rsp\n    mov rdx, 1\n    syscall\n    pop rax\n    pop rax\n    ret\n";

    return output.str();
}

int labelCounter = 0;

void Generator::genStatement(std::shared_ptr<Statement> stmt) {
    if (auto decl = std::dynamic_pointer_cast<VarDeclaration>(stmt)) {
        genExpression(decl->initValue); 
        push("rax");
        localVars[decl->name] = stackOffset; 
    }
    else if (auto print = std::dynamic_pointer_cast<PrintStatement>(stmt)) {
        genExpression(print->expression); 
        if (std::dynamic_pointer_cast<StringLiteral>(print->expression)) emit("call print_string");
        else emit("call print_int"); 
    }
    else if (auto assign = std::dynamic_pointer_cast<Assignment>(stmt)) {
        genExpression(assign->value); 
        if (localVars.count(assign->name)) emit("mov [rbp - " + std::to_string(localVars[assign->name]) + "], rax");
        else emit("mov [" + assign->name + "], rax");
    }
    
    // --- IF / ELIF / ELSE ---
    else if (auto ifStmt = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        int id = labelCounter++;
        std::string endLabel = ".L_end_" + std::to_string(id);
        std::string elseLabel = ".L_else_" + std::to_string(id);
        
        // 1. Condicion Principal
        genExpression(ifStmt->condition);
        emit("cmp rax, 1");
        std::string nextLabel = (ifStmt->elifs.empty()) ? elseLabel : ".L_elif_0_" + std::to_string(id);
        emit("jne " + nextLabel);
        
        for(auto s : ifStmt->thenBlock->statements) genStatement(s);
        emit("jmp " + endLabel);

        // 2. Elifs
        for (size_t i = 0; i < ifStmt->elifs.size(); ++i) {
            std::string currentElifLabel = ".L_elif_" + std::to_string(i) + "_" + std::to_string(id);
            std::string nextElifLabel = (i + 1 < ifStmt->elifs.size()) ? ".L_elif_" + std::to_string(i+1) + "_" + std::to_string(id) : elseLabel;

            output << currentElifLabel << ":\n";
            genExpression(ifStmt->elifs[i].first);
            emit("cmp rax, 1");
            emit("jne " + nextElifLabel);

            for(auto s : ifStmt->elifs[i].second->statements) genStatement(s);
            emit("jmp " + endLabel);
        }

        // 3. Else
        output << elseLabel << ":\n";
        if (ifStmt->elseBlock) {
            for(auto s : ifStmt->elseBlock->statements) genStatement(s);
        }

        output << endLabel << ":\n";
    }
}

void Generator::genExpression(std::shared_ptr<Expression> expr) {
    if (auto num = std::dynamic_pointer_cast<NumberLiteral>(expr)) {
        emit("mov rax, " + std::to_string(num->value));
    }
    else if (auto var = std::dynamic_pointer_cast<VarReference>(expr)) {
        if (localVars.count(var->name)) emit("mov rax, [rbp - " + std::to_string(localVars[var->name]) + "]");
        else emit("mov rax, [" + var->name + "]");
    }
    else if (auto str = std::dynamic_pointer_cast<StringLiteral>(expr)) {
        std::string label = ".str" + std::to_string(stringCount++);
        emit("section .data");
        emit(label + ": db '" + str->value + "', 0");
        emit("section .text");
        emit("mov rax, " + label);
    }
    else if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        genExpression(bin->right);
        push("rax"); 
        genExpression(bin->left);
        pop("rbx"); 
        // AHORA: rax = left, rbx = right

        if (bin->op == TokenType::PLUS) emit("add rax, rbx");
        else if (bin->op == TokenType::MINUS) {
            emit("sub rax, rbx"); // FIX: Left - Right (antes estaba al reves)
        }
        else {
            emit("cmp rax, rbx"); // FIX: Left vs Right (antes estaba rbx, rax)
            std::string setCode;
            switch(bin->op) {
                case TokenType::EQ_EQ: setCode = "sete"; break;
                case TokenType::BANG_EQ: setCode = "setne"; break;
                case TokenType::LT: setCode = "setl"; break;
                case TokenType::GT: setCode = "setg"; break;
                default: setCode = "sete"; 
            }
            emit(setCode + " al");
            emit("movzx rax, al");
        }
    }
}