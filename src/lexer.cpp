
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
        case TOK_OP:          return "OPERATOR";
        case TOK_LEFTPAR:     return "LEFTPAR";
        case TOK_RIGHTPAR:    return "RIGHTPAR";
        case TOK_COMMA:       return "COMMA";
        case TOK_SEMICOLON:   return "SEMICOLON";

        case TOK_UNKNOWN:
        default:              return "UNKNOWN";
    }

    return "";
}

inline bool is_operator(char ch) { return ch == '+' || ch == '-' || ch == '*' || ch == '/'; }

sToken cLexer::get_next_token() {
    char last_char = ' ';
    sToken final_token;

    final_token.value = "";
    final_token.token_type = TOK_UNKNOWN;

    std::string identifier_string;

    while (isspace(last_char))
        last_char = this->consume_char();


    // @TODO: Switch to switch statement

    if (last_char == ';') {
        final_token.token_type = TOK_SEMICOLON;
        final_token.value = ';';
        return final_token;
    }

    if (last_char == '(') {
        final_token.token_type = TOK_LEFTPAR;
        final_token.value = '(';
        return final_token;
    }

    if (last_char == ')') {
        final_token.token_type = TOK_RIGHTPAR;
        final_token.value = ')';
        return final_token;
    }

    if (last_char == ',') {
        final_token.token_type = TOK_COMMA;
        final_token.value = ',';
        return final_token;
    }

    if (last_char == '{') {
        final_token.token_type = TOK_LEFTCURBRACE;
        final_token.value = '{';
        return final_token;
    }

    if (last_char == '}') {
        final_token.token_type = TOK_RIGHTCURBRACE;
        final_token.value = '}';
        return final_token;
    }

    // Alpha
    if (isalpha(last_char) || last_char == '_') {
        identifier_string = last_char;
        while (isalnum(last_char = this->peek_char()) || last_char == '_') {
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

    // Operator
    if (is_operator(last_char)) {
        final_token.token_type = TOK_OP;
        final_token.value = last_char;
        return final_token;
    }

    // Number
    if (isdigit(last_char)) {
        identifier_string = last_char;
        while (isdigit(last_char = this->peek_char())) {
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


char cLexer::consume_char() {
    return this->m_input_str[this->m_current_pos++];
}


char cLexer::peek_char() const {
    return this->m_input_str[this->m_current_pos];
}

void cLexer::lex() {
    sToken token;
    do {
        token = this->get_next_token();
        this->m_tokens.push_back(token);
    } while (token.token_type != TOK_EOF && token.token_type != TOK_UNKNOWN);
}

void cLexer::print_tokens() const {
    std::cout << "Lexer Tokens" << std::endl;
    for (auto token : this->m_tokens)
        std::cout << "Token: " << get_token_type_string(token.token_type) << "; Value: " << token.value << std::endl;
}

