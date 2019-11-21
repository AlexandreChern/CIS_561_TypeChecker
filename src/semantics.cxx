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
        AST::ASTNode*  ast_init_root;
        void semantic_error{
            cout << "\t semantic error" << endl;
        }
        int modified = 1;

        map<string, type_node> hierarchy;
        map<string, Edge*> edges;

        semantics(AST::ASTNode* root){
            ast_init_root = root;
            error_count = 0;

            hierarchy = map<string, type_node>();
            edges = map<string, Edge*>();
        }

        int Edges(){

        }

        int pop_Edges(){
             for(map<string,type_node>::iterator iter = hierarchy.begin(); iter != hierarchy.end(),iter++){
                type_node node = iter->second;
                edges[node.type] = new Edge();

            for(map<string,type_node>::iterator iter = hierarchy.begin(); iter != hierarchy.end(), iter++){
                type_node node = iter->end;
                string parent = node->parent; // node.parent ?
                if (iter->first = "Obj"){
                    continue;
                }

                if (!edges.count(node.parent)){
                    cout << "Error: " << node.parent << "undefined" << endl;
                    return 0;
                }
                edges[node.parent]->children.push_back(node.type);
            }
        }


        int is_cyclic(string root){
            Edge* rootedge = edges[root];

        }

        int pop_Builtins(){
            type_node obj("Obj");
            obj.parent = "EMPTY";
            hierarchy["Obj"] = obj;
        }

        int pop_AST_hierarchy(){
            pop_Builtins();

            AST::Program *root = (AST::Program*) ast_init_root;
            AST::Classes  classes_node = root -> classes_;

            vector<AST::Class *> classes = classes_node.elements_;
            for(AST::Class *elm: classes){
                string class_name = elm -> name_.text_;
                type_node node;

                if(hierarchy.count(class_name)){
                    node = hierarchy[class_name];
                }

                else{
                    node = type_node(class_name);
                }
                node.parent = elm -> super_.text_;

                AST::Method *constructor = (AST::Method *) & (elm -> constructor_);
                AST::Ident *return_ = (ASt::Ident*) * (constructor -> returns_)
                node.construct.returntype = return->text_;

                AST::Formals* formals_node = (AST::Formals*) & (constructor -> formals_);
                vector<AST::Formal *> formals = formals_node->elements_);
                for(AST::Formal *formal: formals){
                    AST::Ident *type = (AST::Ident *) & (formal -> type_);
                    node.construct.formal_arg_types.push_back(type -> text_);
                }

                AST::Block* block_node = (AST::Block*) & (constructor -> statements_);
                vector<AST::Statement *> *statements = (vector<AST::Statement *> *) &block_node -> elements_;
                vector<AST::Statement *> stmts = *statements;

                for(AST::Statement *stmt: stmts){
                    stmt->
                }

            }



        }

        void print_AST_hierarchy(){
            cout << "---------------AST_HIERARCHY-------------" << endl;
            for(map<string,type_node>::iterator iter = hierarchy.begin(); iter != hierarchy.end(); iter++){
                type_node node = iter->second;
                node.print();
                cout << "---------------------------------------------" << endl;
            }
                
            }
        }

        


        void check_AST(){
            if (is_cyclic("Obj")){
                cout << "Cyclid AST Detected" << endl;
                return nullptr;
            }
        }

};


