#include "ASTNode.h"
#include "semantics.h"
#include <iostream>
#include <set>
#include <map>
#include <vector>


using namespace std;

class class_and_methods {
    public:
        string method_name;
        string return_type;

        vector<string> formal_arg_types;
        map<string, string>* vars;

        string inhereted_from;

        class_and_methods(){
            formal_arg_types = vector<string>();
            vars = new map<string, string>();
        }

        class_and_methods(string name){
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

        cout << "\t " << "variables: " << endl;
        for (map<string,string>::iterator iter = vars->begin(); iter != vars->end();iter++) {
            cout << "\t " << iter->first << ": " << iter->second << endl;
        } 
        cout << "\t " << "inhereted_from" << inhereted_from << endl;
        cout << endl;
    }
    
};

class type_node {
    public:
        string var_type;
        string parent;

        map<string, string> instance_vars;
        map<string, class_and_methods> methods;

        class_and_methods constructor;
        int resolved;
        vector<string> method_list;

    type_node(){
        instance_vars = map<string, string>();
        methods = map<string, class_and_methods>();
        constructor = class_and_methods();
        resolved = 0;
        method_list = vector<string>();
    }

    type_node(string var_name){
        var_type = var_name;
        instance_vars = map<string, string>();
        methods = map<string, class_and_methods>();
        constructor = class_and_methods(var_name);
        resolved = 0;
        method_list = vector<string>();
    }

    void print_type(){
        cout << "\t " << "var_type: " << var_type << endl;
        cout << "\t " << "perent: " << parent << endl;
        cout << "\t " << "instance_vars: " << instance_vars;

        for(map<string,string>::iterator iter = instance_vars.begin(); iter != instance_vars.end(); iter++){
            cout << "\t " << iter->first << ":" << iter->second << endl;
        }

        cout << "Method_List: " << endl;
        for (string method: method_list){
            cout << method << ", ";
        }
        cout << endl;

        cout << "Methods: " << endl;

        for(map<string,class_and_methods>::iterator iter = methods.begin(); iter != methods.end(); iter++){
            class_and_methods method = iter->second;
            method.print_method();
        }
        cout << "constructor: " << endl;
        constructor.print_method();
    }
};

class AST_edge{
    public:
        vector<string> children;
        int visited;

    AST_edge(){
        children = vector<string>();
        visited = 0;
    }

    void print_edge(){
        cout << "children: " << endl;
        for (string child: children){
            cout << child << ", "
        }
        cout << endl;
        cout << "visited: " << visited << endl;
    }
};


class semantics{
    public:
        AST::ASTNode*  AST_init_root;
        // void semantic_error{
        //     cout << "\t semantic error" << endl;
        // };
        int found_error;
        int modified = 1;

        map<string, type_node> hierarchy;
        map<string, AST_edge*> AST_edges;
        vector<string> sorted_classes;

        semantics(AST::ASTNode* root){
            AST_init_root = root;
            found_error = 0;

            hierarchy = map<string, type_node>();
            AST_edges = map<string, AST_edge*>();
            sorted_classes = vector<string>();
        }

        void topological_sort_utils(type_node* node){
            if (!node->resolved){
                string parent = node->parent;
                type_node* grand_parent = &hierarchy[parent];
                topological_sort_utils(grand_parent);
                sorted_classes.push_back(node->var_type);
                node->resolved = 1;
            }
        }

        void topological_sort(){
            sorted_classes.push_back("Obj");
            for (map<string,type_node>::iterator iter = hierarchy.begin(); iter != hierarchy.end(); iter++){
                type_node *node = &hierarchy[iter->first];
                topological_sort_utils(node);
            }
        }

        int pop_AST_edges(){
             for(map<string,type_node>::iterator iter = hierarchy.begin(); iter != hierarchy.end(),iter++){
                type_node node = iter->second;
                AST_edges[node.type] = new AST_edge();

            for(map<string,type_node>::iterator iter = hierarchy.begin(); iter != hierarchy.end(), iter++){
                type_node node = iter->end;
                string parent = node->parent; // node.parent ?
                if (iter->first = "Obj"){
                    continue;
                }

                if (!AST_edges.count(node.parent)){
                    cout << "Error: " << node.parent << "undefined" << endl;
                    return 0;
                }
                AST_edges[node.parent]->children.push_back(node->var_type);
            }
            return 1;
        }


        int is_cyclic(string root){
            AST_edge* AST_root_edge = AST_edges[root];
            for (string child: AST_root_edge->children){
                AST_edges AST_child_edge = AST_edges[child];
                if (AST_child_edge->visited){
                    return 1;
                }
                if(is_cyclic(child)){
                    return 1;
                }
            }
            return 0;
        }

        int is_subtype(string sub, string super){
            set<string> sub_path_to_root = set<string>();
            string var_type = sub;
            if (!hierarchy.cout(var_type)){
                cout << "Error: var_type " << var_type << "not found in class hierarchy" << endl;
                return 0;
            }

            while (1) {
                sub_path_to_root.insert(var_type);
                if (var_type == "Obj"){
                    break;
                }
                var_type = hierarchy[var_type].parent;
                if (sub_path_to_root.count(super)){
                    return 1;
                }
                return 0;
            }
        }

        void pop_Builtins(){

            type_node program("PGM");
            program.parent = "Obj";
            hierarchy("PGM") = program;

            type_node obj("Obj");
            obj.parent = "ERROR";
            hierarchy["Obj"] = obj;
            
            class_and_methods mtb("Mtb"); 
            mtb.return_type = "Nothing";
            hierarchy["Obj"].methods["Mtb"] = mtb;

            type_node integer("Int");
            integer.parent = "Obj";
            hierarchy["Int"] = integer;
            
            type_node str("String");
            str.parent = "Obj";
            hierarchy["String"] = str;

            type_node booleen("Booleen");
            booleen.parent = "Obj";
            hierarchy["Booleen"] = booleen;

            type_node nothing("Nothing");
            nothing.parent = "Obj";
        }

        void pop_AST_hierarchy(){
            pop_Builtins();

            AST::Program *root = (AST::Program*) AST_init_root;
            AST::Classes  classes_node = root->classes_;

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
                node.constructor.return_type = return_->text_;

                AST::Formals* formals_node = (AST::Formals*) & (constructor -> formals_);
                vector<AST::Formal *> formals = formals_node->elements_);
                for(AST::Formal *formal: formals){
                    AST::Ident *type = (AST::Ident *) & (formal -> type_);
                    node.constructor.formal_arg_types.push_back(type -> text_);
                }

                AST::Block* block_node = (AST::Block*) & (constructor -> statements_);
                vector<AST::Statement *> *statements = (vector<AST::Statement *> *) &block_node -> elements_;
                vector<AST::Statement *> stmts = *statements;

                for(AST::Statement *stmt: stmts){
                    stmt->get_vars(&node.instance_vars)
                }

                vector<AST::Method *> methods = (elm -> methods_).elements_;
                for (AST::Method *method: methods){
                    AST::Ident* method_name = (AST::Ident*) & (method->name_);
                    AST::Ident* return_type = (AST::Ident*) & (method->returns_);
                    class_and_methods new_method(method_name -> text_);
                    new_method.return_type = return_type->text_;
                    AST::Formals* formals_node = (AST::Formals*) & (constructor->formals_);
                    vector<AST::Formal *> formals = formals_node -> elements;
                    for (AST::Formal *formal: formals){
                        AST::Ident *type = (AST::Ident*) & (formal->type_);
                        new_method.formal_arg_types.push_back(type->text_)
                    }
                    new_method.inhereted_from = class_name;
                    node.methods[new_method.method_name] = new_method;   
                }
                hierarch[class_name] = node;

            }

        }

        void print_AST_hierarchy(){
            cout << "---------------AST_HIERARCHY-------------" << endl;
            for(map<string,type_node>::iterator iter = hierarchy.begin(); iter != hierarchy.end(); iter++){
                type_node node = iter->second;
                node.print_type();
                cout << "---------------------------------------------" << endl;
            }
                
            }
        }

        
        void search_vector(vector<string>* vec, string target){
            for (string s: *vec){
                if (s == target){
                    return 1; 
                }
                return 0;
            }
        }

        void inherit_methods(){
            for (string class_name: sorted_classes){
                if (class_name == "PGM"){
                    continue;
                }
                if (class_name == "Obj") {
                    continue;
                }
                type_node *class_node = &hierarchy[class_name];
                map<string, class_and_methods> *class_methods = &class_node->methods;
                string parent = class_node->parent;
                type_node *parent_node = &hierarchy[parent];
                
                for (string method: parent_node->method_list){
                    class_node->method_list.push_back(method);
                }

                for (map<string, class_and_methods>::iterator iter = class_methods->begin(); iter !=class_methods->end(); iter++){
                    if (!search_vector(&class_node->method_list, iter->first)){
                        class_node -> method_list.push_back(iter->first);
                    }
                }

                for (string s: class_node->method_list){
                    if(!class_methods->count(s)){
                        class_and_methods parent_method = parent_node->methods[s];
                        class_and_methods mtable = class_and_methods(parent_method);
                        (*class_methods)[s] = mtable;
                    }
                }
            }
        }

        string get_LCA(string type_1, string type_2){
            if (type_1 == "Bottom" || type_1 == "TypeError") {return type_2;}
            if (type_2 == "Bottom" || type_2 == "TypeError") {return type_1;}

            if (!hierarchy.count(type_1)){
                return type_1;
            }

            if (!hierarchy.count(type_2)){
                return type_2;
            }
            set<string> type_1_path = set<string>();
            string type = type_1;

            while(1) {
                type_1_path.insert(type)
                if (type == "Obj"){break;}
                type = hierarchy[type].parent;
            }

            type = type_2
            while(1) {
                if (type_1_path.count(type)){
                    return type
                }
                type = hierarchy[type].parent;
            }
        }


        void check_AST(){
            pop_AST_hierarchy();
            cout << "---------- PRINT AST HIERARCHY -----------" << endl;
            print_AST_hierarchy();

            if (is_cyclic("Obj")){
                cout << "---------- CYCLID AST DETECTED -----------" << endl;
                return nullptr;
            }

            if (!pop_AST_edges()){
                return nullptr;
            }

            else {
                cout << "--------------  ASYCLIC AST GENERATED  -------------" << endl;
            }

            topological_sort();
            inherit_methods();
            cout << "---------------  AST AFTER SORTED -----------------" << endl;
            print_AST_hierarchy();

            AST::Program *root = (AST::Program*) AST_init_root;
            set<string> *vars = new set<string>;
            if (root->init_check(vars, this)){
                cout << "-------------- INITIALIZATION ERRORS --------------" << endl;
                return nullptr;
            }
            type_check();
            print_AST_hierarchy();
            return &hierarchy
        }



        vector<string> split(string string_to_split, char delimeter)
        {
            stringstreawm ss(string_to_split);
            string item;
            vector<string> splitted_strings;
            while(getline(ss, item, delimeter)){
                splited_strings.push_back(item);
            }
            return splitted_strings;
        }

        int compare_maps(map<string,string> map_1, map<string,string> map_2){
            if (map_1.size() != map2.size()){
                return 0;
            }
            type_name map<string,string>::iterator i,j;
            for (i = map_1.begin(), j = map_2.begin(); i!= map_1.end(); i++, j++){
                if (*i != *j){
                    return 0;
                }
                return 1;
            }
        }

        map<string, TypeNOde>* type_check(){
            AST::Program *root = (AST::Program*) AST_init_root;
            int changed = 1;
            while (changed) {
                changed = root->type_inference(this, nullptr, nullptr);
            }
            return &this->hierarchy;
        }

        AST::Program *root = (AST::Program*) AST_init_root;
        set<string> *vars = new set<string>;

        if(root->init_check(vars, this))

};


