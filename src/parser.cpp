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
    this->get_next_token();
    std::string identifier_name = this->m_current_token.value;

    // Simple variable
    if (this->m_current_token.token_type != TOK_LEFTPAR)
        return std::make_unique<VariableExprAST>(identifier_name);

    // Function call
    this->get_next_token();
    std::vector<std::unique_ptr<ExprAST>> args;
    if (this->m_current_token.token_type != TOK_RIGHTPAR) {
        while (true) {
            if (auto arg = this->parse_expression())
                args.push_back(std::move(arg));
            else
                return nullptr;

            if (m_current_token.token_type == TOK_RIGHTPAR) 
                break;

            if (m_current_token.token_type != TOK_COMMA)
                // Error
                return nullptr;

            this->get_next_token();
        }
    }

    this->get_next_token();
    return std::make_unique<CallExprAST>(identifier_name, std::move(args));
}

std::unique_ptr<ExprAST> cParser::parse_primary() {
    eTokenType peeked_token_type = this->m_tokens[this->m_current_index].token_type;

    switch (peeked_token_type) {
        case TOK_IDENTIFIER:
            return this->parse_identifier_expr();
        case TOK_NUMBER:
            return this->parse_number_expr();
        case TOK_LEFTPAR:
            return this->parse_paren_expr();
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


// @TODO: Change return type
std::string cParser::parse_function_parameter() {
    this->get_next_token();
    if (this->m_current_token.token_type != TOK_IDENTIFIER) {
        // Error
        std::cerr << "Expected Identifier" << std::endl;
        return "";
    }
    std::string param_name = this->m_current_token.value;
    this->get_next_token();
    // std::cout << "Param: " << param_name << "; Type: " << this->m_current_token.value << std::endl;
    if (this->m_current_token.token_type != TOK_IDENTIFIER) {
        // Error
        std::cerr << "Expected Identifier (type)" << std::endl;
        return "";
    }
    return param_name;
}


// Parse function definition
// func identifier(arg1, arg2, ...) {
//    expressions_list
// }
std::unique_ptr<ExprAST> cParser::parse_function_definition() {
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

    std::vector<std::string> args;
    if (this->m_current_token.token_type != TOK_RIGHTPAR) {
        while (true) {
            std::string param = this->parse_function_parameter();
            args.push_back(param);
            // std::cout << "Got param: " << param << std::endl;
            this->get_next_token();

            if (m_current_token.token_type == TOK_RIGHTPAR) 
                break;

            if (m_current_token.token_type != TOK_COMMA) {
                // Error
                std::cerr << "Expected , or )" << std::endl;
                return nullptr;
            }

            // this->get_next_token();
        }
    }

    std::cout << "Parsed function: " << std::endl;
    std::cout << "Function name: " << function_name << std::endl;
    std::cout << "Args: " << std::endl;

    for (auto arg : args)
        std::cout << arg << ", ";

    std::cout << std::endl << "End Function" << std::endl;
    return nullptr;
}

void cParser::parse() {
    // this->m_current_token = this->m_tokens[0];
    this->get_next_token();
    std::string type = get_token_type_string(this->m_current_token.token_type);
    std::cout << "Found: " << type << std::endl;

    while (true) {
        switch (this->m_current_token.token_type) {
        case TOK_EOF:
            return;
        case TOK_SEMICOLON:
            this->get_next_token();
            break;
        case TOK_DEF:
            this->parse_function_definition();
            return;
        default:
            return;
        }
    }
}


