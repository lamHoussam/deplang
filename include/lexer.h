#pragma once

#include <iostream>

#include <string>
#include <vector>

enum eTokenType {
    TOK_EOF           = -1,
    TOK_UNKNOWN       = -2,

    TOK_DEF           = -3,

    TOK_IDENTIFIER    = -4,
    TOK_NUMBER        = -5,
    
    TOK_OP            = -6,
    
    TOK_LEFTPAR       = -7,
    TOK_RIGHTPAR      = -8,
    TOK_COMMA         = -9,
    TOK_SEMICOLON     = -10,

    TOK_LEFTCURBRACE  = -11,
    TOK_RIGHTCURBRACE = -12,

    TOK_COLON         = -13,
    TOK_ARROW         = -14,
    TOK_EQUAL         = -15,

    TOK_VARDECL       = -16,
    TOK_RETURN        = -17,

    TOK_COMMENT       = -18,
};

std::string get_token_type_string(eTokenType token_type);

struct sToken {
    eTokenType token_type;   
    std::string value;
    int line_number;
};


class cLexer {
public:
    cLexer(std::string input_str);

    sToken get_next_token();
    void lex();
    inline char consume_char();
    inline char peek_char() const;
    void print_tokens() const;
    const std::vector<sToken>& get_tokens();

    ~cLexer() = default;
private:
    std::string m_input_str;
    int m_current_pos;
    std::vector<sToken> m_tokens;
    int m_current_line_count;
};

