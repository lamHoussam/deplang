#include "../include/parser.h"

/**
* Function call
* Variable declaration
* Numbers
* Binary operations
*
*/

// @CHECK: Don't Consume ';' at end of expression



void initialize_module() {
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>("JIT", *TheContext);
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}

llvm::Value* NumberExprAST::codegen() {
    // return llvm::ConstantInt::get(*TheContext, llvm::APInt(this->m_value));
    return llvm::ConstantFP::get(*TheContext, llvm::APFloat(this->m_value));
}

llvm::Value* VariableExprAST::codegen() {
    llvm::Value* value = NamedValues[this->m_name];
    if (!value) {
        // Error
        std::cerr << "Error variable not found" << std::endl;
    }
    return value;
}

llvm::Value* BinaryExprAST::codegen() {
    llvm::Value* l = this->m_lhs->codegen();
    llvm::Value* r = this->m_rhs->codegen();

    if (!l || !r) { return nullptr; }
    switch (this->m_op) {
    case '+':
        return Builder->CreateFAdd(l, r, "addtmp");
    case '-':
        return Builder->CreateFSub(l, r, "subtmp");
    case '*':
        return Builder->CreateFMul(l, r, "multmp");
    // case '<':
    //     l = Builder->CreateFCmpULT(l, r, "cmptmp");
    //     return Builder->CreateUIToFP(l, llvm::Type::getDoubleTy(TheContext), "booltmp");
    default:
        // Error
        std::cerr << "Error " << std::endl;
        return nullptr;    
    }
}

llvm::Value* CallExprAST::codegen() {
    llvm::Function* callee_f = TheModule->getFunction(this->m_callee);
    if (!callee_f) {
        // Error
        std::cerr << "Unknown function referenced" << std::endl;
        return nullptr;
    }

    if (callee_f->arg_size() != this->m_args.size()) {
        // Error
        std::cerr << "Incorrect number of arguments" << std::endl;
        return nullptr;
    }

    std::vector<llvm::Value*> args_v;
    for (unsigned i = 0, e = this->m_args.size(); i != e; ++i) {
        args_v.push_back(this->m_args[i]->codegen());
        if (!args_v.back()) { return nullptr; }
    }

    return Builder->CreateCall(callee_f, args_v, "calltmp");
}

llvm::Value* VariableDeclarationExprAST::codegen() {
    llvm::Value* value = nullptr;
    if (this->m_expression) { value = this->m_expression->codegen(); }
    return NamedValues[this->m_variable_name] = value;
}

llvm::Value* AssignmentExprAST::codegen() {
    llvm::Value* value = this->m_rhs->codegen();
    return NamedValues[this->m_variable] = value;
}



// @TODO: Implement Error management
sToken cParser::get_next_token() {
    return this->m_current_token = this->m_tokens[this->m_current_index++];
}

const sToken& cParser::peek_next_token() {
    return this->m_tokens[this->m_current_index];
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
    std::cout << "Token found here: " << peeked_token.value << std::endl;

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
                // Error
                std::cerr << "Expected , or ) | Got: " << peeked_token.value << std::endl;
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
        int op = peeked_token.value[0];
        int tok_prec = this->get_binop_precedence(op);

        if (tok_prec < expr_prec) { return lhs; }

        this->get_next_token();
        auto rhs = this->parse_primary();
        if (!rhs) { return nullptr; }

        peeked_token = this->peek_next_token();
        int next_op = peeked_token.value[0];
        int next_prec = this->get_binop_precedence(next_op);
        if (tok_prec < next_prec) {
            rhs = this->parse_binop_expression(tok_prec + 1, std::move(rhs));
            if (!rhs) { return nullptr; }
        }
        lhs = std::make_unique<BinaryExprAST>(op, std::move(lhs), std::move(rhs));
    }
}

std::unique_ptr<ExprAST> cParser::parse_return_expr() {
    std::cout << "Parse return expr" << std::endl;
    this->get_next_token(); // Consume 'return'
    auto final_expr = this->parse_expression();
    // @TODO: Code generation
    return final_expr;
}

std::unique_ptr<ExprAST> cParser::parse_expression() {
    auto lhs = this->parse_primary();
    if (!lhs) 
        return nullptr;

    // @TODO: Remove after rewriting parse binop 
    // sToken peeked_token = this->peek_next_token();
    // std::cout << "Token After primary expression: " << peeked_token.value << std::endl;

    return this->parse_binop_expression(0, std::move(lhs));
}


// function_param := identifier ':' identifier
std::unique_ptr<FunctionParameterAST> cParser::parse_function_parameter() {
    sToken peeked_token = this->peek_next_token();

    if (peeked_token.token_type != TOK_IDENTIFIER) {
        // Error
        std::cerr << "Expected Identifier" << std::endl;
        return nullptr;
    }
    
    this->get_next_token(); // Consume identifier

    std::string param_name = peeked_token.value;
    peeked_token = this->peek_next_token();

    if (peeked_token.token_type != TOK_COLON) {
        // Error
        std::cerr << "Expected :" << std::endl;
        return nullptr;
    }

    this->get_next_token(); // Consume ':'
    
    peeked_token = this->peek_next_token();

    if (peeked_token.token_type != TOK_IDENTIFIER) {
        // Error
        std::cerr << "Expected Identifier (type)" << std::endl;
        return nullptr;
    }

    this->get_next_token(); // Consume identifier
    std::string param_type = peeked_token.value;
    return std::make_unique<FunctionParameterAST>(param_name, param_type);
}



llvm::Function* FunctionDefinitionAST::codegen() {
    std::vector<llvm::Type*> doubles(this->m_parameters.size(),
                                llvm::Type::getDoubleTy(*TheContext));


    // Construct a function type
    // @TODO: Check if can be changed for the new type system
    llvm::FunctionType* func_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(*TheContext), doubles, false);

    llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, this->m_function_name, TheModule.get());

    unsigned index = 0;
    for (auto& arg : func->args()) { arg.setName(this->m_parameters[index++]->get_param_name()); }
    
    return func;
}

// Parse function definition
// func identifier(arg1, arg2, ...) {
//    expressions_list
// }
std::unique_ptr<FunctionDefinitionAST> cParser::parse_function_definition() {
    this->get_next_token(); // Consume 'func'
    
    sToken peeked_token = this->peek_next_token();
    if (peeked_token.token_type != TOK_IDENTIFIER) {
        // Error
        std::cerr << "Expected Identifier" << std::endl;
        return nullptr;
    }

    this->get_next_token(); // Consume identifier

    std::string function_name = peeked_token.value;
    peeked_token = this->peek_next_token();
    if (peeked_token.token_type != TOK_LEFTPAR) {
        // Error
        std::cerr << "Expected (" << std::endl;
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
                // Error
                std::cerr << "Expected , or )" << std::endl;
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
            // Error
            std::cerr << "Expected Identifier" << std::endl;
            return nullptr;
        }

        this->get_next_token(); // Consume identifier

        return_type = peeked_token.value;

        // this->get_next_token(); // Move to the '{'
        peeked_token = this->peek_next_token();
    }

    if (peeked_token.token_type != TOK_LEFTCURBRACE) {
        // Error
        std::cerr << "Expected {" << std::endl;
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

        // @CHECK: Code generation
        llvm::Value* value = expression->codegen();
        if (value) { value->print(llvm::errs()); std::cout << std::endl; } 
        else { std::cout << ">>>>>>>> No Value" << std::endl; }

        fn_body.push_back(std::move(expression));
    }

    peeked_token = this->peek_next_token();
    if (peeked_token.token_type != TOK_RIGHTCURBRACE) {
        // Error
        std::cerr << "Expected }; Got: " << peeked_token.value << std::endl;
        return nullptr;
    }

    this->get_next_token(); // Consume last }

    auto function_definition = std::make_unique<FunctionDefinitionAST>(function_name, std::move(args), return_type, std::move(fn_body));
    return function_definition;
}

std::unique_ptr<VariableDeclarationExprAST> cParser::parse_variable_declaration() {

    this->get_next_token(); // Consume 'let'
    sToken peeked_token = this->peek_next_token();

    if (peeked_token.token_type != TOK_IDENTIFIER) {
        // Error
        std::cerr << "Expected identifier" << std::endl;
        return nullptr;
    }
    
    this->get_next_token(); // Consume identifier

    std::string var_name = peeked_token.value;
    peeked_token = this->peek_next_token();

    if (peeked_token.token_type != TOK_COLON) {
        // error
        std::cerr << "Expected :" << std::endl;
        return nullptr;
    }

    this->get_next_token(); // Consume '='

    peeked_token = this->peek_next_token();
    if (peeked_token.token_type != TOK_IDENTIFIER) {
        // error
        std::cerr << "Expected identifier" << std::endl;
        return nullptr;
    }

    std::string var_type = peeked_token.value;

    this->get_next_token(); // Consume identifier
    peeked_token = this->peek_next_token();

    if (peeked_token.token_type == TOK_SEMICOLON) {
        return std::make_unique<VariableDeclarationExprAST>(var_name, var_type);
    }

    if (peeked_token.token_type == TOK_EQUAL) {
        this->get_next_token(); // Consume '='
        // @TODO: Parse Expression
        auto expr = this->parse_expression();
        peeked_token = this->peek_next_token();
        if (peeked_token.token_type == TOK_SEMICOLON) {
            return std::make_unique<VariableDeclarationExprAST>(var_name, var_type, std::move(expr));
        }
    }

    return nullptr;
}

int cParser::get_binop_precedence(char op) {
    switch (op) {
        case '<': return 10;
        case '+':
        case '-': return 20;
        case '*': return 30;
        default:  return -1;
    }
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
            llvm::Function* f = func_def->codegen();
            f->print(llvm::errs());
            break;
        }
    }
}


