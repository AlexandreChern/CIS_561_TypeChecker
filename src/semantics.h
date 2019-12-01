
#ifndef ASTNODE_H
#define ASTNODE_H

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <typeinfo>
#include <map>

using namespace std;


using namespace AST{
    int Program::init_check(){
        // to be implemented
    }

    int Program::type_inference(){
        //
    }

    
}

class semantics;

class class_and_methods {
    public:
        std::string class_name;
        std::string class_method;

        class_and_methods(std::string class_name, std::string class_method){
            this->class_name = class_name;
            this->class_method = class_method;
        }
    void print_method(){
        std::cout << "\t class_name: " << this->class_name << std::endl;
        std::cout << "\t class_method: " << this->class_method << std::endl;
    }
}