#include <stdio.h>
#include "ASTBuiltins.h"
int main(int argc, char **argv) {
obj_Obj tmp__0;
struct class_BadInit_struct;
struct obj_BadInit;
typedef struct obj_BadInit* obj_BadInit;
typedef struct class_BadInit_struct* class_BadInit;

typedef struct obj_BadInit_struct {
class_BadInit clazz;
} * obj_BadInit;

struct class_BadInit_struct the_class_BadInit_struct;

struct class_BadInit_struct {
obj_BadInit (*constructor) ();
};

extern class_BadInit the_class_BadInit;


obj_BadInit new_BadInit() {
obj_BadInit new_obj = (obj_BadInit) malloc(sizeof(struct obj_BadInit_struct));
new_obj->clazz = the_class_BadInit;
}


obj_Nothing BadInit_method_bad_init() {
obj_TypeError tmp__1;
obj_ var_rep;
tmp__1 = int_literal(0);
var_rep = tmp__1;
obj_TypeError tmp__2;
obj_ var_n_reps;
tmp__2 = int_literal(3);
var_n_reps = tmp__2;
check_cond_1: ;
obj_ tmp__3;
obj_ tmp__4;
tmp__4 = var_rep;
obj_ tmp__5;
tmp__5 = var_n_reps;
tmp__3 = tmp__4->clazz-><(tmp__4, tmp__5);
if (tmp__3) goto loop_2;
goto endwhile_3;
// free register tmp__3
loop_2: ;
obj_ tmp__6;
obj_ tmp__7;
tmp__7 = var_rep;
obj_Int tmp__8;
tmp__8 = int_literal(0);
tmp__6 = tmp__7->clazz->>(tmp__7, tmp__8);
if (tmp__6) goto then_4;
goto else_5;
// free register tmp__6
then_4: ;
obj_TypeError tmp__9;
obj_ var_x;
obj_ var_y;
tmp__9 = var_y;
var_x = tmp__9;
goto endif_6;
else_5: ;
endif_6: ;
obj_ tmp__10;
obj_ tmp__11;
tmp__11 = var_rep;
obj_Int tmp__12;
tmp__12 = int_literal(1);
tmp__10 = tmp__11->clazz->PLUS(tmp__11, tmp__12);
var_rep = tmp__10;
obj_ tmp__13;
tmp__13 = int_literal(42);
var_y = tmp__13;
goto check_cond_1;
endwhile_3: ;
}

struct  class_BadInit BadInit_struct = {
new_classBadInit,

};

}
