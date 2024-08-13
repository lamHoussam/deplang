# pragma once

#include <algorithm>
#include <locale>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
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


// @TODO: Change Macro
# define DEPLANG_PARSER_ERROR(err) std::cerr << "::[Parser]::Error: " << err << std::endl


struct sTypedValue;


class cCodeGenerator {
public:
    cCodeGenerator();

    std::unique_ptr<llvm::LLVMContext> m_Context;
    std::unique_ptr<llvm::IRBuilder<>> m_Builder;

    std::unique_ptr<llvm::Module> m_Module;
    std::map<std::string, sTypedValue*> m_NamedValues;
    std::map<std::string, llvm::Type*> m_NamedTypes;

    void delete_named_values();
    ~cCodeGenerator() = default;

    // ~cCodeGenerator() = default;
private:
};

inline llvm::Type* get_llvm_type(const std::string& type, std::shared_ptr<cCodeGenerator> code_generator);

// @TODO: Implement
sTypedValue* build_ir_operation(sTypedValue* l, sTypedValue* r, std::string op, std::shared_ptr<cCodeGenerator> code_generator);


class ExprAST {
public:
    virtual ~ExprAST() = default;
    virtual sTypedValue* codegen(std::shared_ptr<cCodeGenerator> code_generator) = 0;
    virtual void print() = 0;
};


// Value

class LiteralIntExprAST : public ExprAST {
public:
    LiteralIntExprAST(const std::string& value) : m_value(std::stoi(value)) {}
    inline const int get_value() { return m_value; }

    sTypedValue* codegen(std::shared_ptr<cCodeGenerator> code_generator) override;

    void print() override;
private:
    int m_value;
};


class LiteralFloatExprAST : public ExprAST {
public:
    LiteralFloatExprAST(const std::string& value) : m_value(std::stof(value)) {}
    inline const float get_value() { return m_value; }
    std::unique_ptr<ExprAST> parse_type_expr();

    sTypedValue* codegen(std::shared_ptr<cCodeGenerator> code_generator) override;
    void print() override;
private:
    float m_value;
};

class LiteralBoolExprAST: public ExprAST {
public:
    LiteralBoolExprAST(bool value) : m_value(value) {}
    inline const bool get_value() { return m_value; }

    sTypedValue* codegen(std::shared_ptr<cCodeGenerator> code_generator) override;
    void print() override;
private:
    bool m_value;
};

// Name
class VariableExprAST : public ExprAST {
public:
    VariableExprAST(const std::string& name);

    const std::string& get_name();
    sTypedValue* codegen(std::shared_ptr<cCodeGenerator> code_generator) override;
    void print() override;
private:
    std::string m_name;
};

// Type
class TypeExrAST : public ExprAST {
public:
    TypeExrAST(const std::string& name, std::unique_ptr<TypeExrAST> lhs, std::unique_ptr<TypeExrAST> rhs);
    TypeExrAST(const std::string& name);

    // @TODO: Change to real type
    const std::string& get_primitive_type();
    sTypedValue* codegen(std::shared_ptr<cCodeGenerator> code_generator) override;

    llvm::Type* register_type(std::shared_ptr<cCodeGenerator> code_generator);
    void print() override;

    bool type_check(const TypeExrAST* other_type_expr);
private:
    std::string m_prim_type;
    std::unique_ptr<TypeExrAST> m_left, m_right;
};

// Expr Op Expr
class BinaryExprAST : public ExprAST {
public:
    BinaryExprAST(std::string op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs);
    sTypedValue* codegen(std::shared_ptr<cCodeGenerator> code_generator) override;
    void print() override;
private:
    std::string m_op;
    std::unique_ptr<ExprAST> m_lhs, m_rhs;
};

class ReturnExprAST : public ExprAST {
public:
    ReturnExprAST(std::unique_ptr<ExprAST> expression);
    sTypedValue* codegen(std::shared_ptr<cCodeGenerator> code_generator) override;
    void print() override;
private:
    std::unique_ptr<ExprAST> m_expression;
};

// Function parameter expression
// function_parameter := identifier ":" identifier
class FunctionParameterAST {
public:
    FunctionParameterAST(const std::string& param_name, std::unique_ptr<TypeExrAST> param_type);
    inline const std::string& get_param_name();
    
    // @TODO: Change type
    inline const std::string& get_primitive_type();
    std::unique_ptr<TypeExrAST> m_type_expr;

private:
    std::string m_param_name;
};

// Function definition
// func identifier "(" function_parameter* ")" "{"
//    expression* ";"
// "}"
class FunctionDefinitionAST {
public: 
    FunctionDefinitionAST(const std::string& function_name, std::vector<std::unique_ptr<FunctionParameterAST>> parameters, std::unique_ptr<TypeExrAST> return_type, std::vector<std::unique_ptr<ExprAST>> function_body);

    inline const std::string& get_function_name();

    llvm::Function* codegen(std::shared_ptr<cCodeGenerator> code_generator);

    void print();
private:
    std::string m_function_name;
    std::vector<std::unique_ptr<FunctionParameterAST>> m_parameters;
    std::unique_ptr<TypeExrAST> m_return_type;
    std::vector<std::unique_ptr<ExprAST>> m_function_body;
};


// @Check: Looks a lot like FunctionParameterAST
// @Check: Does it need to be an ExprAST
class VariableDeclarationExprAST: public ExprAST {
public:
    VariableDeclarationExprAST(const std::string& variable_name, std::unique_ptr<TypeExrAST> variable_type);
    VariableDeclarationExprAST(const std::string& variable_name, std::unique_ptr<TypeExrAST> variable_type, std::unique_ptr<ExprAST> expression);

    inline const std::string& get_variable_name();

    // @TODO: Change type
    inline const std::string& get_primitive_type();

    sTypedValue* codegen(std::shared_ptr<cCodeGenerator> code_generator) override;
    void print() override;

private:
    std::string m_variable_name;
    std::unique_ptr<TypeExrAST> m_variable_type;
    std::unique_ptr<ExprAST> m_expression;
};


// Callee([args])
class CallExprAST : public ExprAST {
public:
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args);
    sTypedValue* codegen(std::shared_ptr<cCodeGenerator> code_generator) override;
    void print() override;

private:
    std::string m_callee;
    std::vector<std::unique_ptr<ExprAST>> m_args;
};


// Assignment Expression
// identifier '=' expr
class AssignmentExprAST : public ExprAST {
public:
    AssignmentExprAST(const std::string& variable, std::unique_ptr<ExprAST> rhs);

    inline const std::string& get_variable_name();
    sTypedValue* codegen(std::shared_ptr<cCodeGenerator> code_generator) override;
    void print() override;

private:
    std::string m_variable;
    std::unique_ptr<ExprAST> m_rhs;
};

// Custom type declaration
// type identifier '=' type_expr
class TypeDeclarationExprAST {
public:
    TypeDeclarationExprAST(const std::string& type_name, std::unique_ptr<TypeExrAST> type_def);

    llvm::Type* codegen(std::shared_ptr<cCodeGenerator> code_generator);
private:
    std::string m_type_name;
    std::unique_ptr<TypeExrAST> m_type_definition;
};


struct sTypedValue {
    llvm::Value* value;
    llvm::Type* type;
    sTypedValue(llvm::Value* value, llvm::Type* type) {
        this->value = value; this->type = type;
    }
    ~sTypedValue() = default;
};




class cParser {
public:
    cParser(std::vector<sToken> tokens);

    sToken get_next_token();
    const sToken& peek_next_token();

    std::unique_ptr<ExprAST> parse_number_expr();
    std::unique_ptr<ExprAST> parse_paren_expr();
    std::unique_ptr<ExprAST> parse_expression();
    std::unique_ptr<ExprAST> parse_identifier_expr();
    std::unique_ptr<ExprAST> parse_primary();
    std::unique_ptr<TypeExrAST> parse_type();
    std::unique_ptr<TypeDeclarationExprAST> parse_type_declaration();

    std::unique_ptr<ReturnExprAST> parse_return_expr();

    std::unique_ptr<ExprAST> parse_binop_expression(int expr_prec, std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<TypeExrAST> parse_type_expression(int expr_prec, std::unique_ptr<TypeExrAST> lhs);

    std::unique_ptr<FunctionParameterAST> parse_function_parameter();
    std::unique_ptr<FunctionDefinitionAST> parse_function_definition();

    std::unique_ptr<VariableDeclarationExprAST> parse_variable_declaration();

    int get_binop_precedence(std::string op);
    int get_type_operator_precedence(std::string op);

    void emit_object_code(std::string file_name);

    void parse();
    std::shared_ptr<cCodeGenerator> m_code_generator;
    

    ~cParser() = default;

private:
    std::vector<sToken> m_tokens;
    sToken m_current_token;
    int m_current_index;

    std::string m_target_triple;
};



