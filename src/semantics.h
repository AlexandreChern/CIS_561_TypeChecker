#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <typeinfo>
#include <map>




using namespace std;

class class_and_method {
    public:
        std::string class_name;
        std::string method_name;

        class_and_method(std::string class_name, std::string method) {
            this->class_name = class_name;
            this->method_name = method;
        }

};





#endif // SEMANTICS_H