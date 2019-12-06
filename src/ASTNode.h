//
// Created by Michal Young on 9/12/18.
//

#ifndef ASTNODE_H
#define ASTNODE_H

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <typeinfo>
#include <map>

#include "semantics.h"
#include "CodegenContext.h"

using namespace std;

class semantics;
class class_and_method;
class class_and_methods;


namespace AST {
    // Abstract syntax tree.  ASTNode is abstract base class for all other nodes.

    // Json conversion and pretty-printing can pass around a print context object
    // to keep track of indentation, and possibly other things.
    class AST_print_context {
    public:
        int indent_; // Number of spaces to place on left, after each newline
        AST_print_context() : indent_{0} {};
        void indent() { ++indent_; }
        void dedent() { --indent_; }
    };

    class ASTNode {
    public:
        // virtual std::int eval(EvalContext &ctx) = 0;
        virtual std::string gen_rvalue(GenContext *ctx, std::string target_reg) {
            std::cout << "GENERATE RVALUE NOT IMPLEMENTED" << endl; return "";
            }

        virtual std::string gen_lvalue(GenContext *ctx){
            std::cout << "GENERATE LVALUE NOT IMPLEMENTED" << endl; return "";
        }

        virtual std::string gen_branch(GenContext *ctx, std:string true_branch, std::string false_branch){
            cout << "GENERATE BRANCH NOT IMPLEMENTD" << endl;
            return "";
        }



        virtual int init_check(set<std::string>* vars){
            cout << "TYPE CHECK NOT IMPLEMENTED" << endl; 
            return 0;
        }

        virtual std::string type_inference(semantics* stc, map<std::string,std::string>* vtable, std::string class_name){
            cout << "TYPE INFERENCE NOT INPLEMENTED" << endl;
            return "TYPE INFERENCE NOT IMPLEMENTED";
        }

        virtual void json(std::ostream& out, AST_print_context& ctx) = 0; 

        virtual std::string get_vars(std::map<std::string, std::string>* vtable) {std::cout << "GET VARS" <<endl;} // Need to be added
        virtual std::string get_var(){std::cout << "GET VAR" << std::endl; return "";}
        virtual std::string get_type(std::map<std::string, std::string>* vtable, semantics* stc, std::string class_name){
            std::cout << "GET TYPE" << std::endl; return ""; 
        }

        std::string str() {
            std::stringstream ss;
            AST_print_context ctx;
            json(ss, ctx);
            return ss.str();
        }
    protected:
        void json_indent(std::ostream& out, AST_print_context& ctx);
        void json_head(std::string node_kind, std::ostream& out, AST_print_context& ctx);
        void json_close(std::ostream& out, AST_print_context& ctx);
        void json_child(std::string field, ASTNode& child, std::ostream& out, AST_print_context& ctx, char sep=',');
    };

    class Stub : public ASTNode {
        std::string name_;
    public:
        std::string type_inference(semantics* stc, map<std::string, std::string>* vt, class_and_method* info){
            std::cout << "STUB TYPE INFERENCE UNIMPLEMENTED" << std::endl;
            return "STUB TYPE INFERENCE NOT IMPLEMENTED";
        }
        explicit Stub(std::string name) : name_{name} {};
        void json(std::ostream& out, AST_print_context& ctx) override;
    };


    /*
     * Abstract base class for nodes that have sequences
     * of children, e.g., block of statements, sequence of
     * classes.  These may be able to share some operations,
     * especially when applying a method to the sequence just
     * means applying the method to each element of the sequence.
     * We need a different kind of sequence depending on type of
     * elements if we want to access elements without casting while
     * still having access to their fields.
     * (Will replace 'Seq')
     */
    template<class Kind>
    class Seq : public ASTNode {
    protected:
        std::string kind_;
        std::vector<Kind *> elements_;
    public:
        Seq(std::string kind) : kind_{kind}, elements_{std::vector<Kind *>()} {}

        void gen_rvalue(GenContext *ctx, std::string target_reg){
            for (ASTNode *node: elements_){
                node->gen_rvalue(ctx, target_reg);
            }
        }
        std::string get_var() override {return "";}
        void get_vars() override {return "";}

        int init_check(set<std::string>* vars) override {
            for (ASTNode *node: elements_){
                if (node->init_check(vars)) {return 1;} 
            }
            return ;
        }

        std::string type_inference(semantics* stc, std::map<std::string, std::string>* vtable, methods* info) //override;
        {
            info->print()
            int return_val = 0;
            for (Kind *elm: elements_){
                if (elm->type_inference(stc, vtable, info)){return_val = 1;};
            return return_val; 
            }
        }

        void append(Kind *el) { elements_.push_back(el); }

        void json(std::ostream &out, AST_print_context &ctx) {
            json_head(kind_, out, ctx);
            out << "\"elements_\" : [";
            auto sep = "";
            for (Kind *elm: elements_) {
                out << sep;
                elm->json(out, ctx);
                sep = ", ";
            }
            out << "]";
            json_close(out, ctx);
        }

    };

    



    /* L_Expr nodes are AST nodes that can be evaluated for location.
     * Most can also be evaluated for value_.  An example of an L_Expr
     * is an identifier, which can appear on the left_ hand or right_ hand
     * side of an assignment.  For example, in x = y, x is evaluated for
     * location and y is evaluated for value_.
     *
     * For now, a location is just a name, because that's what we index
     * the symbol table with.  In a full compiler, locations can be
     * more complex, and typically in code generation we would have
     * LExpr evaluate to an address in a register.
     */
    class LExpr : public ASTNode {
        /* Abstract base class */
    };


    /* Identifiers like x and literals like 42 are the
    * leaves of the AST.  A literal can only be evaluated
    * for value_ (the 'eval' method), but an identifier
    * can also be evaluated for location (when we want to
    * store something in it).
    */
    class Ident : public LExpr {
        std::string text_;
    public:
        std::string gen_lvalue(GenContext *ctx) override {
            return ctx->get_local_var(text_)
        }

        void gen_rvlaue(GenContext *ctx, std::string target_reg) override {
            std::string loc = ctx->get_local_var(text_);
            ctx->emit(target_reg + " = " + loc + ';');
        }


        std::string get_var() override {return text_;}
        void get_vars(std::map<std::string, std::string>* vtable) override {return;}
        std::int init_check(std::set<std::string>* vars) override{
            if (!vars -> count(text_)){
                cout << "InitError: Identifier" << text_ << "not initialized" << std::endl;
                return 1;
            }
            return 0;
        }

        std::string type_inference(semantics* stc, std::map<std::string, std::string>* vtable, class_and_method* info);

        explicit Ident(std::string txt) : text_{txt} {}
        void json(std::ostream& out, AST_print_context& ctx) override;
    };


    /* A block is a sequence of statements or expressions.
     * For simplicity we'll just make it a sequence of ASTNode,
     * and leave it to the parser to build valid structures.
     */
    class Block : public Seq<ASTNode> {
    public:
        explicit Block() : Seq("Block") {}
    };

    /* Formal arguments list is a list of
     * identifier: type pairs.
     */
    class Formal : public ASTNode {
        ASTNode& var_;
        ASTNode& type_;
    public:
        explicit Formal(ASTNode& var, ASTNode& type_) :
            var_{var}, type_{type_} {};
        
        std::string type_inference(semantics* stc, std::map<std::string, std::string>* vtable, methods* info) override {
            std::string var = var_.get_var()
            std::cout << typeid(type_).name() << std::endl;
            std::string type = type_.get_var();
            (*vtable)[var] = type;
            return type;
        }
        int init_check(std::set<std::string>* vars) override{
            vars -> insert(var_.get_var());
            return 0;
        }

        std::string get_var(){ return "";}
        void get_vars(std::map<std::string,std::string>* vtable) override {return;}
        void json(std::ostream& out, AST_print_context&ctx) override;
    };

    class Formals : public Seq<Formal> {
    public:
        explicit Formals() : Seq("Formals") {}
    };

    class Method : public ASTNode {
        ASTNode& name_;
        Formals& formals_;
        ASTNode& returns_;
        Block& statements_;
    public:
        explicit Method(ASTNode& name, Formals& formals, ASTNode& returns, Block& statements) :
          name_{name}, formals_{formals}, returns_{returns}, statements_{statements} {}
        
        std::string type_inference(semantics* stc, std::map<std::string,std::string>* vtable, class_and_method* info) override{
            std::string formal_return_val = formals_.type_inference(stc, vtable, info);
            std::string statement_return_val = statements_.type_inference(stc, vtable, info);
            return "";
        }

        int init_check(std::set<std::string>* vars) override {
            if (formals_.init_check(vars)) {return 1;}
            if (statements_.init_check(vars)) {return 1;}
            return 0; // zero for success
        }


        std::string get_var() override {return "";}
        void get_vars(std::map<std::string, std::string>* vtable) override {return;}
        void json(std::ostream& out, AST_print_context&ctx) override;
    };

    class Methods : public Seq<Method> {
    public:
        explicit Methods() : Seq("Methods") {}
        std::string type_inference(semantics* stc, std::map<std::string, std::string>* vtable, class_and_method* info) override;
    };




    /* An assignment has an lvalue (location to be assigned to)
     * and an expression.  We evaluate the expression and place
     * the value_ in the variable.  An assignment may also place a
     * static type constraint on a variable.  This is logically a
     * distinct operation, and could be represented as a separate statement,
     * but it's convenient to keep it in the assignment since our syntax
     * puts it there.
     */

    class Statement : public ASTNode { };

    class Assign : public Statement {
    protected:  // But inherited by AssignDeclare
        ASTNode &lexpr_;
        ASTNode &rexpr_;
    public:
        void gen_rvalue(GenContext *ctx, std::string target_reg) override {
            std::string type = ctx->get_type(lexpr_);
            std::string reg = ctx->alloc_reg(type);
            std::string loc = lexpr_.gen_lvalue(ctx);
            rexpr_.gen_rvalue(ctx, reg);
            ctx->emit(loc + " = " + reg + ";");
        }

        void get_vars(map<std::string,std::string>* vtable) override {
            std::string var_name = lexpr_.get_var();
            if (var_name.rfind("this",0)==0){
                (*vtable)[var_name] = "Bottom";
            }
        }

        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override;
        int init_check(set <string>* vars) override{
            if (rexpr_.inie_check(vars)){
                return 1;
            }
            vars->insert(lexpr_.get_var());
            return 0;
        }


        explicit Assign(ASTNode &lexpr, ASTNode &rexpr) :
           lexpr_{lexpr}, rexpr_{rexpr} {}
        void json(std::ostream& out, AST_print_context& ctx) override;
    };



    class AssignDeclare : public Assign {
        Ident &static_type_;
    public:
        explicit AssignDeclare(ASTNode &lexpr, ASTNode &rexpr, Ident &static_type) :
            Assign(lexpr, rexpr), static_type_{static_type} {}

        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override;
        void json(std::ostream& out, AST_print_context& ctx) override;

    };



    /* A statement could be just an expression ... but
     * we might want to interpose a node here.
     */
    class Expr : public Statement { };

    /* When an expression is an LExpr, we
     * the LExpr denotes a location, and we
     * need to load it.
     */
    class Load : public Expr {
        LExpr &loc_;
    public:
        Load(LExpr &loc) : loc_{loc} {}

        void gen_rvalue(GenContext *ctx, std::string target_reg) override{
            std::string var = get_var();
            std::string loc = ctx->get_local_var(var);
            ctx->emit(target_reg + " = " + loc + ";");
        }

        void get_lvalue(GenContext* ctx, std::string target_reg) override{
            std::string var = get_var();
            return ctx->get_local_var();
        }

        std::string get_var() override {return loc_.get_var();}
        void get_vars(map<string,string>* vtable) override {return;}


        int init_check(set<string>* vars) override {return 0;}
        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override {
            return loc_.type_inference(stc, vtable, info);
        }

        void json(std::ostream &out, AST_print_context &ctx) override;
    };



    /* 'return' statement returns value from method */
    class Return : public Statement {
        ASTNode &expr_;
    public:
        Load(LExpr *loc): loc_{loc} {}
        
        explicit Return(ASTNode& expr) : expr_{expr}  {}

        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override;
        int init_check(set<string>* vars) override{
            if (expr_.init_check(vars)) {return 1;}
            return 0;
        }

        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class If : public Statement {
        ASTNode &cond_; // The boolean expression to be evaluated
        Seq<ASTNode> &truepart_; // Execute this block if the condition is true
        Seq<ASTNode> &falsepart_; // Execute this block if the condition is false
    public:

        void gen_rvalue(GenContext *ctx, string target_reg) override {
                std::string then_part = ctx->new_branch_label("then");
                std::string else_part = ctx->new_branch_label("else");
                std::string end_part = ctx->new_branch_label("endif");
                cond_.gen_branch(ctx, then_part, else_part);
                ctx->emit(then_part + ":;");
                truepart_.gen_rvalue(ctx, target_reg);
                ctx->emit(std::string(":goto ") + end_part + ";");
                ctx->emit(else_part + ": ;");
                falsepart_.gen_rvalue(ctx,target_reg);
                ctx->emit(end_part + ":;");
        };


        explicit If(ASTNode& cond, Seq<ASTNode>& truepart, Seq<ASTNode>& falsepart) :
            cond_{cond}, truepart_{truepart}, falsepart_{falsepart} { };

        int init_check(set<string>* vars) override{
            if (cond_.init_check(vars)) {return 1;}
            set<string>* true_set = new set<string>(*vars);
            set<string>* false_set = new set<string>(*vars);

            if (truepart_.init_check(true_set) || falsepart_.init_check(false_set)) {return 1;}
            // if (falsepart_.init_check(false_set)) {return 1;}
            for (set<string>::iterator iter=true_set->begin(); iter!=true_set->end(); iter++){
                if (false_set->count(*iter)) {
                    vars->insert(*iter);
                }
            }
            return 0;
        }
        
        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override;

        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class While : public Statement {
        ASTNode& cond_;  // Loop while this condition is true
        Seq<ASTNode>&  body_;     // Loop body
    public:
        explicit While(ASTNode& cond, Block& body) :
            cond_{cond}, body_{body} { };

        void gen_rvalue(GenContext* ctx, string target_reg) override{
            std::string check_part = ctx->new_branch_label("check_cond");
            std::string loop_part = ctx->new_branch_label("loop");
            std::string end_part = ctx->new_branch_label("endwhile");

            ctx->emit(check_part + ": ;");
            cond_.gen_branch(ctx, loop_part, end_part);
            ctx->emit(loop_part + ": ;");
            body_.gen_rvalue(ctx, target_reg);
            ctx->emit("goto " + check_part + ";");
            ctx->emit(end_part + ": ;"); 
        }

        std::string type_inference(semantic* stc, map<string,string>* vtable, class_and_method* info) override {
            string cond_type = cond_.type_inference(stc, vtable, info);
            if (cond_type != "Boolean"){
                cout << "------- CONDITION NOT BOOLEAN TYPE ----------" << endl;
                return 0;
            }
            std::string inference_result = body_.type_interence(stc, vtable, info);
            return inference_result;
        }


        int init_check(set<string>* vars) override{
            if (cond_.init_check(vars)) {return 1;}
            set<string>* body_set = new set<string>(*vars);
            if (body_.init_check(body_set)) {return 1;}
            return 0;
        }


        void json(std::ostream& out, AST_print_context& ctx) override;

    };




    /* A class has a name, a list of arguments, and a body
    * consisting of a block (essentially the constructor)
    * and a list of methods.
    */
    class Class : public ASTNode {
        Ident& name_;
        Ident& super_;
        ASTNode& constructor_;
        Methods& methods_;
    public:
        void gen_rvalue(GenContext* ctx, std::string target_reg) override {
            std::string class_name = name_.get_var();
            ctx->emit("struct class_" + class_name + "_struct;");
            ctx->emit("typedef struct class_" + class_name + "_struct* class_" + class_name + ";");
            ctx->emit("");
            ctx->emit("typedef struct obj_" + class_name + "_struct {");
            ctx->emit("\tclass_" + class_name + " clazz;");
            ctx->emit_instance_vars();
            ctx->emit("} * obj_" + class_name + ";");
            ctx->emit("");
            ctx->emit("struct class_" + class_name + "_struct {");
            ctx->emit("obj_" + class_name + "(*constructor) (" + ctx->get_formal_argtypes("constructor") + ");");
            ctx->emit("};");
            ctx->emit("extern class_" + class_name + " the_class_" + classname + ";");
        }



        explicit Class(Ident& name, Ident& super,
                 ASTNode& constructor, Methods& methods) :
            name_{name},  super_{super},
            constructor_{constructor}, methods_{methods} {};
        std::string get_var() override {
            return "";
        }
        void get_vars(map<string, string>* vtable) override {return;}

        std::string type_inference(semantics* stc, map<string,string> vtable, class_and_method* info) override;
        int init_check(set<string>* vars) override{
            if (constructor_.init_check(vars) || methods_.init_check(vars)) {
                return 1;
            }
            return 0;
        }

        void json(std::ostream& out, AST_print_context& ctx) override;
        void get_var() override {return "";}
        void get_vars(std::map<std::string, std::string>* vtable) override {return;}
        
    };

    /* A Quack program begins with a sequence of zero or more
     * class definitions.
     */
    class Classes : public Seq<Class> {
    public:
        explicit Classes() : Seq<Class>("Classes") {}
        void gen_rvlaue(GenContext* ctx, std::string target_reg) override{
            for (Class *cls: elements_){
                std::string class_name = cls->name_.get_var();
                Context class_con = Context(*con);
                class_con.class_name = class_name;
                cls->gen_rvalue(&class_con, target_reg);
            }
        }

        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override;
    };



    class IntConst : public Expr {
        int value_;
    public:
        explicit IntConst(int v) : value_{v} {}
        void json(std::ostream& out, AST_print_context& ctx) override;

        void gen_rvalue(GenContext* ctx, std::string target_reg) override{
            ctx->emit(target_ret + " = int_literal(" + to_string(value_) + ");");
        }


        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override {return 0;}

        int init_check(set<string>* vars) override {return 0;}

    };

    class Type_Alternative : public ASTNode {
        Ident& ident_;
        Ident& classname_;
        Block& block_;
    public:
        explicit Type_Alternative(Ident& ident, Ident& classname, Block& block) :
                ident_{ident}, classname_{classname}, block_{block} {}
        void json(std::ostream& out, AST_print_context& ctx) override;

        std::string get_var() override {return "";}
        void get_vars(map<string,string>* vtable) override {return;}

        std::string type_inference(semantics* stc, map<string,string> vtable, class_and_method* info) override; 
    };

    class Type_Alternatives : public Seq<Type_Alternative> {
    public:
        explicit Type_Alternatives() : Seq("Type_Alternatives") {}
    };

    class Typecase : public Statement {
        Expr& expr_; // An expression we want to downcast to a more specific class
        Type_Alternatives& cases_;    // A case for each potential type
    public:
        explicit Typecase(Expr& expr, Type_Alternatives& cases) :
                expr_{expr}, cases_{cases} {};
        void json(std::ostream& out, AST_print_context& ctx) override;
        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override;
    };


    class StrConst : public Expr {
        std::string value_;
    public:
        explicit StrConst(std::string v) : value_{v} {}
        void json(std::ostream& out, AST_print_context& ctx) override;

        void gen_rvalue(GenContext* ctx, std::string target_reg) override{
            ctx->emit(target_reg + " = str_literal(" + value_ + ");");
        }

        int init_check(set<string>* vars) override {return 0;}
    };

    class Actuals : public Seq<Expr> {
    public:
        explicit Actuals() : Seq("Actuals") {}
        std::string gen_lvalue(GenContext* ctx) override{
            std::vector<string> actual_regs =vector<string>();
            for (ASTNode* actual: elements_){
                string type = ctx->get_type(*actual);
                string reg = ctx->alloc_reg(type);
                actual_regs.push_back(reg);
                actual->gen_rvalue(ctx, reg);
            }
            string actuals = "";
            for (string reg: actual_regs){
                actuals += reg;
                actuals += ", ";
            }
            int string_len = actuals.length();
            actuals = actuals.erase(string_len - 2, 2);
            return actuals;
        } 
    };


    /* Constructors are different from other method calls. They
      * are static (not looked up in the vtable), have no receiver
      * object, and have their own type-checking rules.
      */
    class Construct : public Expr {
        Ident&  method_;           /* Method name is same as class name */
        Actuals& actuals_;    /* Actual arguments to constructor */
    public:
        explicit Construct(Ident& method, Actuals& actuals) :
                method_{method}, actuals_{actuals} {}

        void json(std::ostream& out, AST_print_context& ctx) override;

        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override;

        int init_check(set<string>* vars) override{
            if (method_.init_check(vars) || actuals_.init_check(vars)){
                return 1;
            }
            return 0;
        }
    };


    /* Method calls are central to type checking and code
     * generation ... and for us, the operators +, -, etc
     * are method calls to specially named methods.
     */
    class Call : public Expr {
        Expr& receiver_;        /* Expression computing the receiver object */
        Ident& method_;         /* Identifier of the method */
        Actuals& actuals_;     /* List of actual arguments */
    public:
        explicit Call(Expr& receiver, Ident& method, Actuals& actuals) :
                receiver_{receiver}, method_{method}, actuals_{actuals} {};

        // Convenience factory for the special case of a method
        // created for a binary operator (+, -, etc).
        static Call* binop(std::string opname, Expr& receiver, Expr& arg);
        void json(std::ostream& out, AST_print_context& ctx) override;


        void gen_branch(GenContext* ctx, string true_branch, string false_branch) override {
            string var_type = ctx->get_type(*this);
            string reg = ctx->alloc_reg(var_type);
            gen_rvalue(ctx, reg);
            ctx->emit(string("if (") + reg + ") goto" + true_branch + ";");
            ctx->emit(string("goto ") + false_branch + ";");
            ctx->free_reg(reg);
        }

        void gen_rvalue(GenContext* ctx, std::string target_reg){
            string method_name = method_.get_var();
            string receiver_type = ctx->get_type(receiver_);
            string receiver_reg = ctx->alloc_reg(receiver_type);
            receiver_.gen_rvalue(ctx, receiver_reg);
            string actuals = actuals_.gen_lvalue(ctx);
            ctx->emit(target_reg + " = " + receiver_reg + "->clazz->" + method_name + "(" + receiver_reg + ", " + actuals + ");");
        }

        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override;
        int init_check(set<string>* vars) override {
            if (receiver_.init_check(vars) || method_.init_check(vars) || actuals_.init_check(vars)){
                return 1;
            }
            return 0;
        }
    };


    // Virtual base class for binary operations.
    // Does NOT include +, -, *, /, etc, which
    // are method calls.
    // Does include And, Or, Dot, ...
   class BinOp : public Expr {
    protected:
        std::string opsym;
        ASTNode &left_;
        ASTNode &right_;
        BinOp(std::string sym, ASTNode &l, ASTNode &r) :
                opsym{sym}, left_{l}, right_{r} {};
    public:
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

   class And : public BinOp {
   public:
       explicit And(ASTNode& left, ASTNode& right) :
          BinOp("And", left, right) {}

        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override{
            std::string left_type =left_.type_inference(stc, vtable, info);
            std::string reght_type = reight_.type_inference(stc, vtable, info);
            if(left_type != "Booleen" || right_type != "Boolean"){
                return "TypeError: Not Boolean";
            }
            return "Boolean";
        }

        int init_check(set<string>* vars) override {
            if (left_.init_check(vars) || right_.init_check(vars)){
                return 1;
            }
            return 0;
        }
   };

    class Or : public BinOp {
    public:
        explicit Or(ASTNode& left, ASTNode& right) :
                BinOp("Or", left, right) {}

        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override{
            std::string left_type =left_.type_inference(stc, vtable, info);
            std::string reght_type = reight_.type_inference(stc, vtable, info);
            if(left_type != "Booleen" || right_type != "Boolean"){
                return "TypeError: Not Boolean";
            }
            return "Boolean";
        }

        int init_check(set<string>* vars) override {
            if (left_.init_check(vars) || right_.init_check(vars)){
                return 1;
            }
            return 0;
        }
    };

    class Not : public Expr {
        ASTNode& left_;
    public:
        explicit Not(ASTNode& left ):
            left_{left}  {}
        // std::string get_type(std::map<std::string,std::string>* vtable, semantics *stc, std::string class_name) override {
        //     return "Booleen";
        // }
        void json(std::ostream& out, AST_print_context& ctx) override;


        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override{
            std::string left_type = left_.type_inference(stc, vtable, info);
            if (left_type != "Boolean") {
                return "TypeError: Not Boolean";
            }
            return "Boolean";
        }

        int init_check(set<string>* vars) override {
            if (left_.init_check(vars)){
                return 1;
            }
            return 0;
        }
    };


    /* Can a field de-reference (expr . IDENT) be a binary
     * operation?  It can be evaluated to a location (l_exp),
     * whereas an operation like * and + cannot, but maybe that's
     * ok.  We'll tentatively group it with Binop and consider
     * changing it later if we need to make the distinction.
     */

    class Dot : public LExpr {
        Expr& left_;
        Ident& right_;
    public:
        explicit Dot (Expr& left, Ident& right) :
           left_{left},  right_{right} {}
        void json(std::ostream& out, AST_print_context& ctx) override;


        std::string get_var() override {
            return left_.get_var() + "." + right_get_var();
        }

        void get_vars(map<string,string>* vtable) override {return ;}


        int type_inferene(semantics* stc, map<string,string>* vtable, class_and_method* info) override {
            left_.type_inference(stc, vtable, info);
            right_.type_inference(stc, vtable, info);
            return 0;
        }

        int init_check(set<string>* vars) override {
            if (!vars->count(get_var())){
                cout << "-------- INIT ERROR: var" << get_var() << " USED BEFORE INITIALIZED ---------" << endl;
                return 1;
            }
            return 0;
        }

    };


    /* A program has a set of classes (in any order) and a block of
     * statements.
     */
    class Program : public ASTNode {
    public:
        Classes& classes_;
        Block& statements_;
        explicit Program(Classes& classes, Block& statements) :
                classes_{classes}, statements_{statements} {}

        std::string get_var() override {return ""}
        std::get_vars(std::map<std::string,std::string>* vtable) override {return;}
        void json(std::ostream& out, AST_print_context& ctx) override;


        void gen_rvalue(GenContext* ctx, string target_reg) override{
            classes_.get_rvalue(ctx, target_reg);
            GenContext class_ctx = GenContext(*ctx);
            class_ctx.class_name = "PGM";
            class_ctx.method_name = "PGM";
            statements_.get_rvalue(&class_ctx, target_reg);
        }

        std::string type_inference(semantics* stc, map<string,string>* vtable, class_and_method* info) override;
        string get_var() override {return "";}
        void get_vars(map<string,string>* vtable) override {return;}
        int init_check(semantics* stc, set<string>* vars) override;
    };



}
#endif //ASTNODE_H
