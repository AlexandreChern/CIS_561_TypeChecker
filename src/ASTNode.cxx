//
// Created by Michal Young on 9/12/18.
//

#include "ASTNode.h"
#include "semantics.h"
#include "semantics.cxx"
#include <typeinfo>

using namespace std;
class class_and_method;

namespace AST {
    // Abstract syntax tree.  ASTNode is abstract base class for all other nodes.

    // Add type_inference files

    int Program::init_check(set<string>* vars, semantics* stc){
        for (map<std::string,type_node>::iterator iter=stc->heirarchy.begin(); iter!=stc->heirarchy.end(),iter ++){
            vars->insert(iter->first);
            type_node class_node = iter->second;
            map<std::string,class_and_methods> methods = class_node.methods;
            for (map<std::string,class_and_methods>::iterator iter=methods.begin(); iter!=methods.end(),iter++){
                vars->insert(iter->first);
            }
        }
        if(classes_.init_check(vars) || statements_.init_check(vars)){
            return 1;
        }
        return 0;
    }

    std::string Program::type_inference(semantics* stc, map<string,string>*vtable, class_and_method* info){
            classes_.type_inference(stc, vtable, info);
            class_and_methods* program_info = new class_and_methods("PGM");
            map<std::string,std::string>* program_vtable = &(stc->hierarchy)["PGM"].instance_vars;
            statements_.type_inference(stc,program_vtable, program_info);
            return "TYPE INFERENCE FOR PROGRAM"
    }

    std::string Typecase::type_inference(semantics* stc, map<string,string>*vtable, class_and_method* info){
        expr_.type_inference(stc, vtable, info);
        cases_.type_inference(stc,vtable, info);
        return "TYPE INFERENCE FOR TYPECASE";
    }

    std::string TypeAlternative::type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info){
        ident_.type_inference(stc,vtable,info);
        classname_.type_inference(stc,vtable,info);

        map<std::string,std::string>* local_vtable = new map<std::string, std::string>(*vtable);
        (*local_vtable)[ident_.get_var()] = classname_.get_var();
        block_.type_inference(stc,vtable, info);
        return "TYPE INFERENCE FOR TYPE ALTERNATIVE"

    }

    std::string Construct::type_inference(){
        actuals_.type_inference(stc, vtable, info);
        map<std::string, type_node> hierarchy = stc->hierarchy;
        std::string method_name = method_.get_var();
        type_node class_node = hierarchy[method_name];
        class_and_methods constructor_table = class_node.constructor;

        if (constructor_table.formal_arg_types.size()!= actuals_.elements.size()){
            return "TYPE INFERENCE ERROR FOR CONSTRUCTOR";
        }

        for (int i=0; i < actuals_.element_.size(); i++){
            std::string formal_type = constructor_table.formal_arg_types[i];
            std::string actual_type = actuals_.elements_[i]->type_inference(stc, vtable, info);
            if(!stc -> is_subtype(actual_type, formal_type)){
                cout << "TYPE INFERENCE ERROR" << endl;
                return "TYPE INFERENCE ERROR";
            }
        }
        return method_name;

    }

    std::string If::type_inference(semantics* stc, map<std::string,std::string>* vtable, class_and_method* info){
        std::string cond_type = cond_.type_inference(stc, vtable, info);
        if (cond_type != "Boolean"){
            return "TYPE INFERENCE ERROR FOR IF"
        }
        truepart_.type_inference(stc, vtable, info);
        falsepart_.type_inference(stc,vtable, info);
        return "TYPE INFERENCE FOR IF";
    }


    std::string Call::type_inference(semantics* stc, map<std::string,std::string>* vtable, class_and_method* info){
        std::string receiver_type = receiver_.type_inference(stc, vtable, info);
        std::string method_name = method_.get_var();
        method_.type_inference(stc,vtable, info);
        actuals_.type_inference(stc,vtable,info);
        map<std::string,type_node> hierarchy = stc->hierarchy;
        type_node receiver_node = hierarchy[receiver_type];
        map<std::string, class_and_methods*> methods = recver_node.methods;
        if (!methods.count(method_name)){
            while(true){
                std::string parent_name = receiver_node.parent;
                type_node = parent_node = hierarchy[parent_name];
                methods = parent_node.methods;
                if(methods.count(method_name)){break;}
                if(class_name = "Obj"){
                    return "TYPE INFERENCE ERROR FOR CALL";
                }
            }
        }
        class_and_methods* class_and_methods_table = methods[method_name];
        if (class_and_methods_table.formal_arg_types.size() != actuals_.elements_.size()){
            return "TYPE INFERENCE ERROR FOR CALL";
        }

        for (int i = 0; i < actuals_.elements_.size();i++){
            std::string formal_type = class_and_methods_table.formal_arg_types[i];
            std::string actual_type = actuals_.elements_[i]->type_inference(stc, vtable, info);
            if (!stc->is_subtype(actual_type, formal_type)){
                return "TYPE INFERENCE ERROR FOR CALL";
            }
        }
        return class_and_methods_table.return_type;

    }

    std::string AssignDeclare::type_inference(semantics* stc, map<std::string,std::string>* vtable, class_and_method* info){
        lexpr_.type_inference(stc, vtable, info);
        std::string rtype =  repxr_.type_inference(stc, vtable, info);
        std::string lvar = lexpr_.get_var();
        std::string static_type = static_type_.get_var();
        map<std::string,std::string> instance_vars = (stc->hierarchy)[info->class_name].instance_vars;
        if (!vtable->count(lvar)){
            if (instance_vars.count(lvar)){
                (*vtable)[lvar] = instance_vars[lvar];
            }
            else{
                (*vtable)[lvar] = static_type;
                stc->modified=1;
            }
        }
        std::string ltype = (*vtable)[lvar];
        std::string lca = stc->get_LCA(ltype,rtype);
        if (ltype!= lca){
            if (!(stc->is_subtype(lca, static_type))){
                (*vtable)[lvar] = "TYPE ERROR";
                std::cout << "TYPE ERROR" << std::endl;
            }
            (*vtable)[lvar] = lca;
            stc->modified = 1;
        }

        return "TYPE INFERENCE FOR ASSIGNDECLARE";
    }

    std::string Assign::type_inference(){
        lexpr_.type_inference(stc, vtable, info);
        std::string rtype = rexpr_.type_inference(stc, vtable, info);
        std::string lvar = lexpr_.get_var();
        map<std::string,std::string> instance_vars=(stc->hierarchy)[info->class_name].instance_vars;
        if(!vtable->count(lvar)){
            if (instance_vars.count(lvar)){
                (*vtable)[lvar] = instance_vars[lvar];
            }
            else{
                (*vtable)[lvar] = rtype;
                stc->modified=1;
            }
        }

        std::string ltype = (*vt)[lvar];
        std::string lca = stc->get_LCA(ltype,rtype);
        if (ltype!= lca)
    }

    std::string Methods::type_inference(semantics* stc, map<std::string,std::string> vtable, class_and_method* info){
        for (Method* method: elements_){
            std::string method_name = method->name_.get_var();
            type_node method_class = stc->hierarchy[info->class_name];
            class_and_methods m_vars = method_class.methods[method_name];
            map<std::string,std::string>* method_info = new class_and_method(info->class_name, method_name);
            method->type_inference(stc, m_vars, method_info);
            map<std::string, std::string> class_instance = method_class.instance_vars;
            for(map<std::string,std::string>* iterator iter=m_vars->begin(); iter!=m_vars->end(); iter++){
                if(class_instance.count(iter->first)){
                    std::string method_type = iter->second;
                    std::string class_type = class_instance[iter->first];
                    if (!stc->is_subtype(method_type, class_type)){
                        std::cout << "TYPE INFERENCE ERROR FOR METHODS" << std::endl;
                    }
                }
            }
        }
        return "TYPE INFERENCE FOR METHODS";

    }

    std::string Classes::type_inference(semantics* stc, map<std::string, std::string> vtable, class_and_method* info){
        
    }

    std::string Class::type_inference(){
            // Not Implemented
            return "TYPE INFERENCE FOR CLASS";
    }


    std::string Return::type_inference(){
            // Not Implemented
            return "TYPE INFERENCE FOR RETURN";
    }














    // JSON representation of all the concrete node types.
    // This might be particularly useful if I want to do some
    // tree manipulation in Python or another language.  We'll
    // do this by emitting into a stream.

    // --- Utility functions used by node-specific json output methods

    /* Indent to a given level */
    void ASTNode::json_indent(std::ostream& out, AST_print_context& ctx) {
        if (ctx.indent_ > 0) {
            out << std::endl;
        }
        for (int i=0; i < ctx.indent_; ++i) {
            out << "    ";
        }
    }

    /* The head element looks like { "kind" : "block", */
    void ASTNode::json_head(std::string node_kind, std::ostream& out, AST_print_context& ctx) {
        json_indent(out, ctx);
        out << "{ \"kind\" : \"" << node_kind << "\"," ;
        ctx.indent();  // one level more for children
        return;
    }

    void ASTNode::json_close(std::ostream& out, AST_print_context& ctx) {
        // json_indent(out, ctx);
        out << "}";
        ctx.dedent();
    }

    void ASTNode::json_child(std::string field, ASTNode& child, std::ostream& out, AST_print_context& ctx, char sep) {
        json_indent(out, ctx);
        out << "\"" << field << "\" : ";
        child.json(out, ctx);
        out << sep;
    }

    void Stub::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Stub", out, ctx);
        json_indent(out, ctx);
        out  << "\"rule\": \"" <<  name_ << "\"";
        json_close(out, ctx);
    }


    void Program::json(std::ostream &out, AST::AST_print_context &ctx) {
        json_head("Program", out, ctx);
        json_child("classes_", classes_, out, ctx);
        json_child("statements_", statements_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Formal::json(std::ostream &out, AST::AST_print_context &ctx) {
        json_head("Formal", out, ctx);
        json_child("var_", var_, out, ctx);
        json_child("type_", type_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Method::json(std::ostream &out, AST::AST_print_context &ctx) {
        json_head("Method", out, ctx);
        json_child("name_", name_, out, ctx);
        json_child("formals_", formals_, out, ctx);
        json_child("returns_", returns_, out, ctx);
        json_child("statements_", statements_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Assign::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Assign", out, ctx);
        json_child("lexpr_", lexpr_, out, ctx);
        json_child("rexpr_", rexpr_, out, ctx, ' ');
        json_close(out, ctx);
     }

    void AssignDeclare::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Assign", out, ctx);
        json_child("lexpr_", lexpr_, out, ctx);
        json_child("rexpr_", rexpr_, out, ctx);
        json_child("static_type_", static_type_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Return::json(std::ostream &out, AST::AST_print_context &ctx) {
        json_head("Return", out, ctx);
        json_child("expr_", expr_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void If::json(std::ostream& out, AST_print_context& ctx) {
        json_head("If", out, ctx);
        json_child("cond_", cond_, out, ctx);
        json_child("truepart_", truepart_, out, ctx);
        json_child("falsepart_", falsepart_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void While::json(std::ostream& out, AST_print_context& ctx) {
        json_head("While", out, ctx);
        json_child("cond_", cond_, out, ctx);
        json_child("body_", body_, out, ctx, ' ');
        json_close(out, ctx);
    }


    void Typecase::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Typecase", out, ctx);
        json_child("expr_", expr_, out, ctx);
        json_child("cases_", cases_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Type_Alternative::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Type_Alternative", out, ctx);
        json_child("ident_", ident_, out, ctx);
        json_child("classname_", classname_, out, ctx);
        json_child("block_", block_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Load::json(std::ostream &out, AST::AST_print_context &ctx) {
        json_head("Load", out, ctx);
        json_child("loc_", loc_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Ident::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Ident", out, ctx);
        out << "\"text_\" : \"" << text_ << "\"";
        json_close(out, ctx);
    }

    void Class::json(std::ostream &out, AST::AST_print_context &ctx) {
        json_head("Class", out, ctx);
        json_child("name_", name_, out, ctx);
        json_child("super_", super_, out, ctx);
        json_child("constructor_", constructor_, out, ctx);
        json_child("methods_", methods_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Call::json(std::ostream &out, AST::AST_print_context &ctx) {
        json_head("Call", out, ctx);
        json_child("obj_", receiver_, out, ctx);
        json_child("method_", method_, out, ctx);
        json_child("actuals_", actuals_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Construct::json(std::ostream &out, AST::AST_print_context &ctx) {
        json_head("Construct", out, ctx);
        json_child("method_", method_, out, ctx);
        json_child("actuals_", actuals_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void IntConst::json(std::ostream& out, AST_print_context& ctx) {
        json_head("IntConst", out, ctx);
        out << "\"value_\" : " << value_ ;
        json_close(out, ctx);
    }

    void StrConst::json(std::ostream& out, AST_print_context& ctx) {
        json_head("StrConst", out, ctx);
        out << "\"value_\" : \"" << value_  << "\"";
        json_close(out, ctx);
    }


    void BinOp::json(std::ostream& out, AST_print_context& ctx) {
        json_head(opsym, out, ctx);
        json_child("left_", left_, out, ctx);
        json_child("right_", right_, out, ctx, ' ');
        json_close(out, ctx);
    }


    void Not::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Not", out, ctx);
        json_child("left_", left_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Dot::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Dot", out, ctx);
        json_child("left_", left_, out, ctx);
        json_child("right_", right_, out, ctx, ' ');
        json_close(out, ctx);
    }


    /* Convenience factory for operations like +, -, *, / */
    Call* Call::binop(std::string opname, Expr& receiver, Expr& arg) {
        Ident* method = new Ident(opname);
        Actuals* actuals = new Actuals();
        actuals->append(&arg);
        return new Call(receiver, *method, *actuals);
    }
}