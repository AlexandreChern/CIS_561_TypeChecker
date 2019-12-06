//
// Started by Michal Young on 9/12/18.

//
#ifndef ASTNODE_H
#define ASTNODE_H

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <typeinfo>
#include <map>
#include <set>
#include "CodegenContext.h"
#include "semantics.h"

using namespace std;



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
        virtual std::string gen_lvalue(GenContext *ctx) {cout << "gen_lvalue UNIMP" << endl; return "";}
        virtual void gen_rvalue(GenContext *ctx, std::string target_reg) {cout << "gen_rvalue UNIMPLLLL" << endl;}
        virtual void gen_branch(GenContext *ctx, std::string true_branch, std::string false_branch) { cout << "gen_branch UNIMP" << endl; }
        virtual void get_vars(map<std::string, std::string>* v_table) {cout << "UNIMPLEMENTED get_vars" << endl;};
        virtual std::string get_var() {cout << "UNIMPLEMENTED GET_VAR" << endl; return "";};
        virtual int init_check(set<std::string>* vars) {cout << "UNIMPLEMENTED init_check" << endl; return 0;}
        virtual std::string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) {
            cout << "UNIMPLEMENTED type_inference" << endl;
            return "UNIMP type_inference";
        }
        virtual void json(ostream& out, AST_print_context& ctx) = 0;  // Json string representation
        std::string str() {
            stringstream ss;
            AST_print_context ctx;
            json(ss, ctx);
            return ss.str();
        }
    protected:
        void json_indent(ostream& out, AST_print_context& ctx);
        void json_head(std::string node_kind, ostream& out, AST_print_context& ctx);
        void json_close(ostream& out, AST_print_context& ctx);
        void json_child(string field, ASTNode& child, ostream& out, AST_print_context& ctx, char sep=',');
    };

    class Stub : public ASTNode {
        std::string name_;
    public:
        std::string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override {
            cout << "UNIMP TYPEINFER STUB" << endl;
            return 0;
        }
        explicit Stub(string name) : name_{name} {}
        void json(ostream& out, AST_print_context& ctx) override;
    };

    template<class Kind>
    class Seq : public ASTNode {
    public:
        std::string kind_;
        std::vector<Kind *> elements_;

        Seq(std::string kind) : kind_{kind}, elements_{vector<Kind *>()} {}

        void gen_rvalue(GenContext *ctx, std::string target_reg) override {
            for (ASTNode *node: elements_) {
                node->gen_rvalue(ctx, target_reg);
            }
        }
        std::string get_var() override {return "";}
        void get_vars(map<std::string, std::string>* v_table) override {return;}
        int init_check(set<std::string>* vars) override {
            for (ASTNode *node: elements_) {
                if (node->init_check(vars)) { return 1; } // failure
            }
            return 0;
        }
        std::string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override {
            for (Kind *el: elements_) {el->type_inference(stc, v_table, info);}
            return "Nothing";
        };

        void append(Kind *el) { elements_.push_back(el); }

        void json(ostream &out, AST_print_context &ctx) override {
            json_head(kind_, out, ctx);
            out << "\"elements_\" : [";
            auto sep = "";
            for (Kind *el: elements_) {
                out << sep;
                el->json(out, ctx);
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
        public:
            string text_;

            string gen_lvalue(GenContext *ctx) override {
                return ctx->get_local_var(text_);
            }
            void gen_rvalue(GenContext *ctx, string target_reg) override {
                /* The lvalue, i.e., address of memory */
                string loc = ctx->get_local_var(text_);
                ctx->emit(target_reg + " = " + loc + ";");
            }
            std::string get_var() override {return text_;}
            void get_vars(map<std::string, std::string>* v_table) override {return;}
            std::string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
            int init_check(set<string>* vars) override {
                if (!vars->count(text_)) { // not 0 would be 1, indicating failure
                    cout << "INIT ERROR: Used before initialized" << endl;
                    return 1;
                }
                return 0;
            }
            explicit Ident(string txt) : text_{txt} {}
            void json(ostream& out, AST_print_context& ctx) override;
    };

    class Block : public Seq<ASTNode> {
    public:
        explicit Block() : Seq("Block") {}
     };

    class Formal : public ASTNode {
        public:
            ASTNode& var_;
            ASTNode& type_;
            explicit Formal(ASTNode& var, ASTNode& type_) :
                var_{var}, type_{type_} {};
            std::string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override {
                //cout << "ENTERING Formal:type_inference" << endl;
                string var = var_.get_var();
                string type = type_.get_var();
                (*v_table)[var] = type;
                return type;
            }
            int init_check(set<std::string>* vars) override {
                vars->insert(var_.get_var());
                return 0;
            }
            std::string get_var() override {return "";}
            void get_vars(map<std::string, std::string>* v_table) override {return;}
            void json(ostream& out, AST_print_context&ctx) override;
    };

    class Formals : public Seq<Formal> {
    public:
        explicit Formals() : Seq("Formals") {}
    };

    class Method : public ASTNode {
        public:
            ASTNode& name_;
            Formals& formals_;
            ASTNode& returns_;
            Block& statements_;
            
            void gen_rvalue(GenContext *ctx, std::string target_reg) override;
            explicit Method(ASTNode& name, Formals& formals, ASTNode& returns, Block& statements) :
            name_{name}, formals_{formals}, returns_{returns}, statements_{statements} {}
            std::string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override {
                //cout << "ENTERING Method::type_inference for method " << name_.get_var() << endl;
                formals_.type_inference(stc, v_table, info);
                statements_.type_inference(stc, v_table, info);
                return "Nothing";
            }
            int init_check(set<std::string>* vars) override {
                if (formals_.init_check(vars) || statements_.init_check(vars)) { return 1; }
                return 0;
            }
            std::string get_var() override {return "";}
            void get_vars(map<std::string, std::string>* v_table) override {return;}
            void json(ostream& out, AST_print_context&ctx) override;
    };

    class Methods : public Seq<Method> {
    public:
        explicit Methods() : Seq("Methods") {}
        std::string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
    };

    class Statement : public ASTNode { 
    };

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
            /* Store the value in the location */
            ctx->emit(loc + " = " + reg + ";");
        }
        void get_vars(map<std::string, std::string>* v_table) override {
            std::string var_name = lexpr_.get_var();
            if (var_name.rfind("this", 0) == 0) {
                (*v_table)[var_name] = "Error";
            }
        }
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
        int init_check(set<std::string>* vars) override {
            if (rexpr_.init_check(vars)) { return 1; }
            vars->insert(lexpr_.get_var());
            return 0;
        }        
        explicit Assign(ASTNode &lexpr, ASTNode &rexpr) :
           lexpr_{lexpr}, rexpr_{rexpr} {}
        void json(ostream& out, AST_print_context& ctx) override;
    };

    class AssignDeclare : public Assign {
        Ident &static_type_;
    public:
        explicit AssignDeclare(ASTNode &lexpr, ASTNode &rexpr, Ident &static_type) :
            Assign(lexpr, rexpr), static_type_{static_type} {}
        std::string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
        void json(ostream& out, AST_print_context& ctx) override;

    };

    class Expr : public Statement { 
        public:
    };

    /* When an expression is an LExpr, the LExpr denotes a location, 
        and we need to load it. */
    class Load : public Expr {
        LExpr &loc_;
    public:
        Load(LExpr &loc) : loc_{loc} {}
        void gen_rvalue(GenContext *ctx, std::string target_reg) override {
            std::string var = get_var();
            std::string loc = ctx->get_local_var(var);
            ctx->emit(target_reg + " = " + loc + ";");
        }
        std::string gen_lvalue(GenContext *ctx) override {
            std::string var = get_var();
            return ctx->get_local_var(var);
        }
        std::string get_var() override {return loc_.get_var();}
        void get_vars(map<std::string, string>* v_table) override {return;}
        int init_check(set<string>* vars) override { return 0; }  
        std::string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override {
             return loc_.type_inference(stc, v_table, info); 
        }
        void json(ostream &out, AST_print_context &ctx) override;
    };

    class Return : public Statement {
        ASTNode &expr_;
    public:
        explicit Return(ASTNode& expr) : expr_{expr}  {}
        std::string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
        int init_check(set<string>* vars) override {
            if (expr_.init_check(vars)) { return 1; }
            return 0;
        }  
        void json(ostream& out, AST_print_context& ctx) override;
    };

    class If : public Statement {
        ASTNode &cond_; // The boolean expression to be evaluated
        Seq<ASTNode> &truepart_; // Execute this block if the condition is true
        Seq<ASTNode> &falsepart_; // Execute this block if the condition is false
    public:
        void gen_rvalue(GenContext* ctx, string target_reg) override {
            std::string then_statement = ctx->new_branch_label("then");
            std::string else_statement = ctx->new_branch_label("else");
            std::string end_statement = ctx->new_branch_label("endif");
            cond_.gen_branch(ctx, then_statement, else_statement);
            /* Generate the 'then' part here */
            ctx->emit(then_statement + ": ;");
            truepart_.gen_rvalue(ctx, target_reg);
            ctx->emit(std::string("goto ") + end_statement + ";");
            /* Generate the 'else' part here */
            ctx->emit(else_statement + ": ;");
            falsepart_.gen_rvalue(ctx, target_reg);
            /* That's all, folks */
            ctx->emit(end_statement + ": ;");
        }

        explicit If(ASTNode& cond, Seq<ASTNode>& truepart, Seq<ASTNode>& falsepart) :
            cond_{cond}, truepart_{truepart}, falsepart_{falsepart} { };
        int init_check(set<std::string>* vars) override {
            if (cond_.init_check(vars)) {return 1;}
            set<string>* trueset = new set<std::string>(*vars); // copy constructor
            set<string>* falseset = new set<std::string>(*vars); // copy constructor
            if (truepart_.init_check(trueset)) {return 1;}
            if (falsepart_.init_check(falseset)) {return 1;}
            // take set intersection
            for (set<std::string>::iterator iter = trueset->begin(); iter != trueset->end(); iter++) {
                if (falseset->count(*iter)) {
                    vars->insert(*iter); // if also in false part, insert to table (duplication ok, it's a set)
                }
            }
            return 0;
        }  
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;        
        void json(ostream& out, AST_print_context& ctx) override;
    };

    class While : public Statement {
        ASTNode& cond_;  // Loop while this condition is true
        Seq<ASTNode>&  body_;     // Loop body
    public:
        explicit While(ASTNode& cond, Block& body) :
            cond_{cond}, body_{body} { };

        void gen_rvalue(GenContext* ctx, std::string target_reg) override {
            string check_statement = ctx->new_branch_label("check_cond");
            string loop_statement = ctx->new_branch_label("loop");
            string end_statement = ctx->new_branch_label("endwhile");
            ctx->emit(check_statement + ": ;");
            cond_.gen_branch(ctx, loop_statement, end_statement);
            ctx->emit(loop_statement + ": ;");
            body_.gen_rvalue(ctx, target_reg);
            ctx->emit("goto " + check_statement + ";");
            ctx->emit(end_statement + ": ;");
        }

        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override {
            //cout << "ENTERING While::type_inference" << endl;
            string cond_type = cond_.type_inference(stc, v_table, info);
            if (cond_type != "Boolean") {
                cout << "TypeError (While): Condition does not evaluate to type Boolean (ignoring statements)" << endl;
            }
            body_.type_inference(stc, v_table, info);
            return "Nothing";
        }
        int init_check(set<std::string>* vars) override {
            if (cond_.init_check(vars)) {return 1;}
            set<string>* bodyset = new set<string>(*vars); // copy constructor
            if (body_.init_check(bodyset)) {return 1;}
            return 0;
        }  
        void json(ostream& out, AST_print_context& ctx) override;

    };

    class Class : public ASTNode {
        public:
            Ident& name_;
            Ident& super_;
            ASTNode& constructor_;
            Methods& methods_;

            void gen_rvalue(GenContext *ctx, std::string target_reg) override {
                string class_name = name_.get_var();
                ctx->emit("struct class_" + class_name + "_struct;");
                ctx->emit("struct obj_" + class_name + ";");
                ctx->emit("typedef struct obj_" + class_name + "* obj_" + class_name + ";");
                ctx->emit("typedef struct class_" + class_name + "_struct* class_" + class_name + ";");
                ctx->emit("");
                ctx->emit("typedef struct obj_" + class_name + "_struct {");
                ctx->emit("class_" + class_name + " clazz;");
                ctx->emit_instance_vars();
                ctx->emit("} * obj_" + class_name + ";");
                ctx->emit("");
                ctx->emit("struct class_" + class_name + "_struct the_class_" + class_name + "_struct;");
                ctx->emit("");
                ctx->emit("struct class_" + class_name + "_struct {");
                // constructor
                ctx->emit("obj_" + class_name + " (*constructor) (" + ctx->get_formal_argtypes("constructor") + ");");
                ctx->emit_method_signature(); // rest of the methods
                ctx->emit("};\n");
                ctx->emit("extern class_" + class_name + " the_class_" + class_name + ";");
                // now populate constructor
                ctx->emit("");
                GenContext * construct_ctx = new GenContext(*ctx);
                construct_ctx->method_name = "constructor";
                constructor_.gen_rvalue(construct_ctx, target_reg);
                methods_.gen_rvalue(ctx, target_reg);
                ctx->emit_class_struct();
            }

            explicit Class(Ident& name, Ident& super,
                    ASTNode& constructor, Methods& methods) :
                name_{name},  super_{super},
                constructor_{constructor}, methods_{methods} {};
            string get_var() override {return "";}
            void get_vars(map<std::string, std::string>* v_table) override {return;}
            string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
            int init_check(set<std::string>* vars) override {
                if (constructor_.init_check(vars)) {return 1;}
                if (methods_.init_check(vars)) {return 1;}
                return 0;
            }  
            void json(ostream& out, AST_print_context& ctx) override;
    };

    class Classes : public Seq<Class> {
    public:
        void gen_rvalue(GenContext *ctx, std::string target_reg) override {
            for (Class *cls: elements_) {
                string class_name = cls->name_.get_var();
                GenContext class_ctx = GenContext(*ctx); // copy constructor
                class_ctx.class_name = class_name;
                cls->gen_rvalue(&class_ctx, target_reg);
            }
        }
        explicit Classes() : Seq<Class>("Classes") {}
        std::string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
    };

    class IntConst : public Expr {
        int value_;
    public:
        void gen_rvalue(GenContext *ctx, std::string target_reg) override {
            ctx->emit(target_reg + " = int_literal(" + to_string(value_) + ");");
        }
        explicit IntConst(int v) : value_{v} {}
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override { return "Int"; }
        int init_check(set<std::string>* vars) override {return 0;};
        void json(ostream& out, AST_print_context& ctx) override;
    };

    class Type_Alternative : public ASTNode {
        Ident& ident_;
        Ident& class_name_;
        Block& block_;
    public:
        explicit Type_Alternative(Ident& ident, Ident& class_name, Block& block) :
                ident_{ident}, class_name_{class_name}, block_{block} {}
        string get_var() override {return "";}
        void get_vars(map<std::string, std::string>* v_table) override {return;}
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
        void json(ostream& out, AST_print_context& ctx) override;
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
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
        void json(ostream& out, AST_print_context& ctx) override;
    };

    class StrConst : public Expr {
        string value_;
    public:
        void gen_rvalue(GenContext *ctx, string target_reg) override {
            ctx->emit(target_reg + " = str_literal(" + value_ + ");");
        }
        explicit StrConst(string v) : value_{v} {}
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override { return "String"; }
        int init_check(set<string>* vars) override {return 0;};
        void json(ostream& out, AST_print_context& ctx) override;
    };

    class Actuals : public Seq<Expr> {
    public:
        explicit Actuals() : Seq("Actuals") {}
        string gen_lvalue(GenContext *ctx) override {
            vector<string> actualregs = vector<string>();
            for (ASTNode *actual: elements_) {
                string type = ctx->get_type(*actual);
                string reg = ctx->alloc_reg(type);
                actualregs.push_back(reg);
                actual->gen_rvalue(ctx, reg);
            }
            string actuals = "";
            for (string reg: actualregs) {
                actuals += reg;
                actuals += ", ";
            }
            int strlen = actuals.length();
            actuals = actuals.erase(strlen - 2, 2); // erase the final ", "
            return actuals;
        }
    };

    class Construct : public Expr {
        Ident&  method_;           /* Method name is same as class name */
        Actuals& actuals_;    /* Actual arguments to constructor */
    public:
        explicit Construct(Ident& method, Actuals& actuals) :
                method_{method}, actuals_{actuals} {}
        int init_check(set<string>* vars) override {
            if (method_.init_check(vars)) { return 1;}
            if (actuals_.init_check(vars)) { return 1;}
            return 0;
        }
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
        void json(ostream& out, AST_print_context& ctx) override;
    };

    class Call : public Expr {
        Expr& receiver_;        /* Expression computing the receiver object */
        Ident& method_;         /* Identifier of the method */
        Actuals& actuals_;     /* List of actual arguments */
    public:
        void gen_branch(GenContext *ctx, string true_branch, string false_branch) override {
            // At present, we don't have 'and' and 'or'
            string mytype = ctx->get_type(*this);
            string reg = ctx->alloc_reg(mytype);
            gen_rvalue(ctx, reg);
            ctx->emit(string("if (") + reg + ") goto " + true_branch + ";");
            ctx->emit(string("goto ") + false_branch + ";");
            ctx->free_reg(reg);
        }
        void gen_rvalue(GenContext *ctx, string target_reg) override {
            //obj_Int x_sum = this_x->clazz->PLUS(this_x, other_x); 
            // names of actual arguments?
                // what if actual arguments are themselves expressions?
            string method_name = method_.get_var();
            string recv_tableype = ctx->get_type(receiver_);
            string recvreg = ctx->alloc_reg(recv_tableype);
            receiver_.gen_rvalue(ctx, recvreg);
            string actuals = actuals_.gen_lvalue(ctx);
            ctx->emit(target_reg + " = " + recvreg + "->clazz->" + method_name + "(" + recvreg + ", " + actuals + ");");
        }

        explicit Call(Expr& receiver, Ident& method, Actuals& actuals) :
                receiver_{receiver}, method_{method}, actuals_{actuals} {};
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
        int init_check(set<string>* vars) override {
            if (receiver_.init_check(vars)) { return 1;}
            if (method_.init_check(vars)) { return 1;}
            if (actuals_.init_check(vars)) { return 1;}
            return 0;
        }
        // Convenience factory for the special case of a method
        // created for a binary operator (+, -, etc).
        static Call* binop(string opname, Expr& receiver, Expr& arg);
        void json(ostream& out, AST_print_context& ctx) override;
    };

   class BinOp : public Expr {
    protected:
        string opsym;
        ASTNode &left_;
        ASTNode &right_;
        BinOp(string sym, ASTNode &l, ASTNode &r) :
                opsym{sym}, left_{l}, right_{r} {};
    public:
        void json(ostream& out, AST_print_context& ctx) override;
    };

   class And : public BinOp {
   public:
       explicit And(ASTNode& left, ASTNode& right) :
          BinOp("And", left, right) {}
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override {
            string left_type = left_.type_inference(stc, v_table, info);
            string right_type = right_.type_inference(stc, v_table, info);
            if (left_type != "Boolean" || right_type != "Boolean") {return "And:TypeError";}
            return "Boolean";
        }
        int init_check(set<string>* vars) override {
            if (left_.init_check(vars)) { return 1;}
            if (right_.init_check(vars)) { return 1;}
            return 0;
        }
   };

    class Or : public BinOp {
    public:
        explicit Or(ASTNode& left, ASTNode& right) :
                BinOp("Or", left, right) {}
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override {
            string left_type = left_.type_inference(stc, v_table, info);
            string right_type = right_.type_inference(stc, v_table, info);
            if (left_type != "Boolean" || right_type != "Boolean") {return "Or:TypeError";}
            return "Boolean";
        }
        int init_check(set<string>* vars) override {
            if (left_.init_check(vars)) { return 1;}
            if (right_.init_check(vars)) { return 1;}
            return 0;
        }
    };

    class Not : public Expr {
        ASTNode& left_;
    public:
        explicit Not(ASTNode& left ):
            left_{left}  {}
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override {
            string left_type = left_.type_inference(stc, v_table, info);
            if (left_type != "Boolean") {return "Not:TypeError";}
            return "Boolean";
        }
        int init_check(set<string>* vars) override {
            if (left_.init_check(vars)) { return 1;}
            return 0;
        }
        void json(ostream& out, AST_print_context& ctx) override;
    };

    class Dot : public LExpr {
        Expr& left_;
        Ident& right_;
    public:
        void gen_rvalue(GenContext *ctx, string target_reg) override {
            string var = get_var();
            string loc = ctx->get_local_var(var);
            ctx->emit(target_reg + " = " + loc + ";");
        }
        string gen_lvalue(GenContext *ctx) override {
            string var = get_var();
            return ctx->get_local_var(var);
        }
        string get_var() override {return left_.get_var() + "." + right_.get_var();}
        void get_vars(map<std::string, std::string>* v_table) override { return; }
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
        int init_check(set<string>* vars) override {
            if (!vars->count(get_var())) { // not 0 would be 1, indicating failure
                cout << "INIT ERROR: Used before initialized" << endl;
                return 1;
            } 
            return 0;
        }
        explicit Dot (Expr& left, Ident& right) :
           left_{left},  right_{right} {}
        void json(ostream& out, AST_print_context& ctx) override;
    };

    class Program : public ASTNode {
    public:
        Classes& classes_;
        Block& statements_;

        void gen_rvalue(GenContext *ctx, string target_reg) override {
            classes_.gen_rvalue(ctx, target_reg);
            GenContext class_ctx = GenContext(*ctx); // copy constructor
            class_ctx.class_name = "PGM";
            class_ctx.method_name = "PGM";
            statements_.gen_rvalue(&class_ctx, target_reg);
        }
        explicit Program(Classes& classes, Block& statements) :
                classes_{classes}, statements_{statements} {}
        string type_inference(semantics* stc, map<std::string, std::string>* v_table, class_and_method* info) override;
        string get_var() override {return "";}
        int init_check(set<string>* vars, semantics* stc);
        void get_vars(map<std::string, std::string>* v_table) override {return;}
        void json(ostream& out, AST_print_context& ctx) override;
    };
}
#endif //ASTNODE_H