#include <cctype>
#include <cstdio>
#include <iostream>

#include <string>
#include <vector>

enum eTokenType {
    TOK_EOF         = -1,
    TOK_UNKNOWN     = -2,
    TOK_DEF         = -3,
    TOK_IDENTIFIER  = -4,
    TOK_NUMBER      = -5,
};
inline std::string get_token_type_string(eTokenType token_type);

struct sToken {
    eTokenType token_type;   
    std::string value;
};

class cLexer {
public:
    cLexer(std::string input_str);

    sToken get_next_token();
    void lex();
    void print_tokens();

    ~cLexer() = default;
private:
    std::string m_input_str;
    int m_current_pos;
    std::vector<sToken> m_tokens;
};

