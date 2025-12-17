#include "Emitter.h"
#include <iostream>

void Generator::emit(std::string code) { output << "    " << code << "\n"; }

void Generator::push(std::string reg) { 
    emit("push " + reg); 
    tempStackDepth += 8; 
}

void Generator::pop(std::string reg) { 
    emit("pop " + reg); 
    tempStackDepth -= 8; 
}

std::string Generator::inferType(std::shared_ptr<Expression> expr) {
    if (std::dynamic_pointer_cast<StringLiteral>(expr)) return "str";
    if (std::dynamic_pointer_cast<NumberLiteral>(expr)) return "int";
    if (auto var = std::dynamic_pointer_cast<VarReference>(expr)) {
        if (localVars.count(var->name)) return localVars[var->name].type;
        if (globalVars.count(var->name)) return globalVars[var->name];
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
    else if (auto inp = std::dynamic_pointer_cast<InputStatement>(stmt)) { 
        if (!inp->prompt.empty()) { 
            std::string l="msg_"+std::to_string(stringCount++); 
            stringLiterals.push_back({l, inp->prompt}); 
        } 
    }
    else if (auto decl = std::dynamic_pointer_cast<VarDeclaration>(stmt)) {
        if (decl->initValue) {
            collectStringsFromExpr(decl->initValue);
        } else if (decl->type == "str") {
            std::string l = "empty_str_" + std::to_string(stringCount++);
            stringLiterals.push_back({l, ""});
        }
    }
    else if (auto assign = std::dynamic_pointer_cast<Assignment>(stmt)) collectStringsFromExpr(assign->value);
    else if (auto ifStmt = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        collectStringsFromExpr(ifStmt->condition); 
        for(auto s:ifStmt->thenBlock->statements) collectStrings(s);
        for(auto& e:ifStmt->elifs) { 
            collectStringsFromExpr(e.first); 
            for(auto s:e.second->statements) collectStrings(s); 
        }
        if(ifStmt->elseBlock) for(auto s:ifStmt->elseBlock->statements) collectStrings(s);
    }
    else if (auto loop = std::dynamic_pointer_cast<LoopStatement>(stmt)) { 
        if(loop->condition) collectStringsFromExpr(loop->condition); 
        for(auto s:loop->body->statements) collectStrings(s); 
    }
}

void Generator::collectStringsFromExpr(std::shared_ptr<Expression> expr) {
    if (auto str = std::dynamic_pointer_cast<StringLiteral>(expr)) { 
        std::string l="msg_"+std::to_string(stringCount++); 
        stringLiterals.push_back({l, str->value}); 
    }
    else if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) { 
        collectStringsFromExpr(bin->left); 
        collectStringsFromExpr(bin->right); 
    }
}

// --- SCANNING PARA STACK FRAME ---

void Generator::scanBlockForVars(std::shared_ptr<Block> block) {
    if (!block) return;
    for (auto stmt : block->statements) {
        scanStatementForVars(stmt);
    }
}

void Generator::scanStatementForVars(std::shared_ptr<Statement> stmt) {
    if (auto decl = std::dynamic_pointer_cast<VarDeclaration>(stmt)) {
        if (localVars.find(decl->name) == localVars.end()) {
            stackOffset += 8;
            std::string type = decl->type.empty() ? "int" : decl->type; 
            localVars[decl->name] = {stackOffset, type};
        }
    }
    else if (auto ifStmt = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        scanBlockForVars(ifStmt->thenBlock);
        for (auto& elif : ifStmt->elifs) scanBlockForVars(elif.second);
        if (ifStmt->elseBlock) scanBlockForVars(ifStmt->elseBlock);
    }
    else if (auto loop = std::dynamic_pointer_cast<LoopStatement>(stmt)) {
        scanBlockForVars(loop->body);
    }
}

std::string Generator::generate() {
    for (auto stmt : prog->startSection) collectStrings(stmt);
    stringCount = 0;
    
    output << "default rel\nsection .bss\n    input_buffer resb 64\n    current_break dq 0\n\nsection .data\n"; 

    for (auto stmt : prog->dataSection) {
        if (auto varDecl = std::dynamic_pointer_cast<VarDeclaration>(stmt)) {
            if (varDecl->initValue) {
                if (auto num = std::dynamic_pointer_cast<NumberLiteral>(varDecl->initValue)) {
                    output << varDecl->name << ": dq " << num->value << "\n"; 
                    globalVars[varDecl->name] = "int";
                } else if (auto str = std::dynamic_pointer_cast<StringLiteral>(varDecl->initValue)) {
                    std::string label = "gstr_" + varDecl->name;
                    stringLiterals.push_back({label, str->value});
                    output << varDecl->name << ": dq " << label << "\n";
                    globalVars[varDecl->name] = "str";
                } else {
                    output << varDecl->name << ": dq 0\n";
                    globalVars[varDecl->name] = "int";
                }
            } else {
                output << varDecl->name << ": dq 0\n";
                globalVars[varDecl->name] = "int";
            }
        }
    }

    for (auto& [label, value] : stringLiterals) output << label << ": db \"" << value << "\", 0\n";

    output << "\nsection .text\nglobal _start\n";

    for (auto stmt : prog->boxSection) {
        if (auto func = std::dynamic_pointer_cast<FunctionDef>(stmt)) {
            genFunctionDef(func);
        }
    }

    output << "\n_start:\n";
    emit("push rbp");
    emit("mov rbp, rsp");

    emit("mov rax, 12");        
    emit("xor rdi, rdi");       
    emit("syscall");
    emit("mov [current_break], rax"); 

    for (auto stmt : prog->startSection) genStatement(stmt);

    emit("mov rsp, rbp");
    emit("pop rbp");
    emit("mov rax, 60"); 
    emit("xor rdi, rdi"); 
    emit("syscall");

    output << "\nark_malloc:\n";
    output << "    push rbp\n    mov rbp, rsp\n";
    output << "    push rbx\n";             
    output << "    mov rbx, rdi\n";         
    output << "    mov rax, 12\n";          
    output << "    mov rdi, [current_break]\n"; 
    output << "    add rdi, rbx\n";         
    output << "    syscall\n";              
    output << "    mov rdx, [current_break]\n"; 
    output << "    mov [current_break], rax\n"; 
    output << "    mov rax, rdx\n";         
    output << "    pop rbx\n";
    output << "    leave\n    ret\n";

    output << "\nark_free:\n";
    output << "    ret\n"; 

    output << "\nstring_concat:\n";
    output << "    push rbp\n    mov rbp, rsp\n";
    output << "    push rbx\n    push r12\n    push r13\n    push r14\n"; 
    
    output << "    mov rdi, r8\n";
    output << "    xor rcx, rcx\n";
    output << ".len_a:\n    cmp byte [rdi], 0\n    je .calc_b\n    inc rdi\n    inc rcx\n    jmp .len_a\n";

    output << ".calc_b:\n    mov r12, rcx\n"; 
    output << "    mov rdi, r9\n";
    output << "    xor rcx, rcx\n";
    output << ".len_b:\n    cmp byte [rdi], 0\n    je .alloc\n    inc rdi\n    inc rcx\n    jmp .len_b\n";

    output << ".alloc:\n    mov r13, rcx\n"; 
    output << "    mov rdi, r12\n";
    output << "    add rdi, r13\n";
    output << "    inc rdi\n";      
    output << "    call ark_malloc\n"; 
    output << "    mov r14, rax\n"; 

    output << "    mov rsi, r8\n";  
    output << "    mov rdi, r14\n"; 
    output << ".copy_a_loop:\n    cmp byte [rsi], 0\n    je .copy_b_setup\n    mov al, [rsi]\n    mov [rdi], al\n    inc rsi\n    inc rdi\n    jmp .copy_a_loop\n";

    output << ".copy_b_setup:\n    mov rsi, r9\n"; 
    output << ".copy_b_loop:\n    cmp byte [rsi], 0\n    je .finish\n    mov al, [rsi]\n    mov [rdi], al\n    inc rsi\n    inc rdi\n    jmp .copy_b_loop\n";

    output << ".finish:\n    mov byte [rdi], 0\n";
    output << "    mov rax, r14\n";
    output << "    pop r14\n    pop r13\n    pop r12\n    pop rbx\n";
    output << "    leave\n    ret\n";

    output << "\nprint_string_inline:\n    push rbp\n    mov rbp, rsp\n    push rbx\n    mov rbx, rax\n.loop_len:\n    cmp byte [rax], 0\n    je .print\n    inc rax\n    jmp .loop_len\n.print:\n    sub rax, rbx\n    mov rdx, rax\n    mov rsi, rbx\n    mov rax, 1\n    mov rdi, 1\n    syscall\n    pop rbx\n    leave\n    ret\n";
    output << "\nprint_string:\n    call print_string_inline\n    push 10\n    mov rax, 1\n    mov rdi, 1\n    mov rsi, rsp\n    mov rdx, 1\n    syscall\n    pop rax\n    ret\n";
    output << "\nprint_int:\n    mov rcx, 10\n    mov rbx, 0\n    push 0\n.next_digit:\n    xor rdx, rdx\n    div rcx\n    add rdx, '0'\n    push rdx\n    inc rbx\n    test rax, rax\n    jnz .next_digit\n.print_loop:\n    mov rax, 1\n    mov rdi, 1\n    mov rsi, rsp\n    mov rdx, 1\n    syscall\n    pop rdx\n    dec rbx\n    jnz .print_loop\n    push 10\n    mov rax, 1\n    mov rdi, 1\n    mov rsi, rsp\n    mov rdx, 1\n    syscall\n    pop rax\n    pop rax\n    ret\n";
    output << "\nread_input:\n    mov rax, 0\n    mov rdi, 0\n    mov rsi, input_buffer\n    mov rdx, 64\n    syscall\n    mov rbx, input_buffer\n    add rbx, rax\n    dec rbx\n    mov byte [rbx], 0\n    mov rax, input_buffer\n    ret\n";

    return output.str();
}

void Generator::genStatement(std::shared_ptr<Statement> stmt) {
    if (auto decl = std::dynamic_pointer_cast<VarDeclaration>(stmt)) {
        std::string type = decl->type; 

        if (decl->initValue) {
            if (type.empty()) type = inferType(decl->initValue);
            genExpression(decl->initValue);
        } else {
            if (type == "str") {
                std::string emptyLabel = "empty_str_" + std::to_string(stringCount++);
                emit("mov rax, " + emptyLabel);
            } else {
                if (type.empty()) type = "int";
                emit("xor rax, rax");
            }
        }
        
        // CORRECCION: Usar espacio reservado (No Push)
        if (localVars.count(decl->name)) {
            // Actualizar tipo si fue inferido ahora
            localVars[decl->name].type = type; 
            emit("mov [rbp - " + std::to_string(localVars[decl->name].offset) + "], rax");
        } else {
            // Fallback para globales o errores
            emit("mov [" + decl->name + "], rax");
        }
    } 
    else if (auto assign = std::dynamic_pointer_cast<Assignment>(stmt)) {
        genExpression(assign->value); 
        if (localVars.count(assign->name)) {
            emit("mov [rbp - " + std::to_string(localVars[assign->name].offset) + "], rax");
        } else {
            emit("mov [" + assign->name + "], rax");
        }
    }
    else if (auto print = std::dynamic_pointer_cast<PrintStatement>(stmt)) {
        genExpression(print->expression); 
        std::string t = inferType(print->expression);
        if (t == "str") emit("call print_string"); 
        else emit("call print_int"); 
    }
    else if (auto pil = std::dynamic_pointer_cast<PilStatement>(stmt)) {
        genExpression(pil->expression); 
        std::string t = inferType(pil->expression);
        if (t == "str") emit("call print_string_inline"); 
        else emit("call print_int"); 
    }
    else if (auto inp = std::dynamic_pointer_cast<InputStatement>(stmt)) {
        if (!inp->prompt.empty()) { 
            std::string l="msg_"+std::to_string(stringCount++); 
            emit("mov rax, "+l); 
            emit("call print_string_inline"); 
        }
        emit("call read_input");
        if (localVars.count(inp->varName)) {
            emit("mov [rbp - " + std::to_string(localVars[inp->varName].offset) + "], rax");
        } else {
            emit("mov [" + inp->varName + "], rax");
        }
    } 
    else if (auto loop = std::dynamic_pointer_cast<LoopStatement>(stmt)) {
        int id = labelCounter++; 
        std::string start=".L_start_"+std::to_string(id), end=".L_end_"+std::to_string(id);
        loopStack.push_back({start, end}); 
        output << start << ":\n";
        if(loop->type != LoopType::INFINITE) { 
            genExpression(loop->condition); 
            emit("cmp rax, 1"); 
            if(loop->type==LoopType::WHILE) emit("jne "+end); 
            else emit("je "+end); 
        }
        for(auto s:loop->body->statements) genStatement(s); 
        emit("jmp "+start); 
        output << end << ":\n"; 
        loopStack.pop_back();
    } 
    else if (std::dynamic_pointer_cast<BreakStatement>(stmt)) {
        emit("jmp "+loopStack.back().second);
    }
    else if (std::dynamic_pointer_cast<JumpStatement>(stmt)) {
        emit("jmp "+loopStack.back().first);
    }
    else if (auto ifs = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        int id = labelCounter++; 
        std::string endL=".L_ifend_"+std::to_string(id), elseL=".L_else_"+std::to_string(id);
        genExpression(ifs->condition); 
        emit("cmp rax, 1"); 
        emit("jne "+(ifs->elifs.empty() ? elseL : ".L_elif_0_"+std::to_string(id)));
        for(auto s:ifs->thenBlock->statements) genStatement(s); 
        emit("jmp "+endL);
        for(size_t i=0; i<ifs->elifs.size(); ++i) {
            output << ".L_elif_" << i << "_" << id << ":\n"; 
            genExpression(ifs->elifs[i].first); 
            emit("cmp rax, 1");
            emit("jne "+(i+1 < ifs->elifs.size() ? ".L_elif_"+std::to_string(i+1)+"_"+std::to_string(id) : elseL));
            for(auto s:ifs->elifs[i].second->statements) genStatement(s); 
            emit("jmp "+endL);
        }
        output << elseL << ":\n"; 
        if(ifs->elseBlock) for(auto s:ifs->elseBlock->statements) genStatement(s); 
        output << endL << ":\n";
    } 
    else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(stmt)) {
        genExpression(exprStmt->expression);
    }
}

void Generator::genExpression(std::shared_ptr<Expression> expr) {
    if (auto num = std::dynamic_pointer_cast<NumberLiteral>(expr)) {
        emit("mov rax, " + std::to_string(num->value));
    }
    else if (auto var = std::dynamic_pointer_cast<VarReference>(expr)) { 
        if (localVars.count(var->name)) {
            emit("mov rax, [rbp - " + std::to_string(localVars[var->name].offset) + "]");
        } else {
            emit("mov rax, [" + var->name + "]");
        }
    }
    else if (auto str = std::dynamic_pointer_cast<StringLiteral>(expr)) { 
        std::string l="msg_"+std::to_string(stringCount++); 
        emit("mov rax, "+l); 
    }
    else if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        genExpression(un->right);
        if (un->op == TokenType::KW_NOT || un->op == TokenType::BANG) {
            emit("xor rax, 1");
        }
    } 
    else if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        if (bin->op == TokenType::LSHIFT) {
            genExpression(bin->left); 
            push("rax");
            genExpression(bin->right); 
            pop("rbx");
            emit("mov r8, rbx"); 
            emit("mov r9, rax"); 
            emit("call string_concat");
            return;
        }
        
        genExpression(bin->right); 
        push("rax"); 
        genExpression(bin->left); 
        pop("rbx");
        
        if (bin->op == TokenType::PLUS) emit("add rax, rbx");
        else if (bin->op == TokenType::MINUS) emit("sub rax, rbx");
        else if (bin->op == TokenType::STAR) emit("imul rax, rbx"); 
        else if (bin->op == TokenType::SLASH) { 
            emit("cqo"); 
            emit("idiv rbx"); 
        } 
        else if (bin->op == TokenType::KW_AND || bin->op == TokenType::AMPERSAND) {
            emit("and rax, rbx");
        }
        else if (bin->op == TokenType::KW_OR || bin->op == TokenType::PIPE) {
            emit("or rax, rbx");
        }
        else {
            emit("cmp rax, rbx"); 
            std::string setCode;
            switch(bin->op) {
                case TokenType::EQ_EQ: setCode="sete"; break; 
                case TokenType::BANG_EQ: setCode="setne"; break;
                case TokenType::LT: setCode="setl"; break; 
                case TokenType::GT: setCode="setg"; break;
                case TokenType::LTE: setCode="setle"; break; 
                case TokenType::GTE: setCode="setge"; break;
                default: setCode="sete";
            } 
            emit(setCode+" al"); 
            emit("movzx rax, al");
        }
    } 
    else if (auto call = std::dynamic_pointer_cast<FunctionCall>(expr)) {
        genFunctionCall(call);
    }
}

void Generator::genFunctionDef(std::shared_ptr<FunctionDef> func) {
    currentFuncArgs.clear();
    localVars.clear();
    stackOffset = 0;
    tempStackDepth = 0;

    output << "\n_fn_" << func->name << ":\n";
    emit("push rbp");
    emit("mov rbp, rsp");

    std::vector<std::string> argRegs = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

    // 1. Calcular offsets para argumentos
    for (size_t i = 0; i < func->params.size(); i++) {
        stackOffset += 8;
        localVars[func->params[i]] = {stackOffset, "int"};
    }

    // 2. Calcular variables locales (pre-scan)
    scanBlockForVars(func->body);

    // 3. Alinear stack a 16 bytes
    if (stackOffset % 16 != 0) stackOffset += 8;

    // 4. Reservar espacio de una sola vez
    if (stackOffset > 0) {
        emit("sub rsp, " + std::to_string(stackOffset));
    }

    // 5. Mover argumentos a sus slots de memoria
    for (size_t i = 0; i < func->params.size(); i++) {
        std::string offset = std::to_string(localVars[func->params[i]].offset);
        if (i < 6) {
            emit("mov [rbp - " + offset + "], " + argRegs[i]);
        } else {
            int callerOffset = 16 + (i - 6) * 8;
            emit("mov rax, [rbp + " + std::to_string(callerOffset) + "]");
            emit("mov [rbp - " + offset + "], rax");
        }
    }

    for (auto stmt : func->body->statements) {
        genStatement(stmt);
    }

    if (func->returnValue) {
        genExpression(func->returnValue);
    } else {
        emit("xor rax, rax");
    }

    emit("leave");
    emit("ret");
}

void Generator::genFunctionCall(std::shared_ptr<FunctionCall> call) {
    std::vector<std::string> argRegs = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

    for (auto& arg : call->arguments) {
        genExpression(arg);
        push("rax");
    }

    size_t regArgs = std::min(call->arguments.size(), (size_t)6);
    for (int i = regArgs - 1; i >= 0; i--) {
        pop(argRegs[i]);
    }

    emit("call _fn_" + call->callee);

    if (call->arguments.size() > 6) {
        int extraArgs = call->arguments.size() - 6;
        emit("add rsp, " + std::to_string(extraArgs * 8));
        tempStackDepth -= extraArgs * 8;
    }
}