#include "ASTNode.h"
#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include "semantics.h"

using namespace std;

class class_and_methods {
    public:
        string method_name;
        string return_type;
        vector<std::string> formal_arg_types;
        map<string, string>* vars;
        string inheritence;

        class_and_methods () {
            formal_arg_types = vector<std::string>();
            vars = new map<string, string>();
        }

        class_and_methods(string name) {
            method_name = name;
            formal_arg_types = vector<std::string>();
            vars = new map<string, string>();
        }

 
};

class AST_Type_Node {
    public:
        std::string type;
        std::string parent_type;
        map<std::string, std::string> instance_vars;
        map<std::string, class_and_methods> methods;
        class_and_methods construct;
        int resolved;
        vector<std::string> method_list;

        AST_Type_Node() {
            instance_vars = map<std::string, std::string>();
            methods = map<std::string, class_and_methods>();
            construct = class_and_methods();
            resolved = 0;
            method_list = vector<std::string>();
        }

        AST_Type_Node(std::string name) {
            type = name;
            instance_vars = map<std::string, std::string>();
            methods = map<std::string, class_and_methods>();
            construct = class_and_methods(name);
            construct.return_type = name;
            resolved = 0;
            method_list = vector<std::string>();
        }
};

class AST_Edge {
    public:
        vector<std::string> children;
        int visited;

        AST_Edge() {
            children = vector<std::string>();
            visited = 0;
        }
};

class semantics {
    public:
        AST::ASTNode* AST_root_init;
        int error_detected;
        int modified;
        map<string, AST_Type_Node> AST_hierarchy;
        map<string, AST_Edge*> edges;
        vector<std::string> classes_resolved;

        semantics(AST::ASTNode* root) { 
            AST_root_init = root;
            error_detected = 0;
            modified = 1;
            AST_hierarchy = map<string, AST_Type_Node>();
            edges = map<string, AST_Edge*>();
            classes_resolved = vector<std::string>();
        }

        void topological_sort() {
            classes_resolved.push_back("Obj");
            for(map<string,AST_Type_Node>::iterator iter = AST_hierarchy.begin(); iter != AST_hierarchy.end(); iter++) {
                AST_Type_Node *node = &AST_hierarchy[iter->first]; 
                topological_sort_utils(node);
            }
        }
        void topological_sort_utils(AST_Type_Node* node) {
            if (!node->resolved) {
                std::string parent_type = node->parent_type;
                AST_Type_Node* parent_node = &AST_hierarchy[parent_type];
                topological_sort_utils(parent_node);
                classes_resolved.push_back(node->type);
                node->resolved = 1;
            }
        }

        int pop_AST_Edges() {
            for(map<string,AST_Type_Node>::iterator iter = AST_hierarchy.begin(); iter != AST_hierarchy.end(); iter++) {
                AST_Type_Node node = iter->second;
                edges[node.type] = new AST_Edge();
            }
            for(map<string,AST_Type_Node>::iterator iter = AST_hierarchy.begin(); iter != AST_hierarchy.end(); iter++) {
                AST_Type_Node node = iter->second;
                string parent_type = node.parent_type;
                if (iter->first == "Obj") {
                    continue;
                }
                if (!edges.count(node.parent_type)) {
                    cout << "Error: " << endl;
                    return 0;
                }
                edges[node.parent_type]->children.push_back(node.type);
            }
            return 1;
        }

        int is_AST_cyclic(string root) {
            AST_Edge* root_edge = edges[root];
            for (string child: root_edge->children) {
                AST_Edge* child_edge = edges[child];
                if (child_edge->visited || is_AST_cyclic(child)) { 
                    return 1;
                }
            }
            return 0;
        }

        void pop_AST_hierarchy() { 
            AST::Program *root = (AST::Program*) AST_root_init;
            AST::Classes classes_node = root->classes_;
            vector<AST::Class *> classes = classes_node.elements_;
            for (AST::Class *element: classes) {
                string class_name = element->name_.text_;
                AST_Type_Node node;
                if (AST_hierarchy.count(class_name)) { 
                    node = AST_hierarchy[class_name]; 
                }
                else {
                    node = AST_Type_Node(class_name);
                }
                node.parent_type = element->super_.text_; 

                AST::Method *constructor = (AST::Method *) &(element->constructor_);
                AST::Ident *returned_type = (AST::Ident*) &(constructor->returns_);
                node.construct.return_type = returned_type->text_; 
                AST::Formals* formals_node = (AST::Formals*) &(constructor->formals_);
                vector<AST::Formal *> formals = formals_node->elements_;
                for (AST::Formal *formal: formals) {
                        AST::Ident *type = (AST::Ident *) &(formal->type_);
                        node.construct.formal_arg_types.push_back(type->text_); 
                    }

                AST::Block* block_node = (AST::Block*) &(constructor->statements_);
                vector<AST::Statement *> *statements = (vector<AST::Statement *> *) &block_node->elements_;
                vector<AST::Statement *> stmts = *statements;

                for (AST::Statement *stmt: stmts) {
                    stmt->get_vars(&node.instance_vars);
                } 

                vector<AST::Method *> methods = (element->methods_).elements_;
                for (AST::Method *method: methods) {
                    AST::Ident* method_name = (AST::Ident*) &(method->name_);
                    AST::Ident* return_type = (AST::Ident*) &(method->returns_);
                    class_and_methods newmethod(method_name->text_);
                    newmethod.return_type = return_type->text_;
                    AST::Formals* formals_node = (AST::Formals*) &(method->formals_);
                    vector<AST::Formal *> formals = formals_node->elements_;
                    for (AST::Formal *formal: formals) {
                        AST::Ident *type = (AST::Ident *) &(formal->type_);
                        newmethod.formal_arg_types.push_back(type->text_);
                    }
                    newmethod.inheritence = class_name;
                    node.methods[newmethod.method_name] = newmethod;
                }
                AST_hierarchy[class_name] = node; 

            } 
        } 

        int method_found(vector<std::string>* method_list, string target_method) {
            for (string method: *method_list) {
                if (method == target_method) { 
                    return 1; 
                }
            }
            return 0;
        }
        
        void methods_inheritance() {
            for (string class_name: classes_resolved) {
                if (class_name == "Obj" || class_name == "PGM") {continue;}
                AST_Type_Node *class_node = &AST_hierarchy[class_name];
                map<string, class_and_methods> *class_methods = &class_node->methods;

                std::string parent_type = class_node->parent_type;
                AST_Type_Node *parent_node = &AST_hierarchy[parent_type];
                for (std::string method_name: parent_node->method_list) {
                    class_node->method_list.push_back(method_name);
                }
                for (map<string, class_and_methods>::iterator iter = class_methods->begin(); iter != class_methods->end(); iter++) {
                    if (!method_found(&class_node->method_list, iter->first)) { // iter->first found in class_node->method_list
                        class_node->method_list.push_back(iter->first); // Add iter->first to the method_list
                    }
                }
                for (string s: class_node->method_list) {
                    if (!class_methods->count(s)) {
                        class_and_methods parent_method = parent_node->methods[s];
                        (*class_methods)[s] = class_and_methods(parent_method);
                    }
                }
            }
        }

        int is_subtype(string type_1, string type_2) { // type_1 subtype, type_2 supertype
            set<std::string> path = set<std::string>();
            std::string type = type_1;
            while (true) {
                path.insert(type);
                if (type == "Obj") { break; }
                type = AST_hierarchy[type].parent_type; // recursively going up though the AST tree
                if (path.count(type_2)){
                    cout << "type_1 is a subtype of " << type_2;
                    return 1; 
                }
            }
            if (!AST_hierarchy.count(type)) { 
                cout << "ERROR: Type Error" << endl;
                return 0;
            }
        }

        std::string Type_LCA(string type_1, string type_2) {

            if (type_1== "TypeError" || type_2 == "TypeError") { return "TypeError";}
            if (type_1 == type_2){
                return type_1;
            }

            else{
                std::string type = type_1;
                set<std::string> type_path = set<std::string>();
                while (true) {
                    type_path.insert(type);
                    if (type == "Obj") { 
                        cout << "Error: Obj detected " << endl;
                        break; 
                    }
                    type = AST_hierarchy[type].parent_type;
                     if (type_path.count(type_2)) {
                        return type_2;
                    }
                }  
                type = AST_hierarchy[type].parent_type;
                return type;
            }
        }

        vector<std::string> split(string Target_String, char delimeter)
        {
            stringstream ss(Target_String);
            string item;
            vector<std::string> Splitted_Strings;
            while (getline(ss, item, delimeter))
            {
            Splitted_Strings.push_back(item);
            }
            return Splitted_Strings;
        }

        map<std::string, AST_Type_Node>* type_check() {
            AST::Program *root = (AST::Program*) AST_root_init;
            while (modified) {
                modified = 0;
                root->type_inference(this, nullptr, nullptr); 
            }
            return &this->AST_hierarchy;
        }

        void pop_AST_Builtins() {
            AST_Type_Node program("PGM");
            program.parent_type = "Obj";
            AST_hierarchy["PGM"] = program;

            AST_Type_Node obj("Obj");
            obj.parent_type = "TYPE_ERROR";

            class_and_methods obj_print("PRINT");
            obj_print.return_type = "Obj";
            obj_print.formal_arg_types.push_back("Obj");
            obj_print.inheritence = "Obj";
            obj.methods["PRINT"] = obj_print;

            class_and_methods obj_string("STRING");
            obj_string.return_type = "String";
            obj_string.formal_arg_types.push_back("Obj");
            obj_string.inheritence = "Obj";
            obj.methods["STRING"] = obj_string;

            class_and_methods rel_equals("EQUALS");
            rel_equals.return_type = "Boolean";
            rel_equals.formal_arg_types.push_back("Obj");
            rel_equals.formal_arg_types.push_back("Obj");
            rel_equals.inheritence = "Obj";

            obj.methods["EQUALS"] = rel_equals;
            obj.resolved = 1;
            obj.method_list.push_back("STRING");
            obj.method_list.push_back("PRINT");
            obj.method_list.push_back("EQUALS");
            AST_hierarchy["Obj"] = obj;

            AST_Type_Node integer("Int");
            integer.parent_type = "Obj";
            AST_hierarchy["Int"] = integer;


            class_and_methods bi_plus("PLUS");
            bi_plus.return_type = "Int";
            bi_plus.inheritence = "Int";
            bi_plus.formal_arg_types.push_back("Int");
            AST_hierarchy["Int"].methods["PLUS"] = bi_plus;

            class_and_methods bi_minus("MINUS");
            bi_minus.return_type = "Int";
            bi_minus.inheritence = "Int";
            bi_minus.formal_arg_types.push_back("Int");
            AST_hierarchy["Int"].methods["MINUS"] = bi_minus;

            class_and_methods bi_times("TIMES");
            bi_times.return_type = "Int";
            bi_times.inheritence = "Int";
            bi_times.formal_arg_types.push_back("Int");
            AST_hierarchy["Int"].methods["TIMES"] = bi_times;

            class_and_methods bi_divide("DIVIDE");
            bi_divide.return_type = "Int";
            bi_divide.inheritence = "Int";
            bi_divide.formal_arg_types.push_back("Int");
            AST_hierarchy["Int"].methods["DIVIDE"] = bi_divide;
 
            class_and_methods rel_greater(">");
            rel_greater.return_type = "Boolean";
            rel_greater.inheritence = "Int";
            rel_greater.formal_arg_types.push_back("Int");
            AST_hierarchy["Int"].methods[">"] = rel_greater;

            class_and_methods rel_smaller("<");
            rel_smaller.return_type = "Boolean";
            rel_smaller.inheritence = "Int";
            rel_smaller.formal_arg_types.push_back("Int");
            AST_hierarchy["Int"].methods["<"] = rel_smaller;

            AST_Type_Node str("String");
            str.parent_type = "Obj";
            AST_hierarchy["String"] = str;

            AST_Type_Node boolean("Boolean");
            boolean.parent_type = "Obj";
            AST_hierarchy["Boolean"] = boolean;

            AST_Type_Node nothing("Nothing");
            nothing.parent_type = "Obj";
        }

        void* AST_check() { 
            pop_AST_hierarchy();
            if (!pop_AST_Edges() || is_AST_cyclic("obj")) {
                return nullptr;
                cout << "AST FAILED TO BUILD" << endl;
            }
            else {
                cout << "ACYLIC AST BUILT SUCCESSFULLY" << endl;
            }
            topological_sort();
            methods_inheritance();

            AST::Program *root = (AST::Program*) AST_root_init;
            set<std::string> *vars = new set<std::string>;
            if (root->init_check(vars, this)) { 
                cout << "INITIALIZATION ERRORS" << endl;
                return nullptr;
            }
            type_check();
            return &AST_hierarchy;
        }
}; 

