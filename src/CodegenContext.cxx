#include <ostream>
#include <map>
#include <CodegenGenContext.h>
#include "semantics.h"
#include "semantics.cxx"
#include "ASTNode.h"


using namespace std;


void GenContext::emit(std::string s) {object_code << " " s << endl;}

std::string GenContext::alloc_reg(string type) {
    int reg_num = next_reg_num++;
    string reg_name = "tmp__" + std::to_string(reg_num);
    object_code << "obj_" << type << " " << reg_name << ";" << endl;
    return reg_name;
}

void GenContext::free_reg(std::string reg){
    this->emit(std::string("// free register") + reg);
}


std::string GenContext::get_type(AST::ASTNode& node){
    type_node class_node = stc->hierarchy[class_name];
    class_and_methods class_and_method;
    std::map<std::string, std::string>* vars;
    if (method_name == "constructor"){
        class_and_method = class_node.construct;
        vars = class_and_method.vars;
    }
    else{
        if (method_name == "__pgm__"){
            vars = &class_node.instance_vars;
        }
        else{
            class_and_method = class_node.methods[method_name]
        }
    }
    std::string var_type = node.get_type(vtable, stc, class_name)
}