#include "ASTNode.h"
#include "semantics.h"
#include <iostream>
#include <set>
#include <map>
#include <vector>


using namespace std;

class method_table {
    public:
        string method_name;
        string return_type;

        vector<string> formal_arg_types;
        map<string, string>* vars;

        method_table(string name){
            method_name = name;
            formal_arg_types = vector<string>();
            vars = new map<string, string> ()
        }

    void print_method() {
        cout << "\t " << "method_name: " << method_name << endl;
        cout << "\t " << "return_type: " << return_type << endl;
        cout << "\t " << "formal_arg_types: " << formal_arg_types << endl;

        for (string formal_arg: formal_arg_types){
            cout << formal_arg << ", ";
        }

        cout << endl;

        cout << "\t" << "variables: " << endl;
        for (map<string,string>::iterator iter = vars->begin(); iter != vars->end();iter++) {
            cout << "\t " << iter->first << ": " << iter->second << endl;
        }
        cout << endl;
    }
    
};

class type_node {
    public:
        string var_type;
        string parent;

        map<string, string> instance_vars;
        map<string, method_table> methods;

        method_table construct;

    type_node(string var_name){
        var_type = var_name;
        instance_vars = map<string, string>();
        methods = map<string, method_table>();
        construct = method_table(name);
    }

    void print_type(){
        cout << "\t" << "var_type: " << var_type << endl;
        cout << "\t" << "perent: " << parent << endl;
        cout << "\t" << "instance_vars: " << instance_vars;

        
    }
}



class semantics{
    public:
        AST::ASTNode*  ast_initial;
        void semantic_error{
            cout << "\t semantic error" << endl;
        }
        int modified = 1;

        map<string, type_node> hierarchy;
        map<string, Edge*> edges;

        semantics(AST::ASTNode* root){
            ast_initial = root;
            error_count = 0;

            hierarchy = map<string, type_node>();
            edges = map<string, Edge*>();
        }

        int Edges(){

        }

        int is_cyclic(string root){
            Edge* rootedge = edges[root];

        }


        void check_AST(){
            if (is_cyclic("Obj")){
                cout << "Cyclid AST Detected" << endl;
                return nullptr;
            }
        }

};


