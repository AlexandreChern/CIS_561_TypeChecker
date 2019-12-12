// CodegenContext.cxx
#include <ostream>
#include <map>
#include "CodegenContext.h"
#include "semantics.cxx"
#include "semantics.h"
#include "ASTNode.h"

using namespace std;

void GenContext::emit(string s) { 
    object_code << s  << endl; 
    cout << s << endl;
}

/* Getting the name of a "register" (really a local variable in C)
    * has the side effect of emitting a declaration for the variable.
    */
string GenContext::alloc_reg(string type) {
    int reg_num = next_reg_num++;
    string reg_name = "tmp__" + to_string(reg_num);
    object_code << "obj_" << type << " " << reg_name << ";" << endl;
    return reg_name;
}

void GenContext::free_reg(string reg) {
    this->emit(string("// free register ") + reg);
}

/* Get internal name for a calculator variable.
    * Possible side effect of generating a declaration if
    * the variable has not been mentioned before.  (Later,
    * we should buffer up the program to avoid this.)
    */    

string GenContext::get_type(AST::ASTNode& node) {
    AST_Type_Node class_node = stc->AST_hierarchy[class_name];
    class_and_methods classandmethods;
    map<string, string>* vars;
    if (method_name == "constructor" || method_name == class_name) {
        classandmethods = class_node.construct;
        vars = classandmethods.vars;
    }
    else{ 
        classandmethods = class_node.methods[method_name];
        vars = classandmethods.vars;
    }
    class_and_method *info = new class_and_method(class_name, method_name);
    string type = node.type_inference(stc, vars, info);
    return type;
}

string GenContext::get_formal_argtypes(string method_name) {
    AST_Type_Node class_node = stc->AST_hierarchy[class_name];
    class_and_methods method;
    if (method_name == "constructor" || method_name == class_name) {
        method = class_node.construct;
    }
    else {
        method = class_node.methods[method_name];
    }
    std::string formal_argtypes = "";
    for (std::string s: method.formal_arg_types) {
        formal_argtypes = formal_argtypes + "obj_" + s + ", ";
    }
    return formal_argtypes;
}

string GenContext::get_local_var(string &ident) {
    if (local_vars.count(ident) == 0) {
        std::string local_var = string("var_") + ident;
        local_vars[ident] = local_var;
        AST_Type_Node class_node = stc->AST_hierarchy[class_name];
        class_and_methods classandmethods;
        map<string, string>* vars;
        if (method_name == "constructor" || method_name == class_name) {
            classandmethods = class_node.construct;
            vars = classandmethods.vars;
        }
        else{ 
            classandmethods = class_node.methods[method_name];
            vars = classandmethods.vars;
        }
        std::string var_type = (*vars)[ident];
        this->emit(string("obj_") + var_type + " " + local_var + ";");
        return local_var;
    }
    return local_vars[ident];
}

string GenContext::new_branch_label(const char* prefix) {
    return std::string(prefix) + "_" + to_string(++next_label_num);
}

void GenContext::emit_instance_vars() {
    AST_Type_Node class_node = stc->AST_hierarchy[class_name];
    map<std::string, std::string> instance_vars = class_node.instance_vars;
    for (map<string, string>::iterator iter = instance_vars.begin(); iter != instance_vars.end(); iter++) {
        emit("obj_" + iter->second + " " + iter->first + ";");
    }
}



void GenContext::method_signature() {
    AST_Type_Node class_node = stc->AST_hierarchy[class_name];
    for (std::string method_name: class_node.method_list) {
        class_and_methods classandmethods = class_node.methods[method_name];
        emit("obj_" + classandmethods.return_type + " (*" + method_name + ") (" + get_formal_argtypes(method_name) + ");");
    }
}

void GenContext::emit_class_struct() {
    emit("struct  class_" + class_name + " " + class_name + "_struct = {");
    AST_Type_Node class_node = stc->AST_hierarchy[class_name];
    string class_struct = "";
    class_struct += "new_class" + class_name + ",\n";
    for (string method: class_node.method_list) {
        class_struct = class_struct  + class_node.methods[method].inheritence + "_method_" + method + ",\n";
    }
    emit(class_struct);                
    emit("};\n");
}

