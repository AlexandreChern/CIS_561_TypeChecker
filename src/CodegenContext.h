// CodegenContext.h
#ifndef AST_CODEGENCONTEXT_H
#define AST_CODEGENCONTEXT_H

#include <ostream>
#include <map>

using namespace std;

class semantics;
namespace AST {class ASTNode; }

class GenContext {
    int next_reg_num = 0;
    int next_label_num = 0;
    map<string, string> local_vars;
    ostream &object_code;
public:
    string class_name;
    string method_name;
    semantics* stc;

    explicit GenContext(ostream &out, semantics* ss, string clsname, string methname) : 
        object_code{out}, stc{ss}, class_name{clsname}, method_name{methname} {};

    void emit(string s);

    string alloc_reg(string type);

    void free_reg(string reg);

    string get_local_var(string &ident);
    string get_type(AST::ASTNode& node);
    string new_branch_label(const char* prefix);
    void emit_instance_vars();
    string get_formal_argtypes(string methodname);
    void emit_method_signature();
    void emit_class_struct();
};

#endif //AST_CODEGENCONTEXT_H