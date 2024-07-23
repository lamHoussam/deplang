
#include "../include/lexer.h"

cLexer::cLexer(std::string input_str) {
    this->m_input_str = input_str;
    this->m_current_pos = 0;
}

std::string get_token_type_string(eTokenType token_type) {
    switch (token_type) {
        case TOK_EOF:         return "EOF";
        case TOK_DEF:         return "DEF";
        case TOK_IDENTIFIER:  return "IDENTIFIER";
        case TOK_NUMBER:      return "NUMBER";
        case TOK_UNKNOWN:
        default:              return "UNKNOWN";
    }

    return "";
}

sToken cLexer::get_next_token() {
    char last_char = ' ';
    sToken final_token;

    final_token.value = "";
    final_token.token_type = TOK_UNKNOWN;

    std::string identifier_string;

    while (isspace(last_char)) {
        last_char = this->m_input_str[this->m_current_pos];
        // std::cout << "Last: " << last_char << std::endl;
        this->m_current_pos++;
    }

    if (isalpha(last_char)) {
        identifier_string = last_char;
        while (isalnum(last_char = this->m_input_str[this->m_current_pos])) {
            identifier_string += last_char;
            this->m_current_pos++;
        }

        if (identifier_string == "func") {
            final_token.token_type = TOK_DEF;
            final_token.value = identifier_string;

            return final_token;
        } else {
            final_token.token_type = TOK_IDENTIFIER;
            final_token.value = identifier_string;

            return final_token;
        }
    }

    if (isdigit(last_char)) {
        identifier_string = last_char;
        while (isdigit(last_char = this->m_input_str[this->m_current_pos])) {
            identifier_string += last_char; 
            this->m_current_pos++;
        }

        final_token.token_type = TOK_NUMBER;
        final_token.value = identifier_string;

        return final_token;
    }

    if (last_char == EOF) {
        final_token.token_type = TOK_EOF;
        return final_token;
    }
    
    return final_token;
}

void cLexer::lex() {
    sToken token;
    do {
        token = this->get_next_token();
        this->m_tokens.push_back(token);
    } while (token.token_type != TOK_EOF && token.token_type != TOK_UNKNOWN);
}

void cLexer::print_tokens() {
    std::cout << "Lexer Tokens" << std::endl;
    for (auto token : this->m_tokens)
        std::cout << "Token: " << get_token_type_string(token.token_type) << "; Value: " << token.value << std::endl;
}

