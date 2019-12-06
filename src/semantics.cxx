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
                AST_Type_Node *node = &AST_hierarchy[iter->first]; // get node directly from map
                topological_sort_utils(node);
            }
        }
        void topological_sort_utils(AST_Type_Node* node) {
            if (!node->resolved) {
                string parent_type = node->parent_type;
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

        // void print_AST_hierarchy() {
        //     cout << "---------------PRINT CLASS AST_hierarchy----------------" << endl;
        //     for(map<string,AST_Type_Node>::iterator iter = AST_hierarchy.begin(); iter != AST_hierarchy.end(); iter++) {
        //         AST_Type_Node node = iter->second;
        //         // node.print_AST_Node();
        //         cout << "--------------------------------" << endl;
        //     }
        // }

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

                // methods 
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
                if (method == target_method) { return 1; }
            }
            return 0;
        }
        void inherit_methods() {
            for (string class_name: classes_resolved) {
                if (class_name == "Obj" || class_name == "PGM") {continue;}
                AST_Type_Node *class_node = &AST_hierarchy[class_name];
                map<string, class_and_methods> *class_methods = &class_node->methods;
                string parent_type = class_node->parent_type;
                AST_Type_Node *parent_node = &AST_hierarchy[parent_type];
                for (string method: parent_node->method_list) {
                    class_node->method_list.push_back(method);
                }
                for (map<string, class_and_methods>::iterator iter = class_methods->begin(); iter != class_methods->end(); iter++) {
                    if (!method_found(&class_node->method_list, iter->first)) {
                        class_node->method_list.push_back(iter->first);
                    }
                }
                for (string s: class_node->method_list) {
                    if (!class_methods->count(s)) {
                        class_and_methods parent_method = parent_node->methods[s];
                        class_and_methods classandmethods = class_and_methods(parent_method);
                        (*class_methods)[s] = classandmethods;
                    }
                }
            }
        }

        int is_subtype(string sub, string super) {
            set<std::string> path = set<std::string>();
            string type = sub;
            if (!AST_hierarchy.count(type)) { // subtype not in class AST_hierarchy
                cout << "ERROR: Type Error" << endl;
                return 0;
            }
            while (true) {
                path.insert(type);
                if (type == "Obj") { break; }
                type = AST_hierarchy[type].parent_type;
            }
            if (path.count(super)) {
                return 1;
            }
            return 0;
        }

        string Type_LCA(string type_1, string type_2) {

            if (type_1 == "Error" || type_1== "TypeError") { return type_2;}
            if (type_2 == "Error" || type_1 == "TypeError") { return type_1;}

            if (!AST_hierarchy.count(type_1)) { 
                return type_1; 
            }
            if (!AST_hierarchy.count(type_2)) { 
                return type_2; 
            }
            set<std::string> type_1_path = set<std::string>();
            string type = type_1;
            while (1) {
                type_1_path.insert(type);
                if (type == "Obj") { break; }
                type = AST_hierarchy[type].parent_type;
            }
            type = type_2;
            while (1) {
                if (type_1_path.count(type)) {
                    return type;
                }
                type = AST_hierarchy[type].parent_type;
            }
        }

        int compare_maps(map<string, string> map_1, map<string, string>map_2) {
            // same types, proceed to compare maps here
            if(map_1.size() != map_2.size())
                return 0;  // differing sizes, they are not the same
            typename map<std::string,std::string>::const_iterator i, j;
            for(i = map_1.begin(), j = map_2.begin(); i != map_1.end(); ++i, ++j)
            {
                if(*i != *j)
                return 0;
            }
            return 1;
        }

        vector<std::string> split(string strToSplit, char delimeter)
        {
            stringstream ss(strToSplit);
            string item;
            vector<std::string> splittedStrings;
            while (getline(ss, item, delimeter))
            {
            splittedStrings.push_back(item);
            }
            return splittedStrings;
        }

        map<string, AST_Type_Node>* type_check() {
            AST::Program *root = (AST::Program*) AST_root_init;
            while (modified) {
                modified = 0;
                root->type_inference(this, nullptr, nullptr); 
            }
            return &this->AST_hierarchy;
        } // end typeCheck

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


            class_and_methods rel_greater(">");
            rel_greater.return_type = "Boolean";
            rel_greater.inheritence = "Int";
            rel_greater.formal_arg_types.push_back("Int");
            AST_hierarchy["Int"].methods[">"] = rel_greater;

            AST_Type_Node str("String");
            str.parent_type = "Obj";
            AST_hierarchy["String"] = str;

            AST_Type_Node boolean("Boolean");
            boolean.parent_type = "Obj";
            AST_hierarchy["Boolean"] = boolean;

            AST_Type_Node nothing("Nothing");
            nothing.parent_type = "Obj";
        }

        void* AST_check() { // top-level
            pop_AST_hierarchy();
            if (!pop_AST_Edges() || is_AST_cyclic("obj")) {
                return nullptr;
                cout << "AST FAILED TO BUILD" << endl;
            }

            else {
                cout << "ACYLIC AST BUILT SUCCESSFULLY" << endl;
            }
            topological_sort();
            inherit_methods();
            // print_AST_hierarchy();
            AST::Program *root = (AST::Program*) AST_root_init;
            set<std::string> *vars = new set<std::string>;
            if (root->init_check(vars, this)) { 
                cout << "INITIALIZATION ERRORS" << endl;
                return nullptr;
            }
            type_check();
            // print_AST_hierarchy();
            return &AST_hierarchy;
        }
}; 

