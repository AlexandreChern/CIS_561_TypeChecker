// //
// // The parser driver just glues together a parser object
// // and a lexer object.
// //

#include "lex.yy.h"
#include "ASTNode.h"
#include "Messages.h"
#include <unistd.h>  
#include "semantics.cxx"
#include "semantics.h"
#include "CodegenContext.h"
#include <iostream>
#include <fstream>



class Driver {
    int debug_level = 0;
public:
    explicit Driver(const reflex::Input in) : lexer(in), parser(new yy::parser(lexer, &root)) { root = nullptr; }

    ~Driver() { delete parser; }

    void debug() { debug_level = 1; }

    AST::ASTNode *parse() {
        parser->set_debug_level(debug_level); // 0 = no debugging, 1 = full tracing
        // std::cout << "Running parser\n";
        int result = parser->parse();
        if (result == 0 && report::ok()) {  // 0 == success, 1 == failure
            // std::cout << "Extracting result\n";
            if (root == nullptr) {
                std::cout << "But I got a null result!  How?!\n";
            }
            return root;
        } else {
            // std::cout << "Parse failed, no tree\n";
            return nullptr;
        }
    }

private:
    yy::Lexer lexer;
    yy::parser *parser;
    AST::ASTNode *root;
};

void generate_code(AST::ASTNode *root, semantics* stc) {
    AST::Program *AST_init_root = (AST::Program*) root;
    ofstream outfile;
    outfile.open("quack_output.c");
    GenContext ctx(outfile, stc, "", "");
    // Header
    ctx.emit("#include <stdio.h>");
    ctx.emit("#include \"ASTBuiltins.h\"");

    // Function
    ctx.emit("int main(int argc, char **argv) {");
    std::string target = ctx.alloc_reg("Obj");
    AST_init_root->gen_rvalue(&ctx, target);
  
    ctx.emit("}");
    outfile.close();
}

int main(int argc, char **argv) {
    // std::string filename;
    char c;
    FILE *filename;
    int index;
    int debug; // 0 = no debugging, 1 = full tracing

    while ((c = getopt(argc, argv, "t")) != -1) {
        if (c == 't') {
            std::cerr <<  "Debugging mode\n";
            debug = 1;
        }
    }

    for (index = optind; index < argc; ++index) {
        // if( !(filename = fopen(argv[index], "r"))) {
        //     perror(argv[index]);
        //     exit(1);
        // }
        filename = fopen(argv[index],"r");
        Driver driver(filename);
        if (debug) driver.debug();
        AST::ASTNode *root = driver.parse();
        if (root != nullptr) {
            // std::cout << "Parsed!\n";
            AST::AST_print_context context;
            root->json(std::cout, context);
            std::cout << std::endl;
            // STATIC SEMANTIC CHECK ON TREE
            // return (or null pointer if error)
            semantics semantic_check(root);
            semantic_check.AST_check();
            AST::Program *AST_init_root = (AST::Program*) root;
            generate_code(AST_init_root, &semantic_check);
        } else {
            std::cout << "No tree produced." << std::endl;
        }
    }

}
