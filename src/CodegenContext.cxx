#include <ostream>
#include <map>
// #include "CodegenContext.h"
#include "CodegenContext.h"
#include "semantics.h"
#include "semantics.cxx"
#include "ASTNode.h"


using namespace std;


void GenContext::emit(std::string s) {
    object_code << s << endl;
}

std::string GenContext::alloc_reg(std::string type) {
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
    class_and_methods classandmethods;
    std::map<std::string, std::string>* vars;
    if (method_name == "constructor" || method_name == class_name){
        classandmethods = class_node.constructor;
        vars = classandmethods.vars;
    }
    else{
        if (method_name == "PGM"){
            vars = &class_node.instance_vars;
        }
        else{
            class_and_method = class_node.methods[method_name];
            vars = classandmethods.vars;
        }
    }
    class_and_method *info = new class_and_method(class_name, method_name);
    std::string var_type = node.type_inference(stc, vars, info);
    return var_type;
}

std::string GenContext::get_local_var(std::string &ident){
    if (local_vars.count(ident) == 0){
        std::string internal = std::string("var_") + ident;
        local_vars[ident] = internal;

        type_node class_node = stc->hierarchy[class_name];
        class_and_methods classandmethods;
        map<std::string,std::string>* vars;
        if(method_name == "constructor" || method_name == class_name){
            classandmethods = class_node.constroctor;
            vars = classandmethods.vars;
        }
        else{
            if (method_name == "PGM"){
                vars = &class_node.instance_vars;
            }
            else{
                classandmethods = classnode.methods[method_name];
                vars = classandmethods.vars;
            }
        }
        std::string var_type = (*vars)[ident];
        this->emit(std::string("obj_") + type + " " + internal + ";" );
    }
    return local_vars[ident];
}

std::string GenContext::new_branch_label(const char* prefix){
    return string(prefix) + "_" + to_string(next_label_num++);
}

void GenContext::emit_instance_vars(){
    type_node class_node = stc->hierarchy[class_name];
    map<std::string, std::string>* instance_vars = class_node.instance_vars;
    for (map<std::string, std::string>::iterator iter= instance_vars.begin(); iter!=instance_vars.end(); iter++){
        emit("obj_" + iter->second + " " + iter->first + ";");
    }
}


void GenContext::get_formal_argtypes(std::string method_name){
    type_node class_node = stc->hierarchy[class_name];
    class_and_methods classandmethods;
    if (method_name == "constructor" || method_name == class_name){
        method = class_node.constructor;
    }
    else{
        method = class_node.methods[method_name];
    }

    std::string formals="";
    for (std::string s: classandmethods.formal_arg_types){
        formals = formals + "obj_" + s + ", ";
    }
    int string_length = formals.length();
    if (string_length > 2){
        formals = formals.erase(string_length - 2, 2);
    }
    return formals;
}

void GenContext::emit_class_struct(){
    emit("struct class_" + class_name + "_struct the_class_" + class_name + "_struct = {");
    type_node class_node = stc->hierarchy[class_name];
    std::string class_struct = "new_" + class_name;
    for (std::string method: class_node.method_list){
        class_struct += ",\n";
        class_and_methods classandmethods = class_node.methods[method];
        class_struct += classandmethods.inherited_from + "_method_" + method;
    }
    emit(class_struct);
    emit("};\n");
}