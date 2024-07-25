# pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "../include/lexer.h"

class ExprAST {
public:
    virtual ~ExprAST() = default;
};


// Value
class NumberExprAST : public ExprAST {
public:
    NumberExprAST(int value) : m_value(value) {}
private:
    int m_value;
};

// Name
class VariableExprAST : public ExprAST {
public:
    VariableExprAST(const std::string& name) : m_name(name) {}

private:
    std::string m_name;
};

// Expr Op Expr
class BinaryExprAST : public ExprAST {
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs) :
        m_op(op), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}   

private:
    char m_op;
    std::unique_ptr<ExprAST> m_lhs, m_rhs;
};


// Callee([args])
class CallExprAST : public ExprAST {
public:
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args) : 
        m_callee(callee), m_args(std::move(args)) {}

private:
    std::string m_callee;
    std::vector<std::unique_ptr<ExprAST>> m_args;
};


// Name([args])
class PrototypeAST {
public:
    PrototypeAST(const std::string& name, std::vector<std::string> args) : 
        m_name(name), m_args(std::move(args)) {}

    const std::string& get_name() const { return m_name; }

private:
    std::string m_name;
    std::vector<std::string> m_args;
};


class FunctionAST {
public:
    FunctionAST(std::unique_ptr<PrototypeAST> prototype, std::unique_ptr<ExprAST> body) : 
        m_function_proto(std::move(prototype)), m_body(std::move(body)) {}

private:
    std::unique_ptr<PrototypeAST> m_function_proto;
    std::unique_ptr<ExprAST> m_body;
};


class cParser {
public:
    cParser(std::vector<sToken> tokens) : m_tokens(std::move(tokens)), m_current_index(0) {}

    sToken get_next_token();

    std::unique_ptr<ExprAST> parse_number_expr();
    std::unique_ptr<ExprAST> parse_paren_expr();
    std::unique_ptr<ExprAST> parse_expression();
    std::unique_ptr<ExprAST> parse_identifier_expr();
    std::unique_ptr<ExprAST> parse_primary();

    std::string parse_function_parameter();
    std::unique_ptr<ExprAST> parse_function_definition();

    void parse();

    ~cParser() = default;

private:
    std::vector<sToken> m_tokens;
    sToken m_current_token;
    int m_current_index;
};
