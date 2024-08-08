#include "../include/parser.h"

/**
* Function call
* Variable declaration
* Numbers
* Binary operations
*
*/

// @CHECK: Don't Consume ';' at end of expression
// @TODO: Add a ret instruction for void functions, to avoid seg fault on pass run
// @TODO: Implement Better Error management

// Code Generator
cCodeGenerator::cCodeGenerator() {
    this->m_Context = std::make_unique<llvm::LLVMContext>();
    this->m_Module = std::make_unique<llvm::Module>("DepLangModule", *m_Context);

    this->m_Builder = std::make_unique<llvm::IRBuilder<>>(*m_Context);
}

llvm::Type* get_llvm_type(const ePrimitiveType& type, llvm::LLVMContext& context) {
    switch (type) {
    case TYPE_INT:    return llvm::Type::getInt32Ty(context);
    case TYPE_BOOL:   return llvm::Type::getInt1Ty(context);
    case TYPE_FLOAT:  return llvm::Type::getFloatTy(context);
    case TYPE_EMPTY:  return llvm::Type::getVoidTy(context);
    }

    return nullptr;
}

std::unique_ptr<sTypedValue> build_ir_operation(std::unique_ptr<sTypedValue> l, std::unique_ptr<sTypedValue> r, std::string op, std::shared_ptr<cCodeGenerator> code_generator) {

    if (!l || !r || !l->value || !r->value) { return nullptr; }
    // @TODO: Change for type coersion
    if (l->type->get_primitive_type() != r->type->get_primitive_type()) { return nullptr; }

    llvm::Value* final_value = nullptr;
    if (l->type->get_primitive_type() == TYPE_FLOAT) {
        if (op == "+") { final_value = code_generator->m_Builder->CreateFAdd(l->value, r->value, "addtmp"); } 
        else if (op == "-") { final_value = code_generator->m_Builder->CreateFSub(l->value, r->value, "subtmp"); }
        else if (op == "*") { final_value = code_generator->m_Builder->CreateFMul(l->value, r->value, "multmp"); }
        else if (op == "<") {
            l->value = code_generator->m_Builder->CreateFCmpULT(l->value, r->value, "cmptmp");
            final_value = code_generator->m_Builder->CreateUIToFP(l->value, llvm::Type::getInt1Ty(*code_generator->m_Context), "booltmp");
        }
        else if (op == ">") {
            l->value = code_generator->m_Builder->CreateFCmpULT(r->value, l->value, "cmptmp");
            final_value = code_generator->m_Builder->CreateUIToFP(l->value, llvm::Type::getInt1Ty(*code_generator->m_Context), "booltmp");
        }
        else {
            DEPLANG_PARSER_ERROR("Expected Operator, got " << op);
            return nullptr;
        }
    } 
    else if (l->type->get_primitive_type() == TYPE_INT) {
        if (op == "+") { code_generator->m_Builder->CreateAdd(l->value, r->value, "addtmp"); } 
        else if (op == "-") { final_value = code_generator->m_Builder->CreateSub(l->value, r->value, "subtmp"); }
        else if (op == "*") { final_value = code_generator->m_Builder->CreateMul(l->value, r->value, "multmp"); }
        else if (op == "<") {
            l->value = code_generator->m_Builder->CreateICmpULT(l->value, r->value, "cmptmp");
            final_value = code_generator->m_Builder->CreateUIToFP(l->value, llvm::Type::getInt1Ty(*code_generator->m_Context), "booltmp");
        }
        else if (op == ">") {
            l->value = code_generator->m_Builder->CreateICmpULT(r->value, l->value, "cmptmp");
            final_value = code_generator->m_Builder->CreateUIToFP(l->value, llvm::Type::getInt1Ty(*code_generator->m_Context), "booltmp");
        }
        else {
            DEPLANG_PARSER_ERROR("Expected Operator, got " << op);
            return nullptr;
        }

    }
    else {
        // @TODO: Implement binary operations for bools
        DEPLANG_PARSER_ERROR("Bool does not support binary operations");
        return nullptr;
    }

    return std::make_unique<sTypedValue>(final_value, std::move(l->type));
}


// Expressions
NumberExprAST::NumberExprAST(int value) : m_value(value) {}

std::unique_ptr<sTypedValue> NumberExprAST::codegen(std::shared_ptr<cCodeGenerator> code_generator) {
    // return llvm::ConstantInt::get(*code_generator->m_Context, llvm::APInt(this->m_value));
    llvm::Value* val = llvm::ConstantFP::get(*code_generator->m_Context, llvm::APFloat(this->m_value));
    auto type = std::make_unique<TypeExrAST>("float");
    return std::make_unique<sTypedValue>(val, std::move(type));
}

// Variable Expression AST

VariableExprAST::VariableExprAST(const std::string& name) : m_name(name) {}

const std::string& VariableExprAST::get_name() { return m_name; }

std::unique_ptr<sTypedValue> VariableExprAST::codegen(std::shared_ptr<cCodeGenerator> code_generator) {
    std::unique_ptr<sTypedValue> value = std::move(code_generator->m_NamedValues[this->m_name]);
    if (value) { return value; }
    else {
        DEPLANG_PARSER_ERROR("Variable " << this->m_name << " not found");
        return nullptr;
    }
}


TypeExrAST::TypeExrAST(const std::string& name) {
    if (name == "int") { this->m_prim_type = TYPE_INT; }
    else if (name == "bool") { this->m_prim_type = TYPE_BOOL; }
    else if (name == "float") { this->m_prim_type = TYPE_FLOAT; }
    else if (name == "void") { this->m_prim_type = TYPE_EMPTY; }
}

const ePrimitiveType& TypeExrAST::get_primitive_type() { return this->m_prim_type; }

std::unique_ptr<sTypedValue> TypeExrAST::codegen(std::shared_ptr<cCodeGenerator> code_generator) {
    return nullptr;
}


// Binary Expr AST
BinaryExprAST::BinaryExprAST(std::string op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs) :
    m_op(op), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}   

std::unique_ptr<sTypedValue> BinaryExprAST::codegen(std::shared_ptr<cCodeGenerator> code_generator) {
    std::unique_ptr<sTypedValue> l = this->m_lhs->codegen(code_generator);
    std::unique_ptr<sTypedValue> r = this->m_rhs->codegen(code_generator);

    return build_ir_operation(std::move(l), std::move(r), this->m_op, code_generator);
}

// Return Expr AST
ReturnExprAST::ReturnExprAST(std::unique_ptr<ExprAST> expression) : m_expression(std::move(expression)) {}
std::unique_ptr<sTypedValue> ReturnExprAST::codegen(std::shared_ptr<cCodeGenerator> code_generator) {
    if (this->m_expression) { return this->m_expression->codegen(code_generator); }
    return nullptr;
}

// Function Parameter AST
FunctionParameterAST::FunctionParameterAST(const std::string& param_name, std::unique_ptr<TypeExrAST> param_type): m_type_expr(std::move(param_type)), m_param_name(param_name) {}

const std::string& FunctionParameterAST::get_param_name() { return m_param_name; }
const ePrimitiveType& FunctionParameterAST::get_primitive_type() { return m_type_expr->get_primitive_type(); }

// Functio ndefinition AST
FunctionDefinitionAST::FunctionDefinitionAST(const std::string& function_name, std::vector<std::unique_ptr<FunctionParameterAST>> parameters, std::unique_ptr<TypeExrAST> return_type, std::vector<std::unique_ptr<ExprAST>> function_body) : m_function_name(function_name), m_parameters(std::move(parameters)), m_return_type(std::move(return_type)), m_function_body(std::move(function_body)) {}

const std::string& FunctionDefinitionAST::get_function_name() { return m_function_name; }

llvm::Function* FunctionDefinitionAST::codegen(std::shared_ptr<cCodeGenerator> code_generator) {
    // @CHECK: possible memory leak
    // std::vector<llvm::Type*> doubles(this->m_parameters.size(),
    //                     llvm::Type::getDoubleTy(*code_generator->m_Context));

    std::vector<llvm::Type*> param_types;
    for (auto& param : this->m_parameters) {
        param_types.push_back(get_llvm_type(param->get_primitive_type(), *code_generator->m_Context));
    }

    // std::vector<std::shared_ptr<llvm::Type>> doubles(this->m_parameters.size(),
    //                 std::make_shared<llvm::Type>(llvm::Type::getDoubleTy(*code_generator->m_Context)));


    // Construct a function type
    // @TODO: Check if can be changed for the new type system

    llvm::Type* func_return_type = get_llvm_type(this->m_return_type->get_primitive_type(), *code_generator->m_Context);
    llvm::FunctionType* func_type = llvm::FunctionType::get(func_return_type, param_types, false);
    llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, this->m_function_name, code_generator->m_Module.get());

    code_generator->m_NamedValues.clear();
    unsigned index = 0;
    for (auto& arg : func->args()) {
        arg.setName(this->m_parameters[index]->get_param_name()); 
        std::cout << "Adding parameter: " << std::string(arg.getName()) << std::endl;
        // @TODO: Set arg type
        code_generator->m_NamedValues[std::string(arg.getName())] = std::make_unique<sTypedValue>(&arg, std::move(this->m_parameters[index]->m_type_expr));
        index++;
    }

    if (!func) { return nullptr; }

    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*code_generator->m_Context, "entry", func);
    code_generator->m_Builder->SetInsertPoint(bb);

    // code_generator->m_NamedValues.clear();
    std::unique_ptr<sTypedValue> value;
    for (auto& expr : this->m_function_body) {
        value = expr->codegen(code_generator);
        if (dynamic_cast<ReturnExprAST*>(expr.get())) {
            code_generator->m_Builder->CreateRet(value->value);
            break;
        }
        // @TODO: On Error reading function body, remove function
        // func->eraseFromParent();
    }

    llvm::verifyFunction(*func);
    return func;
}


// Variable Declaration Expression
VariableDeclarationExprAST::VariableDeclarationExprAST(const std::string& variable_name, std::unique_ptr<TypeExrAST> variable_type) : m_variable_name(variable_name), m_variable_type(std::move(variable_type)), m_expression(nullptr) {}

VariableDeclarationExprAST::VariableDeclarationExprAST(const std::string& variable_name, std::unique_ptr<TypeExrAST> variable_type, std::unique_ptr<ExprAST> expression) : m_variable_name(variable_name), m_variable_type(std::move(variable_type)), m_expression(std::move(expression)) {}

const std::string& VariableDeclarationExprAST::get_variable_name() { return m_variable_name; }
const ePrimitiveType& VariableDeclarationExprAST::get_primitive_type() { return m_variable_type->get_primitive_type(); }

std::unique_ptr<sTypedValue> VariableDeclarationExprAST::codegen(std::shared_ptr<cCodeGenerator> code_generator) {
    std::unique_ptr<sTypedValue> value;
    if (this->m_expression) { value = this->m_expression->codegen(code_generator); }
    code_generator->m_NamedValues[this->m_variable_name] = std::move(value);
    return value;
}

// Call Expression AST
CallExprAST::CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args) :
    m_callee(callee), m_args(std::move(args)) {}

std::unique_ptr<sTypedValue> CallExprAST::codegen(std::shared_ptr<cCodeGenerator> code_generator) {
    llvm::Function* callee_f = code_generator->m_Module->getFunction(this->m_callee);
    if (!callee_f) {
        DEPLANG_PARSER_ERROR("Function " << this->m_callee << " not found");
        return nullptr;
    }

    if (callee_f->arg_size() != this->m_args.size()) {
        DEPLANG_PARSER_ERROR("Expected " << callee_f->arg_size() << ", got " << this->m_args.size() << "arguments");
        return nullptr;
    }

    std::vector<llvm::Value*> args_v;
    for (unsigned i = 0, e = this->m_args.size(); i != e; ++i) {
        args_v.push_back(this->m_args[i]->codegen(code_generator)->value);
        if (!args_v.back()) { return nullptr; }
    }

    llvm::Value* val = code_generator->m_Builder->CreateCall(callee_f, args_v, "calltmp");
    return std::make_unique<sTypedValue>(val, std::make_unique<TypeExrAST>("float"));
}

// Assignment Expr AST
AssignmentExprAST::AssignmentExprAST(const std::string& variable, std::unique_ptr<ExprAST> rhs) : m_variable(variable), m_rhs(std::move(rhs)) {}
const std::string& AssignmentExprAST::get_variable_name() { return m_variable; }

std::unique_ptr<sTypedValue> AssignmentExprAST::codegen(std::shared_ptr<cCodeGenerator> code_generator) {
    auto value = this->m_rhs->codegen(code_generator);
    code_generator->m_NamedValues[this->m_variable] = std::move(value);
    return value;
}

// Parser
cParser::cParser(std::vector<sToken> tokens) : m_code_generator(std::make_shared<cCodeGenerator>()),
    m_tokens(std::move(tokens)), m_current_index(0) {}

sToken cParser::get_next_token() {
    while (this->m_tokens[this->m_current_index].token_type == TOK_COMMENT) {
        this->m_current_index++;
    } 
    return this->m_current_token = this->m_tokens[this->m_current_index++];
}

const sToken& cParser::peek_next_token() {
    while (this->m_tokens[this->m_current_index].token_type == TOK_COMMENT) {
        this->m_current_index++;
    } 
    return this->m_tokens[this->m_current_index];
}


void cParser::emit_object_code(std::string object_file_name) {
    // Initialize the target registry etc.
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    this->m_code_generator->m_Module->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        llvm::errs() << Error;
        exit(1);
    }

    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto TheTargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, llvm::Reloc::PIC_);
    this->m_code_generator->m_Module->setDataLayout(TheTargetMachine->createDataLayout());

    std::error_code EC;
    llvm::raw_fd_ostream dest(object_file_name, EC, llvm::sys::fs::OF_None);

    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        exit(1);
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CodeGenFileType::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        exit(1);
    }

    pass.run(*this->m_code_generator->m_Module);
    dest.flush();

    llvm::outs() << "Wrote " << object_file_name << "\n";
}


std::unique_ptr<ExprAST> cParser::parse_number_expr() {
    sToken peeked_token = this->peek_next_token();
    int num_value = atoi(peeked_token.value.c_str());
    std::cout << "Found Numeric Value: " << num_value << std::endl;
    auto result = std::make_unique<NumberExprAST>(num_value);
    this->get_next_token(); // Consume number

    return std::move(result);
}


// '(' expression ')'
std::unique_ptr<ExprAST> cParser::parse_paren_expr() {
    this->get_next_token(); // Consume '('
    // Parse Expression
    if (this->m_current_token.token_type != TOK_RIGHTPAR) {
        auto result = this->parse_expression();
        this->get_next_token(); // Consume ')'
        return result;
    }

    this->get_next_token(); // Consume ')'
    return nullptr;
}


// identifier_expr := identifier | identifier '=' expr ';' | function_call
std::unique_ptr<ExprAST> cParser::parse_identifier_expr() {
    // this->get_next_token();
    sToken peeked_token = this->peek_next_token();
    std::string identifier_name = peeked_token.value;
    this->get_next_token(); // Consume identifier

    peeked_token = this->peek_next_token();

    // Assignment
    if (peeked_token.token_type == TOK_EQUAL) {
        this->get_next_token(); // Consume '='
        // @TODO: Parse Expression
        auto expr = this->parse_expression();
        peeked_token = this->peek_next_token();
        if (peeked_token.token_type == TOK_SEMICOLON) {
            return std::make_unique<AssignmentExprAST>(identifier_name, std::move(expr));
        }
    }

    // Simple variable
    if (peeked_token.token_type != TOK_LEFTPAR)
        return std::make_unique<VariableExprAST>(identifier_name);

    this->get_next_token(); // Consume '('
    peeked_token = this->peek_next_token();

    // Function call
    std::vector<std::unique_ptr<ExprAST>> args;
    // Parse function parameters
    if (peeked_token.token_type != TOK_RIGHTPAR) {
        while (true) {
            if (auto arg = this->parse_expression()) {
                args.push_back(std::move(arg));
            }
            else { return nullptr; }
            // this->get_next_token();
            peeked_token = this->peek_next_token();

            if (peeked_token.token_type == TOK_RIGHTPAR) { 
                this->get_next_token(); // Consume ')'
                break; 
            }

            if (peeked_token.token_type != TOK_COMMA) {
                DEPLANG_PARSER_ERROR("Expected ',' or ')', got " << peeked_token.value << " at line " << peeked_token.line_number);
                return nullptr;
            }

            this->get_next_token(); // Consume ','
        }
    } else { 
        this->get_next_token(); // Consume the ')' if no parameters
    }

    peeked_token = this->peek_next_token();
    if (peeked_token.token_type == TOK_SEMICOLON) {
        std::cout << std::endl;
        // this->get_next_token();
        return std::make_unique<CallExprAST>(identifier_name, std::move(args));
    } else { 
        std::cout << "Expected ; | Got: " << peeked_token.value << std::endl;
        return nullptr; 
    }

}

std::unique_ptr<ExprAST> cParser::parse_primary() {
    // this->get_next_token();

    sToken peeked_token = this->peek_next_token();
    switch (peeked_token.token_type) {
        case TOK_IDENTIFIER:
            return this->parse_identifier_expr();
        case TOK_NUMBER:
            return this->parse_number_expr();
        case TOK_LEFTPAR:
            return this->parse_paren_expr();
        case TOK_VARDECL:
            return this->parse_variable_declaration();
        case TOK_RETURN:
            return this->parse_return_expr();
        default:
            return nullptr;
    }
}

std::unique_ptr<ExprAST> cParser::parse_binop_expression(int expr_prec, std::unique_ptr<ExprAST> lhs) {
    sToken peeked_token;

    while (true) {
        peeked_token = this->peek_next_token();
        if (peeked_token.token_type == TOK_EOF) { return nullptr; }
        std::string op = peeked_token.value;
        int tok_prec = this->get_binop_precedence(op);

        if (tok_prec < expr_prec) { return lhs; }

        this->get_next_token();
        auto rhs = this->parse_primary();
        if (!rhs) { return nullptr; }

        peeked_token = this->peek_next_token();
        int next_prec = this->get_binop_precedence(peeked_token.value);
        if (tok_prec < next_prec) {
            rhs = this->parse_binop_expression(tok_prec + 1, std::move(rhs));
            if (!rhs) { return nullptr; }
        }
        lhs = std::make_unique<BinaryExprAST>(op, std::move(lhs), std::move(rhs));
    }
}

std::unique_ptr<ReturnExprAST> cParser::parse_return_expr() {
    this->get_next_token(); // Consume 'return'
    auto final_expr = this->parse_expression();

    return std::make_unique<ReturnExprAST>(std::move(final_expr));
}

std::unique_ptr<ExprAST> cParser::parse_expression() {
    auto lhs = this->parse_primary();
    if (!lhs) 
        return nullptr;
    return this->parse_binop_expression(0, std::move(lhs));
}


// function_param := identifier ':' identifier
std::unique_ptr<FunctionParameterAST> cParser::parse_function_parameter() {
    sToken peeked_token = this->peek_next_token();

    if (peeked_token.token_type != TOK_IDENTIFIER) {
        DEPLANG_PARSER_ERROR("Expected identifier, got " << peeked_token.value << " at line " << peeked_token.line_number);
        return nullptr;
    }
    
    this->get_next_token(); // Consume identifier

    std::string param_name = peeked_token.value;
    peeked_token = this->peek_next_token();

    if (peeked_token.token_type != TOK_COLON) {
        DEPLANG_PARSER_ERROR("Expected ':', got " << peeked_token.value << " at line " << peeked_token.line_number);
        return nullptr;
    }

    this->get_next_token(); // Consume ':'
    
    peeked_token = this->peek_next_token();

    if (peeked_token.token_type != TOK_IDENTIFIER) {
        DEPLANG_PARSER_ERROR("Expected identifier, got " << peeked_token.value << " at line " << peeked_token.line_number);
        return nullptr;
    }

    this->get_next_token(); // Consume identifier
    std::string param_type = peeked_token.value;
    
    // @TODO: Change to parse type expression
    auto param_type_expr = std::make_unique<TypeExrAST>(param_type);
    return std::make_unique<FunctionParameterAST>(param_name, std::move(param_type_expr));
}




// Parse function definition
// func identifier(arg1, arg2, ...) {
//    expressions_list
// }
std::unique_ptr<FunctionDefinitionAST> cParser::parse_function_definition() {
    this->get_next_token(); // Consume 'func'
    
    sToken peeked_token = this->peek_next_token();
    if (peeked_token.token_type != TOK_IDENTIFIER) {
        DEPLANG_PARSER_ERROR("Expected identifier, got " << peeked_token.value << " at line " << peeked_token.line_number);
        return nullptr;
    }

    this->get_next_token(); // Consume identifier

    std::string function_name = peeked_token.value;
    peeked_token = this->peek_next_token();
    if (peeked_token.token_type != TOK_LEFTPAR) {
        DEPLANG_PARSER_ERROR("Expected '(', got " << peeked_token.value << " at line " << peeked_token.line_number);
        return nullptr;
    }

    this->get_next_token(); // Consume '('

    peeked_token = this->peek_next_token();

    // Parse function parameters
    std::vector<std::unique_ptr<FunctionParameterAST>> args;
    if (peeked_token.token_type != TOK_RIGHTPAR) {
        while (true) {
            auto param = this->parse_function_parameter();
            args.push_back(std::move(param));
            this->get_next_token();

            if (m_current_token.token_type == TOK_RIGHTPAR) 
                break;

            if (m_current_token.token_type != TOK_COMMA) {
                DEPLANG_PARSER_ERROR("Expected ',' or ')', got " << peeked_token.value << " at line " << peeked_token.line_number);
                return nullptr;
            }
        }
    } else { 
        this->get_next_token(); // Consume the ')' if no parameters
    }

    // this->get_next_token();
    peeked_token = this->peek_next_token();

    std::string return_type = "void";
    if (peeked_token.token_type == TOK_ARROW) {
        this->get_next_token(); // Consume '->'

        peeked_token = this->peek_next_token();
        
        if (peeked_token.token_type != TOK_IDENTIFIER) {
            DEPLANG_PARSER_ERROR("Expected identifier got " << peeked_token.value << " at line " << peeked_token.line_number);
            return nullptr;
        }

        this->get_next_token(); // Consume identifier
        return_type = peeked_token.value;

        // this->get_next_token(); // Move to the '{'
        peeked_token = this->peek_next_token();
    }

    if (peeked_token.token_type != TOK_LEFTCURBRACE) {
        DEPLANG_PARSER_ERROR("Expected '{' got " << peeked_token.value << " at line " << peeked_token.line_number);
        return nullptr;
    }

    this->get_next_token(); // consume '{'

    std::vector<std::unique_ptr<ExprAST>> fn_body;
    while (true) {
        peeked_token = this->peek_next_token();
        if (peeked_token.token_type == TOK_RIGHTCURBRACE) { break; }

        auto expression = this->parse_expression();
        peeked_token = this->peek_next_token();

        this->get_next_token(); // Consume ';'
        if (!expression) { std::cout << "Got no expression\n"; break; }
        fn_body.push_back(std::move(expression));
    }

    peeked_token = this->peek_next_token();
    if (peeked_token.token_type != TOK_RIGHTCURBRACE) {
        DEPLANG_PARSER_ERROR("Expected '}' got " << peeked_token.value << " at line " << peeked_token.line_number);
        return nullptr;
    }

    this->get_next_token(); // Consume last }

    // @TODO: Change
    auto return_type_expr = std::make_unique<TypeExrAST>(return_type);
    auto function_definition = std::make_unique<FunctionDefinitionAST>(function_name, std::move(args), std::move(return_type_expr), std::move(fn_body));
    return function_definition;
}

std::unique_ptr<VariableDeclarationExprAST> cParser::parse_variable_declaration() {

    this->get_next_token(); // Consume 'let'
    sToken peeked_token = this->peek_next_token();

    if (peeked_token.token_type != TOK_IDENTIFIER) {
        DEPLANG_PARSER_ERROR("Expected identifier got " << peeked_token.value << " at line " << peeked_token.line_number);
        return nullptr;
    }
    
    this->get_next_token(); // Consume identifier

    std::string var_name = peeked_token.value;
    peeked_token = this->peek_next_token();

    if (peeked_token.token_type != TOK_COLON) {
        DEPLANG_PARSER_ERROR("Expected ':' got " << peeked_token.value << " at line " << peeked_token.line_number);
        return nullptr;
    }

    this->get_next_token(); // Consume '='

    peeked_token = this->peek_next_token();
    if (peeked_token.token_type != TOK_IDENTIFIER) {
        DEPLANG_PARSER_ERROR("Expected identifier got " << peeked_token.value << " at line " << peeked_token.line_number);
        return nullptr;
    }

    std::string var_type = peeked_token.value;

    this->get_next_token(); // Consume identifier
    peeked_token = this->peek_next_token();

    if (peeked_token.token_type == TOK_SEMICOLON) {
        auto var_type_expr = std::make_unique<TypeExrAST>(var_type);
        return std::make_unique<VariableDeclarationExprAST>(var_name, std::move(var_type_expr));
    }

    if (peeked_token.token_type == TOK_EQUAL) {
        this->get_next_token(); // Consume '='
        auto expr = this->parse_expression();
        peeked_token = this->peek_next_token();
        if (peeked_token.token_type == TOK_SEMICOLON) {
            auto var_type_expr = std::make_unique<TypeExrAST>(var_type);
            return std::make_unique<VariableDeclarationExprAST>(var_name, std::move(var_type_expr), std::move(expr));
        }
    }

    return nullptr;
}

int cParser::get_binop_precedence(std::string op) {
    if (op == ">" || op == "<" || op == ">=" || op == "<=") { return 10; }
    else if (op == "+" || op == "-") { return 20; }
    else if (op == "*" || op == "/") { return 30; }
    else { return -1; }
}

void cParser::parse() {
    // this->m_current_token = this->m_tokens[0];
    
    while (true) {
        // this->get_next_token();
        
        sToken peeked = this->peek_next_token();
        std::string type = get_token_type_string(peeked.token_type);
        // this->get_next_token();
        // continue;
        switch (peeked.token_type) {
        default:
            return;
        case TOK_EOF:
            std::cout << "Found EOF" << std::endl;
            return;
        case TOK_SEMICOLON:
            this->get_next_token();
            break;
        case TOK_DEF:
            auto func_def = this->parse_function_definition();
            llvm::Function* f = func_def->codegen(this->m_code_generator);
            f->print(llvm::errs());
            break;
        }
    }
}


