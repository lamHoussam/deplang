#include "../include/parser.h"
#include <iostream>
#include <memory>

/**
* Function call
* Variable declaration
* Numbers
* Binary operations
*
*/


// @TODO: Implement Error management

sToken cParser::get_next_token() {
    return this->m_current_token = this->m_tokens[this->m_current_index++];
}

std::unique_ptr<ExprAST> cParser::parse_number_expr() {
    sToken* token = &this->m_current_token;
    int num_value = atoi(token->value.c_str());
    auto result = std::make_unique<NumberExprAST>(num_value);
    return std::move(result);
}


// ( Expression )
std::unique_ptr<ExprAST> cParser::parse_paren_expr() {
    this->get_next_token();
    // Parse Expression
    if (this->m_current_token.token_type != TOK_RIGHTPAR) {
        auto result = this->parse_expression();
        return result;
    }

    this->get_next_token(); // Consume ')'
    return nullptr;
}


std::unique_ptr<ExprAST> cParser::parse_identifier_expr() {
    // this->get_next_token();
    std::string identifier_name = this->m_current_token.value;
    this->get_next_token();

    // Simple variable
    if (this->m_current_token.token_type != TOK_LEFTPAR)
        return std::make_unique<VariableExprAST>(identifier_name);

    // Function call
    std::vector<std::unique_ptr<ExprAST>> args;

    eTokenType peeked_token_type = this->m_tokens[this->m_current_index].token_type;

    std::cout << "Calling function (" << identifier_name << ") with args ";
    // Parse function parameters
    if (peeked_token_type != TOK_RIGHTPAR) {
        while (true) {
            if (auto arg = this->parse_expression()) {
                if (VariableExprAST* var_expr = dynamic_cast<VariableExprAST*>(arg.get())) {
                    std::cout << var_expr->get_name() << ", ";
                }
                args.push_back(std::move(arg));
            }
            else
                return nullptr;
            // std::cout << "Got param: " << param << std::endl;

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

    std::cout << std::endl;

    this->get_next_token();
    return std::make_unique<CallExprAST>(identifier_name, std::move(args));
}

std::unique_ptr<ExprAST> cParser::parse_primary() {
    // eTokenType peeked_token_type = this->m_tokens[this->m_current_index].token_type;
    this->get_next_token();

    switch (this->m_current_token.token_type) {
        case TOK_IDENTIFIER:
            return this->parse_identifier_expr();
        case TOK_NUMBER:
            return this->parse_number_expr();
        case TOK_LEFTPAR:
            return this->parse_paren_expr();
        case TOK_VARDECL:
            return this->parse_variable_declaration();
        default:
            return nullptr;
    }

    return nullptr;
}

std::unique_ptr<ExprAST> cParser::parse_expression() {
    auto lhs = this->parse_primary();
    if (!lhs) 
        return nullptr;

    return lhs;
}

std::unique_ptr<FunctionParameterAST> cParser::parse_function_parameter() {
    this->get_next_token();
    if (this->m_current_token.token_type != TOK_IDENTIFIER) {
        // Error
        std::cerr << "Expected Identifier" << std::endl;
        return nullptr;
    }
    std::string param_name = this->m_current_token.value;
    this->get_next_token();
    if (this->m_current_token.token_type != TOK_COLON) {
        // Error
        std::cerr << "Expected :" << std::endl;
        return nullptr;
    }

    this->get_next_token();
    
    // std::cout << "Param: " << param_name << "; Type: " << this->m_current_token.value << std::endl;
    if (this->m_current_token.token_type != TOK_IDENTIFIER) {
        // Error
        std::cerr << "Expected Identifier (type)" << std::endl;
        return nullptr;
    }
    std::string param_type = this->m_current_token.value;

    return std::make_unique<FunctionParameterAST>(param_name, param_type);
}

// Parse function definition
// func identifier(arg1, arg2, ...) {
//    expressions_list
// }
std::unique_ptr<FunctionDefinitionAST> cParser::parse_function_definition() {
    this->get_next_token();
    if (this->m_current_token.token_type != TOK_IDENTIFIER) {
        // Error
        std::cerr << "Expected Identifier" << std::endl;
        return nullptr;
    }

    std::string function_name = this->m_current_token.value;
    this->get_next_token();
    if (this->m_current_token.token_type != TOK_LEFTPAR) {
        // Error
        std::cerr << "Expected (" << std::endl;
        return nullptr;
    }

    eTokenType peeked_token_type = this->m_tokens[this->m_current_index].token_type;

    // Parse function parameters
    std::vector<std::unique_ptr<FunctionParameterAST>> args;
    if (peeked_token_type != TOK_RIGHTPAR) {
        while (true) {
            auto param = this->parse_function_parameter();
            args.push_back(std::move(param));
            // std::cout << "Got param: " << param << std::endl;
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

    this->get_next_token();
    std::string return_type = "void";
    std::cout << "-----------------" << this->m_current_token.value << std::endl;
    if (this->m_current_token.token_type == TOK_ARROW) {
        this->get_next_token();
        if (this->m_current_token.token_type != TOK_IDENTIFIER) {
            // Error
            std::cerr << "Expected Identifier" << std::endl;
            return nullptr;
        }

        return_type = this->m_current_token.value;

        this->get_next_token(); // Move to the '{'
    }

    if (this->m_current_token.token_type != TOK_LEFTCURBRACE) {
        // Error
        std::cerr << "Expected {" << std::endl;
        return nullptr;
    }


    std::cout << "Parsed function: " << std::endl;
    std::cout << "Function name: " << function_name << std::endl;
    std::cout << "Args: " << std::endl;

    for (auto& arg: args) {
        std::cout << "Name: " << arg->get_param_name();
        std::cout << "; Type: " << arg->get_param_type() << std::endl;
    }

    std::vector<std::unique_ptr<ExprAST>> fn_body;
    while (true) {
        if (this->m_current_token.token_type == TOK_RIGHTCURBRACE) { break; }

        auto expression = this->parse_expression();
        if (!expression) { break; }
        

        fn_body.push_back(std::move(expression));

        /**
        this->get_next_token(); // Consume ;
        if (this->m_current_token.token_type != TOK_SEMICOLON) {
            // Error
            std::cerr << "Expected ;" << std::endl;
            break;
        }
        */
    }

    // @TODO: Test with shared_ptr
    auto function_definition = std::make_unique<FunctionDefinitionAST>(function_name, std::move(args), return_type, std::move(fn_body));

    std::cout << "Return type: " << return_type;
    std::cout << std::endl << "End Function" << std::endl;

    // this->get_next_token(); // Consume last }
    return function_definition;
}

std::unique_ptr<VariableDeclarationExprAST> cParser::parse_variable_declaration() {
    this->get_next_token();
    if (this->m_current_token.token_type != TOK_IDENTIFIER) {
        // Error
        std::cerr << "Expected identifier" << std::endl;
        return nullptr;
    }
    std::string var_name = this->m_current_token.value;
    
    this->get_next_token();
    if (this->m_current_token.token_type != TOK_COLON) {
        // error
        std::cerr << "Expected :" << std::endl;
        return nullptr;
    }

    this->get_next_token();
    if (this->m_current_token.token_type != TOK_IDENTIFIER) {
        // error
        std::cerr << "Expected identifier" << std::endl;
        return nullptr;
    }

    std::string var_type = this->m_current_token.value;

    this->get_next_token();
    if (this->m_current_token.token_type == TOK_SEMICOLON) {
        std::cout << "Declared variable: " << var_name << " of type " << var_type << std::endl;
        return std::make_unique<VariableDeclarationExprAST>(var_name, var_type);
    }

    if (this->m_current_token.token_type == TOK_EQUAL) {
        // @TODO: Parse Expression
        auto expr = this->parse_expression();
        this->get_next_token();
        std::cout << "Declared variable: " << var_name << " of type " << var_type << "; with expression" << std::endl;
        if (this->m_current_token.token_type == TOK_SEMICOLON) {
            return std::make_unique<VariableDeclarationExprAST>(var_name, var_type, std::move(expr));
        }
    }

    return nullptr;
}

void cParser::parse() {
    // this->m_current_token = this->m_tokens[0];

    while (true) {
        this->get_next_token();
        std::string type = get_token_type_string(this->m_current_token.token_type);
        std::cout << "Found: " << type << std::endl;
        switch (this->m_current_token.token_type) {
        case TOK_EOF:
            return;
        case TOK_SEMICOLON:
            this->get_next_token();
            break;
        case TOK_DEF:
            this->parse_function_definition();
            break;
        default:
            return;
        }
    }
}


