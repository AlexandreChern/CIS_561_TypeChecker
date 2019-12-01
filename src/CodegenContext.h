// CodegenContext.h
#ifndef AST_CODEGENCONTEXT_H
#define AST_CODEGENCONTEXT_H

#include <ostream>
#include <map>

class semantics;
namespace AST {class ASTNode;}


class Context{
    int nex_reg_num = 0;
    int next_label_num = 0;

    std::map<std::string, std::string> local_vars;
    std::ostream & object_code;

public:
    std::string class_name;
    std::string method_name;
    semantics* stc;

    explicit Context(std::ostream &out, semantics* stx, std::string classname, std::string methodname):
        object_code{out}, stc{stx}, class_name(classname), method_name(methodname) {};

    void emit(std::string s);

    std::string alloc_reg(std::string type);

    void free_reg(std::string reg);

    std::string get_local_var(std::string &ident){
        if (local_vars.count(ident) == 0) {
            std::string internal = std::string("calc_var_") + ident;
            local_vars[ident] = internal;
            // We'll need a declaration in the generated code
            this->emit(std::string("int ") + internal + "; // Source variable " + ident);
            return internal;
        }
        return local_vars[ident];
    };
    std::string get_type(AST::ASTNode& node);
    std::string new_branch_label(const char* prefix){
        return std::string(prefix) + "_" + std::to_string(++next_label_num);
    }
    void emit_instance_vars();
    std::string get_formal_argtypes(std::string method_name){
        return std::to_string(++next_label_num) + "." + std::to_string(method_name); 
    };
    void emit_method_sigs();
}

#endif // AST_CODEGENCONTEXT_H