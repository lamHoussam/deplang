# pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>


#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
// #include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "llvm/MC/TargetRegistry.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "llvm/Support/Host.h"


#include "../include/lexer.h"

class cCodeGenerator {
public:
    cCodeGenerator();

    std::unique_ptr<llvm::LLVMContext> m_Context;
    std::unique_ptr<llvm::IRBuilder<>> m_Builder;

    std::unique_ptr<llvm::Module> m_Module;

    std::map<std::string, llvm::Value*> m_NamedValues;
    std::map<std::string, llvm::Value*> m_ArgumentsValues;

    ~cCodeGenerator() = default;
private:
};

class ExprAST {
public:
    virtual ~ExprAST() = default;
    virtual llvm::Value* codegen(cCodeGenerator* code_generator) = 0;
};


// Value
class NumberExprAST : public ExprAST {
public:
    NumberExprAST(int value) : m_value(value) {}
    llvm::Value* codegen(cCodeGenerator* code_generator) override;

private:
    float m_value;
};

// Name
class VariableExprAST : public ExprAST {
public:
    VariableExprAST(const std::string& name) : m_name(name) {}

    const std::string& get_name() { return m_name; }
    llvm::Value* codegen(cCodeGenerator* code_generator) override;
private:
    std::string m_name;
};

// Expr Op Expr
class BinaryExprAST : public ExprAST {
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs) :
        m_op(op), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}   

    llvm::Value* codegen(cCodeGenerator* code_generator) override;
private:
    char m_op;
    std::unique_ptr<ExprAST> m_lhs, m_rhs;
};

class ReturnExprAST : public ExprAST {
public:
    ReturnExprAST(std::unique_ptr<ExprAST> expression) : m_expression(std::move(expression)) {}
    llvm::Value* codegen(cCodeGenerator* code_generator) override;
private:
    std::unique_ptr<ExprAST> m_expression;
};


// Function parameter expression
// function_parameter := identifier ":" identifier
class FunctionParameterAST {
public:
   FunctionParameterAST(const std::string& param_name, const std::string& param_type): m_param_name(param_name), m_param_type(param_type) {}

    inline const std::string& get_param_name() { return m_param_name; }
    inline const std::string& get_param_type() { return m_param_type; }

private:
    std::string m_param_name, m_param_type;
};

// Function definition
// func identifier "(" function_parameter* ")" "{"
//    expression* ";"
// "}"
class FunctionDefinitionAST {
public: 
    FunctionDefinitionAST(const std::string& function_name, std::vector<std::unique_ptr<FunctionParameterAST>> parameters, const std::string& return_type, std::vector<std::unique_ptr<ExprAST>> function_body) : m_function_name(function_name), m_parameters(std::move(parameters)), m_return_type(return_type), m_function_body(std::move(function_body)) {}

    inline const std::string& get_function_name() { return m_function_name; }

    llvm::Function* codegen(cCodeGenerator* code_generator);

private:
    std::string m_function_name;
    std::vector<std::unique_ptr<FunctionParameterAST>> m_parameters;
    std::string m_return_type;
    std::vector<std::unique_ptr<ExprAST>> m_function_body;
};


// @Check: Looks a lot like FunctionParameterAST
// @Check: Does it need to be an ExprAST
class VariableDeclarationExprAST: public ExprAST {
public:
    VariableDeclarationExprAST(const std::string& variable_name, const std::string& variable_type) : m_variable_name(variable_name), m_variable_type(variable_type), m_expression(nullptr) {}

    VariableDeclarationExprAST(const std::string& variable_name, const std::string& variable_type, std::unique_ptr<ExprAST> expression) : m_variable_name(variable_name), m_variable_type(variable_type), m_expression(std::move(expression)) {}

    inline const std::string& get_variable_name() { return m_variable_name; }
    inline const std::string& get_variable_type() { return m_variable_type; }

    llvm::Value* codegen(cCodeGenerator* code_generator) override;

private:
    std::string m_variable_name, m_variable_type;
    std::unique_ptr<ExprAST> m_expression;
};


// Callee([args])
class CallExprAST : public ExprAST {
public:
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args) : 
        m_callee(callee), m_args(std::move(args)) {}

    llvm::Value* codegen(cCodeGenerator* code_generator) override;

private:
    std::string m_callee;
    std::vector<std::unique_ptr<ExprAST>> m_args;
};


// Assignment Expression
// identifier '=' expr
class AssignmentExprAST : public ExprAST {
public:
    AssignmentExprAST(const std::string& variable, std::unique_ptr<ExprAST> rhs) : m_variable(variable), m_rhs(std::move(rhs)) {}

    const std::string& get_variable_name() { return m_variable; }
    llvm::Value* codegen(cCodeGenerator* code_generator) override;

private:
    std::string m_variable;
    std::unique_ptr<ExprAST> m_rhs;
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
    cParser(std::vector<sToken> tokens) : m_tokens(std::move(tokens)), m_current_index(0) {
        m_code_generator = new cCodeGenerator();
    }

    sToken get_next_token();
    const sToken& peek_next_token();

    std::unique_ptr<ExprAST> parse_number_expr();
    std::unique_ptr<ExprAST> parse_paren_expr();
    std::unique_ptr<ExprAST> parse_expression();
    std::unique_ptr<ExprAST> parse_identifier_expr();
    std::unique_ptr<ExprAST> parse_primary();
    std::unique_ptr<ReturnExprAST> parse_return_expr();

    std::unique_ptr<ExprAST> parse_binop_expression(int expr_prec, std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<FunctionParameterAST> parse_function_parameter();
    std::unique_ptr<FunctionDefinitionAST> parse_function_definition();

    std::unique_ptr<VariableDeclarationExprAST> parse_variable_declaration();

    int get_binop_precedence(char op);

    void emit_object_code(std::string file_name);

    void parse();
    cCodeGenerator* m_code_generator;

    ~cParser() = default;

private:
    std::vector<sToken> m_tokens;
    sToken m_current_token;
    int m_current_index;

    std::string m_target_triple;
    llvm::TargetMachine* m_target_machine;

};



