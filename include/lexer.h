#pragma once

#include <iostream>

#include <string>
#include <vector>

enum eTokenType {
    TOK_EOF           = -1,
    TOK_UNKNOWN       = -2,

    TOK_DEF           = -3,

    TOK_IDENTIFIER    = -4,
    TOK_INTEGER       = -5,
    TOK_FLOAT         = -6,
    
    TOK_OP            = -7,
    
    TOK_LEFTPAR       = -8,
    TOK_RIGHTPAR      = -9,
    TOK_COMMA         = -10,
    TOK_SEMICOLON     = -11,

    TOK_LEFTCURBRACE  = -12,
    TOK_RIGHTCURBRACE = -13,

    TOK_COLON         = -14,
    TOK_ARROW         = -15,
    TOK_EQUAL         = -16,

    TOK_VARDECL       = -17,
    TOK_TYPEDECL      = -18,
    TOK_RETURN        = -19,

    TOK_COMMENT       = -20,

    TOK_TRUE          = -21,
    TOK_FALSE         = -22,
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

