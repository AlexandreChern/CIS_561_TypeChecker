//
// Created by Michal Young on 9/12/18.
//

#include "ASTNode.h"
#include "semantics.h"
#include "semantics.cxx"
#include <typeinfo>

using namespace std;

namespace AST {
    // Abstract syntax tree.  ASTNode is abstract base class for all other nodes.

    void Method::gen_rvalue(GenContext *ctx, string target_reg) {
        std::string method_name = name_.get_var();
        GenContext *copy_ctx = new GenContext(*ctx);
        copy_ctx->method_name = method_name;
        AST_Type_Node class_node = ctx->stc->AST_hierarchy[copy_ctx->class_name];
        class_and_methods classandmethods = class_node.methods[method_name];
        copy_ctx->emit("");
        if (copy_ctx->class_name == method_name) { 
            copy_ctx->emit("obj_" + method_name + " new_" + method_name + "(" + copy_ctx->get_formal_argtypes("constructor") + ") {");
            copy_ctx->emit("obj_" + method_name + " new_obj = (obj_" + method_name + ") malloc(sizeof(struct obj_" + method_name + "_struct));");
            copy_ctx->emit("new_obj->clazz = the_class_" + method_name + ";");
        }
        else { 
            copy_ctx->emit("obj_" + classandmethods.return_type + " " + copy_ctx->class_name + "_method_" + method_name + "(" + copy_ctx->get_formal_argtypes(method_name) + ") {");
        }
        statements_.gen_rvalue(copy_ctx, target_reg);
        copy_ctx->emit("}");
        copy_ctx->emit("");
    }

    int Program::init_check(set<std::string>* vars, semantics* stc) {
        for (map<std::string, AST_Type_Node>::iterator iter = stc->AST_hierarchy.begin(); iter != stc->AST_hierarchy.end(); iter++) {
            vars->insert(iter->first); 
            AST_Type_Node class_node = iter->second;
            map<std::string, class_and_methods> methods = class_node.methods;
            for(map<std::string, class_and_methods>::iterator iter = methods.begin(); iter != methods.end(); iter++) {
                vars->insert(iter->first); 
            }
        }
        if (classes_.init_check(vars) || statements_.init_check(vars)) {return 1;}
        return 0;
    }

    std::string Ident::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info) {
        if (text_ == "this") {
            AST_Type_Node class_node = stc->AST_hierarchy[info->class_name];
            map<string, string> instance_vars = class_node.instance_vars;
            if (instance_vars.count(text_)) {return instance_vars[text_];}
            else { return "TypeError";}
        }
        if (v_table->count(text_)) { 
            return (*v_table)[text_];
        }
        else { 
            cout << "Type Inference Error: Ident" << endl;
            return "TypeError"; 
        }
    }

    std::string Dot::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info) { 
        string l_type = left_.type_inference(stc, v_table, info); 
        right_.type_inference(stc, v_table, info);
        string r_id = right_.get_var();
        string l_id = left_.get_var();
        map<string, AST_Type_Node> AST_hierarchy = stc->AST_hierarchy;
        AST_Type_Node class_node = AST_hierarchy[l_type];
        map<string, string> instance_vars = class_node.instance_vars;
        if (instance_vars.count(r_id)) {
            return instance_vars[r_id];
        }
        return "Dot:TypeError";
    }    

    std::string Program::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info) { 
        classes_.type_inference(stc, v_table, info);
        class_and_method* pgminfo = new class_and_method("PGM", "");
        map<string, string>* pgmv_table = &(stc->AST_hierarchy)["PGM"].instance_vars;
        statements_.type_inference(stc, pgmv_table, pgminfo);
        return "Nothing";
    }

    std::string Typecase::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info) {
        expr_.type_inference(stc, v_table, info);
        cases_.type_inference(stc, v_table, info);
        return "Nothing";
    }

    std::string Type_Alternative::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info) {
        ident_.type_inference(stc, v_table, info);
        class_name_.type_inference(stc, v_table, info);

        map<string, string>* localv_table = new map<string, string>(*v_table);
        (*localv_table)[ident_.get_var()] = class_name_.get_var();
        block_.type_inference(stc, localv_table, info);

        return "Nothing";
    }

    std::string Construct::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info) { 
        actuals_.type_inference(stc, v_table, info);
        map<std::string, AST_Type_Node> AST_hierarchy = stc->AST_hierarchy;
        std::string method_name = method_.get_var();
        AST_Type_Node class_node = AST_hierarchy[method_name]; 
        class_and_methods constructortable = class_node.construct;        
        if (constructortable.formal_arg_types.size() != actuals_.elements_.size()) {
            cout << "Type Inference Error: Construct" << endl;
            return "Construct:TypeError";
        }
        for (int i = 0; i < actuals_.elements_.size(); i++) {
            std::string formal_type = constructortable.formal_arg_types[i];
            std::string actual_type = actuals_.elements_[i]->type_inference(stc, v_table, info);
            if (!stc->is_subtype(actual_type, formal_type)) {
                cout << "Type Inference Error: Construct" << endl;
                return "Construct:TypeError";
            }
        }
        return method_name; 
    }

    std::string If::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info) {
        std::string cond_type = cond_.type_inference(stc, v_table, info);
        if (cond_type != "Boolean") {
            cout << "TypeError Inference Error: If" << endl;
        }
        truepart_.type_inference(stc, v_table, info);
        falsepart_.type_inference(stc, v_table, info);
        return "Nothing";
    }

    std::string Call::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info) {
        std::string receiver_type = receiver_.type_inference(stc, v_table, info);
        std::string method_name = method_.get_var();
        method_.type_inference(stc, v_table, info);
        actuals_.type_inference(stc, v_table, info);
        map<std::string, AST_Type_Node> AST_hierarchy = stc->AST_hierarchy;
        AST_Type_Node recvnode = AST_hierarchy[receiver_type];
        map<std::string, class_and_methods> methods = recvnode.methods;
        if (!methods.count(method_name)) {
            while (true) {
                std::string class_name = recvnode.parent_type;
                AST_Type_Node parent_node = AST_hierarchy[class_name];
                methods = parent_node.methods;
                if (methods.count(method_name)) {break;} 
                if (class_name == "Obj") {
                    cout << "Type Inference Error: Call " << endl;
                    return "Call:TypeError";
                }
            }
        }
        class_and_methods class_and_methods = methods[method_name];
        if (class_and_methods.formal_arg_types.size() != actuals_.elements_.size()) {
            cout << "Type Inference Error: Call" << endl;
            return "Call:TypeError";
        }
        for (int i = 0; i < actuals_.elements_.size(); i++) {
            string formal_type = class_and_methods.formal_arg_types[i];
            string actual_type = actuals_.elements_[i]->type_inference(stc, v_table, info);
            if (!stc->is_subtype(actual_type, formal_type)) {
                cout << "Type Inference Error: Call" << endl;
                return "Call:TypeError";
            }
        }
        return class_and_methods.return_type;    
    }

    string AssignDeclare::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info)  {
        lexpr_.type_inference(stc, v_table, info);
        string r_type = rexpr_.type_inference(stc, v_table, info);
        string l_var = lexpr_.get_var();
        string static_type = static_type_.get_var();
        map<string, string> instance_vars = (stc->AST_hierarchy)[info->class_name].instance_vars;
        if (!v_table->count(l_var)) { 
            if (instance_vars.count(l_var)) { 
                (*v_table)[l_var] = instance_vars[l_var]; 
            }
            else { 
                (*v_table)[l_var] = static_type; 
                stc->modified = 1; 
            } 
        }
     
        string l_type = (*v_table)[l_var];
        string lca = stc->Type_LCA(l_type, r_type);
        if (l_type != lca) { 
            if (!(stc->is_subtype(lca, static_type))) {
                (*v_table)[l_var] = "TypeError";
                cout << "Type Inference Error: AssignDeclare" << endl;
            }
            (*v_table)[l_var] = lca;
            stc->modified = 1;
        }
        return "Nothing";
    }

    string Assign::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info)  {
        lexpr_.type_inference(stc, v_table, info);
        string r_type = rexpr_.type_inference(stc, v_table, info);
        string l_var = lexpr_.get_var();
        map<string, string> instance_vars = (stc->AST_hierarchy)[info->class_name].instance_vars;
        if (!v_table->count(l_var)) { 
            if (instance_vars.count(l_var)) { 
                (*v_table)[l_var] = instance_vars[l_var]; 
            }
            else { 
                (*v_table)[l_var] = r_type; 
                stc->modified = 1; 
            } 
        }
        string l_type = (*v_table)[l_var];
        string lca = stc->Type_LCA(l_type, r_type);
        if (l_type != lca) { 
            (*v_table)[l_var] = lca;
            stc->modified = 1;
        }
        return "Nothing";
    }

    string Methods::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info) {

        for (Method* method: elements_) {
            string method_name = method->name_.get_var();
            AST_Type_Node classentry = stc->AST_hierarchy[info->class_name];
            class_and_methods class_and_methods = classentry.methods[method_name];
            map<string, string>* methodvars = class_and_methods.vars;
            class_and_method* methodinfo = new class_and_method(info->class_name, method_name);
            method->type_inference(stc, methodvars, methodinfo);
            map<string, string> classinstance = classentry.instance_vars;
            for(map<string, string>::iterator iter = methodvars->begin(); iter != methodvars->end(); iter++) {
                if (classinstance.count(iter->first)) { 
                    string methodtype = iter->second;
                    string classtype = classinstance[iter->first];
                    if (!stc->is_subtype(methodtype, classtype)) {
                        cout << "Type Inference Error: Methods" << endl;
                    }
                }
            }
        }
        return "Nothing";
    }

    string Classes::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info) {
        for (AST::Class *cls: elements_) {
            class_and_method* info = new class_and_method(cls->name_.get_var(), "");
            cls->type_inference(stc, v_table, info);
        }

        map<string, AST_Type_Node> AST_hierarchy = stc->AST_hierarchy;
        for (AST::Class *cls: elements_) {
            string class_name  = cls->name_.get_var();
            AST_Type_Node class_node = AST_hierarchy[class_name];
            string parent_name = class_node.parent_type;
            AST_Type_Node parent_node = AST_hierarchy[parent_name];
            map<string, string> class_iv = class_node.instance_vars;
            map<string, string> parent_iv = parent_node.instance_vars;
            for(map<string, string>::iterator iter = parent_iv.begin(); iter != parent_iv.end(); iter++) {
                string var_name = iter->first;
                if (!class_iv.count(var_name)) {
                    cout << "Type Inference Error: Classes" << endl;
                }
            }
        }
        return "Nothing";
    }

    string Class::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info) {
            int returnval = 0;
            map<string, string>* instance_vars = &(stc->AST_hierarchy[info->class_name].instance_vars);
            AST_Type_Node * class_node = &stc->AST_hierarchy[info->class_name];
            class_and_methods * constructor = &class_node->construct;
            map<string, string>* construct_instvars = constructor->vars;
            constructor_.type_inference(stc, construct_instvars, info);

            for(map<string, string>::iterator iter = instance_vars->begin(); iter != instance_vars->end(); iter++) {
                if (iter->first.rfind("this", 0) == 0) {
                    (*instance_vars)[iter->first] = (*construct_instvars)[iter->first];
                    vector<string> class_split = stc->split(iter->first, '.');
                    if (class_split.size() == 2) {
                        (*instance_vars)[class_split[1]] = (*construct_instvars)[iter->first];
                    }
                }   
            }
            (*instance_vars)["this"] = info->class_name; 
            (*construct_instvars)["this"] = info->class_name;

            info = new class_and_method(name_.get_var(), "");
            methods_.type_inference(stc, v_table, info);
            return "Nothing";
    }

    string Return::type_inference(semantics* stc, map<string, string>* v_table, class_and_method* info) {
        string method_name = info->method_name;
        AST_Type_Node class_node = stc->AST_hierarchy[info->class_name];
        class_and_methods class_and_methods = class_node.methods[method_name];
        string method_type = class_and_methods.return_type;
        string inference_type = expr_.type_inference(stc, v_table, info);
        if (!stc->is_subtype(inference_type, method_type)) {
            cout << "Type Inference Error: Return" << endl;
        }
        return "Nothing";
    }

    // JSON representation of all the concrete node types.
    // This might be particularly useful if I want to do some
    // tree manipulation in Python or another language.  We'll
    // do this by emitting into a stream.

    // --- Utility functions used by node-specific json output methods

    /* Indent to a given level */
    void ASTNode::json_indent(ostream& out, AST_print_context& ctx) {
        if (ctx.indent_ > 0) {
            out << endl;
        }
        for (int i=0; i < ctx.indent_; ++i) {
            out << "    ";
        }
    }

    /* The head element looks like { "kind" : "block", */
    void ASTNode::json_head(string node_kind, ostream& out, AST_print_context& ctx) {
        json_indent(out, ctx);
        out << "{ \"kind\" : \"" << node_kind << "\"," ;
        ctx.indent();  // one level more for children
        return;
    }

    void ASTNode::json_close(ostream& out, AST_print_context& ctx) {
        // json_indent(out, ctx);
        out << "}";
        ctx.dedent();
    }

    void ASTNode::json_child(string field, ASTNode& child, ostream& out, AST_print_context& ctx, char sep) {
        json_indent(out, ctx);
        out << "\"" << field << "\" : ";
        child.json(out, ctx);
        out << sep;
    }

    void Stub::json(ostream& out, AST_print_context& ctx) {
        json_head("Stub", out, ctx);
        json_indent(out, ctx);
        out  << "\"rule\": \"" <<  name_ << "\"";
        json_close(out, ctx);
    }


    void Program::json(ostream &out, AST::AST_print_context &ctx) {
        json_head("Program", out, ctx);
        json_child("classes_", classes_, out, ctx);
        json_child("statements_", statements_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Formal::json(ostream &out, AST::AST_print_context &ctx) {
        json_head("Formal", out, ctx);
        json_child("var_", var_, out, ctx);
        json_child("type_", type_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Method::json(ostream &out, AST::AST_print_context &ctx) {
        json_head("Method", out, ctx);
        json_child("name_", name_, out, ctx);
        json_child("formals_", formals_, out, ctx);
        json_child("returns_", returns_, out, ctx);
        json_child("statements_", statements_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Assign::json(ostream& out, AST_print_context& ctx) {
        json_head("Assign", out, ctx);
        json_child("lexpr_", lexpr_, out, ctx);
        json_child("rexpr_", rexpr_, out, ctx, ' ');
        json_close(out, ctx);
     }

    void AssignDeclare::json(ostream& out, AST_print_context& ctx) {
        json_head("Assign", out, ctx);
        json_child("lexpr_", lexpr_, out, ctx);
        json_child("rexpr_", rexpr_, out, ctx);
        json_child("static_type_", static_type_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Return::json(ostream &out, AST::AST_print_context &ctx) {
        json_head("Return", out, ctx);
        json_child("expr_", expr_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void If::json(ostream& out, AST_print_context& ctx) {
        json_head("If", out, ctx);
        json_child("cond_", cond_, out, ctx);
        json_child("truepart_", truepart_, out, ctx);
        json_child("falsepart_", falsepart_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void While::json(ostream& out, AST_print_context& ctx) {
        json_head("While", out, ctx);
        json_child("cond_", cond_, out, ctx);
        json_child("body_", body_, out, ctx, ' ');
        json_close(out, ctx);
    }


    void Typecase::json(ostream& out, AST_print_context& ctx) {
        json_head("Typecase", out, ctx);
        json_child("expr_", expr_, out, ctx);
        json_child("cases_", cases_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Type_Alternative::json(ostream& out, AST_print_context& ctx) {
        json_head("Type_Alternative", out, ctx);
        json_child("ident_", ident_, out, ctx);
        json_child("class_name_", class_name_, out, ctx);
        json_child("block_", block_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Load::json(ostream &out, AST::AST_print_context &ctx) {
        json_head("Load", out, ctx);
        json_child("loc_", loc_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Ident::json(ostream& out, AST_print_context& ctx) {
        json_head("Ident", out, ctx);
        out << "\"text_\" : \"" << text_ << "\"";
        json_close(out, ctx);
    }

    void Class::json(ostream &out, AST::AST_print_context &ctx) {
        json_head("Class", out, ctx);
        json_child("name_", name_, out, ctx);
        json_child("super_", super_, out, ctx);
        json_child("constructor_", constructor_, out, ctx);
        json_child("methods_", methods_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Call::json(ostream &out, AST::AST_print_context &ctx) {
        json_head("Call", out, ctx);
        json_child("obj_", receiver_, out, ctx);
        json_child("method_", method_, out, ctx);
        json_child("actuals_", actuals_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Construct::json(ostream &out, AST::AST_print_context &ctx) {
        json_head("Construct", out, ctx);
        json_child("method_", method_, out, ctx);
        json_child("actuals_", actuals_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void IntConst::json(ostream& out, AST_print_context& ctx) {
        json_head("IntConst", out, ctx);
        out << "\"value_\" : " << value_ ;
        json_close(out, ctx);
    }

    void StrConst::json(ostream& out, AST_print_context& ctx) {
        json_head("StrConst", out, ctx);
        out << "\"value_\" : \"" << value_  << "\"";
        json_close(out, ctx);
    }


    void BinOp::json(ostream& out, AST_print_context& ctx) {
        json_head(opsym, out, ctx);
        json_child("left_", left_, out, ctx);
        json_child("right_", right_, out, ctx, ' ');
        json_close(out, ctx);
    }


    void Not::json(ostream& out, AST_print_context& ctx) {
        json_head("Not", out, ctx);
        json_child("left_", left_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Dot::json(ostream& out, AST_print_context& ctx) {
        json_head("Dot", out, ctx);
        json_child("left_", left_, out, ctx);
        json_child("right_", right_, out, ctx, ' ');
        json_close(out, ctx);
    }


    /* Convenience factory for operations like +, -, *, / */
    Call* Call::binop(string opname, Expr& receiver, Expr& arg) {
        Ident* method = new Ident(opname);
        Actuals* actuals = new Actuals();
        actuals->append(&arg);
        return new Call(receiver, *method, *actuals);
    }
}