/* Lydrige - A simple interpreted programming language inspired by Lisp.
 * Copyright (c) 2016, Christian Seibold
 * Under MIT License
 *
 * You can find latest source code at:
 * https://github.com/krixano/Lydrige
 *
 * -----------------------------------------------------------------------
 * MIT License
 *
 * Copyright (c) 2016 Christian Seibold
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * -----------------------------------------------------------------------
 */

#include <builtin.h>
#include <colors.h>
#include <stdlib.h>

/*
* k - key
* v - value
* constant - not editable/modifiable
* You must delete v yourself, the function will not do this for you
*/
dval *denv_put(denv *e, char *k, dval *v, int constant) {
    dval *t;
    bstring k_bstr = bfromcstr(k);
    dval *item = (dval *) Hashmap_get(e->map, k_bstr);
    if (item != NULL) { // If already defined in hashtable
        if (item->constant == 0) { // If not constant (in hashtable)
            dval *deleted = (dval *) Hashmap_delete(e->map, k_bstr);
            dval_del(deleted); // Note that `deleted` is the same as `item`!
            item = NULL;
            t = dval_copy(v); // Copy value into t
            t->constant = constant; // set constant
            Hashmap_set(e->map, k_bstr, t); // TODO: Check for errors!
            return(dval_int(0)); // TODO: Return something else?
        } else { // TODO
            bdestroy(k_bstr);
            return(dval_error("Cannot edit '%s'. It is a constant.", k));
        }
    } else { // Not in hashtable yet!
        //e->count++; // TODO: Doesn't hashmap have a count?
        t = dval_copy(v);
        t->constant = constant; // set constant
        Hashmap_set(e->map, k_bstr, t); // TODO: Check for errors!
        return(denv_get(e, "true")); // Return true
    }
}

dval *denv_get(denv *e, char *k) { // Copies the value gotten from hashmap, so it must be freed after calling this function!
    dval *v;
    bstring k_bstr = bfromcstr(k);
    if ((v = (dval *) Hashmap_get(e->map, k_bstr))) {
        dval *result = dval_copy(v); // TODO: Possible Memory Leak - should d get deleted? Not until end of program (when destroying hashmap)
        bdestroy(k_bstr);
        return(result);
    }
    bdestroy(k_bstr);
    
    if (e->parent) {
        return(denv_get(e->parent, k));
    }
    
    return(dval_error("Unbound symbol '%s'.", k));
}

dval *builtin_add(denv *e, dval *args, unsigned int argc) {
    if (argc == 0) {
        return(dval_error("Function '+' given wrong amount of arguments. Got '%d', Expected at least 1.", argc));
    }
    // Check if any are doubles. Set isDouble to true is so.
    int isDouble = false;
    for (int i = 0; i < argc; i++) {
        if (args[i].type == DVAL_LIST) { // DEBUG/DEV
            //print all of the elements
            builtin_print(e, args[i].elements, args[i].count);
        }
        if (!(args[i].type == DVAL_DOUBLE || args[i].type == DVAL_INT)) {
            return(dval_error("Function '+' may only be passed values of type int or double."));
        }
        if (args[i].type == DVAL_DOUBLE) {
            isDouble = true;
        }
    }
    if (isDouble) {
        double accumulator = 0.0;
        if (args[0].type == DVAL_DOUBLE) {
            accumulator = args[0].doub;
        } else if (args[0].type == DVAL_INT) {
            accumulator = (double) args[0].integer;
        }
        for (int i = 1; i < argc; i++) {
            if (args[i].type == DVAL_INT) {
                accumulator += (double) args[i].integer;
            } else if (args[i].type == DVAL_DOUBLE) {
                accumulator += args[i].doub;
            }
        }
        return(dval_double(accumulator));
    } else {
        int accumulator = args[0].integer;
        for (int i = 1; i < argc; i++) {
            accumulator += args[i].integer;
        }
        return(dval_int(accumulator));
    }
}

dval *builtin_subtract(denv *e, dval *args, unsigned int argc) {
    if (argc == 0) {
        return(dval_error("Function '+' given wrong amount of arguments. Got '%d', Expected at least 1.", argc));
    }
    // Check if any are doubles. Set isDouble to true is so.
    int isDouble = false;
    for (int i = 0; i < argc; i++) {
        if (!(args[i].type == DVAL_DOUBLE || args[i].type == DVAL_INT)) {
            return(dval_error("Function '-' may only be passed values of type int or double."));
        }
        if (args[i].type == DVAL_DOUBLE) {
            isDouble = true;
        }
    }
    if (isDouble) {
        double accumulator;
        if (args[0].type == DVAL_DOUBLE) {
            accumulator = args[0].doub;
        } else if (args[0].type == DVAL_INT) {
            accumulator = (double) args[0].integer;
        }
        for (int i = 1; i < argc; i++) {
            if (args[i].type == DVAL_INT) {
                accumulator -= (double) args[i].integer;
            } else if (args[i].type == DVAL_DOUBLE) {
                accumulator -= args[i].doub;
            }
        }
        return(dval_double(accumulator));
    } else {
        int accumulator = args[0].integer;
        for (int i = 1; i < argc; i++) {
            accumulator -= args[i].integer;
        }
        return(dval_int(accumulator));
    }
}

dval *builtin_multiply(denv *e, dval *args, unsigned int argc) {
    if (argc == 0) {
        return(dval_error("Function '+' given wrong amount of arguments. Got '%d', Expected at least 1.", argc));
    }
    // Check if any are doubles. Set isDouble to true is so.
    int isDouble = false;
    for (int i = 0; i < argc; i++) {
        if (!(args[i].type == DVAL_DOUBLE || args[i].type == DVAL_INT)) {
            return(dval_error("Function '*' may only be passed values of type int or double."));
        }
        if (args[i].type == DVAL_DOUBLE) {
            isDouble = true;
        }
    }
    if (isDouble) {
        double accumulator = 0.0;
        if (args[0].type == DVAL_DOUBLE) {
            accumulator = args[0].doub;
        } else if (args[0].type == DVAL_INT) {
            accumulator = (double) args[0].integer;
        }
        for (int i = 1; i < argc; i++) {
            if (args[i].type == DVAL_INT) {
                accumulator *= (double) args[i].integer;
            } else if (args[i].type == DVAL_DOUBLE) {
                accumulator *= args[i].doub;
            }
        }
        return(dval_double(accumulator));
    } else {
        int accumulator = args[0].integer;
        for (int i = 1; i < argc; i++) {
            accumulator *= args[i].integer;
        }
        return(dval_int(accumulator));
    }
}

dval *builtin_divide(denv *e, dval *args, unsigned int argc) {
    if (argc == 0) {
        return(dval_error("Function '/' given wrong amount of arguments. Got '%d', Expected at least 1.", argc));
    }
    // Check if any are doubles. Set isDouble to true is so.
    int isDouble = false;
    for (int i = 0; i < argc; i++) {
        if (args[i].type == DVAL_DOUBLE) {
            if (args[i].doub == 0.0 && i != 0) {
                return(dval_error("Function '/' - Cannot divide by zero."));
            }
            isDouble = true;
        } else if (args[i].type == DVAL_INT) {
            if (args[i].integer == 0 && i != 0) {
                return(dval_error("Function '/' - Cannot divide by zero."));
            }
        } else {
            return(dval_error("Function '/' may only be passed values of type int or double."));
        }
    }
    if (isDouble) {
        double accumulator = 0.0;
        if (args[0].type == DVAL_DOUBLE) {
            accumulator = args[0].doub;
        } else if (args[0].type == DVAL_INT) {
            accumulator = (double) args[0].integer;
        }
        for (int i = 1; i < argc; i++) {
            if (args[i].type == DVAL_INT) {
                accumulator /= (double) args[i].integer;
            } else if (args[i].type == DVAL_DOUBLE) {
                accumulator /= args[i].doub;
            }
        }
        return(dval_double(accumulator));
    } else {
        int accumulator = args[0].integer;
        for (int i = 1; i < argc; i++) {
            accumulator /= args[i].integer;
        }
        return(dval_int(accumulator));
    }
}

dval *builtin_mod(denv *e, dval *args, unsigned int argc) {
    if (argc == 0) {
        return(dval_error("Function '%' given wrong amount of arguments. Got '%d', Expected at least 1.", argc));
    }
    // Check if any are doubles. Set isDouble to true is so.
    for (int i = 0; i < argc; i++) {
        if (args[i].type == DVAL_INT) {
            if (args[i].integer == 0 && i != 0) {
                return(dval_error("Function 'mod' - Cannot divide by zero."));
            }
        } else {
            return(dval_error("Function 'mod' may only be passed values of type int."));
        }
    }
    int accumulator = args[0].integer;
    for (int i = 1; i < argc; i++) {
        accumulator %= args[i].integer;
    }
    return(dval_int(accumulator));
}

dval *builtin_succ(denv *e, dval *args, unsigned int argc) {
    if (argc > 1) {
        return(dval_error("Function 'succ' may only be passed 1 argument."));
    }
    
    switch (args[0].type) {
        case DVAL_INT:
        return(dval_int(args[0].integer + 1));
        case DVAL_DOUBLE:
        return(dval_double(args[0].doub + 1));
        default:
        return(dval_error("Function 'succ' must be given values of type int or double."));
    }
}

dval *builtin_list(denv *e, dval *args, unsigned int argc) {
    if (argc == 0) {
        return(dval_error("Function 'list' must be passed 1 or more arguments."));
    }
    
    dval *largs = calloc(argc, sizeof(dval));
    memcpy(largs, args, sizeof(dval) * argc);
    return(dval_list(largs, argc));
}

dval *builtin_len(denv *a, dval *args, unsigned int argc) {
    if (argc > 1 || argc == 0) {
        return(dval_error("Function 'len' must be passed only 1 argument."));
    }
    if (args[0].type != DVAL_LIST) {
        return(dval_error("Function 'len' must be passed a list."));
    }
    
    return(dval_int(args[0].count));
}

dval *builtin_get(denv *a, dval *args, unsigned int argc) {
    if (argc > 2 || argc == 0) {
        return(dval_error("Function 'get' must be passed 2 arguments."));
    }
    if (args[0].type != DVAL_INT) {
        return(dval_error("Function 'get' must be passed an integer for argument 1."));
    }
    if (args[1].type != DVAL_LIST) {
        return(dval_error("Function 'get' must be passed a list for argument 2."));
    }
    
    dval *arg = calloc(1, sizeof(dval));
    memcpy(arg, args[1].elements + args[0].integer, sizeof(dval) * 1); // Copy element at given index into arg
    return(arg);
}

dval *builtin_set(denv *a, dval *args, unsigned int argc) {
    if (argc > 3 || argc == 0) {
        return(dval_error("Function 'get' must be passed 3 arguments."));
    }
    if (args[0].type != DVAL_INT) {
        return(dval_error("Function 'get' must be passed an integer for argument 1."));
    }
    if (args[2].type != DVAL_LIST) {
        return(dval_error("Function 'get' must be passed a list for argument 3."));
    }
    
    dval *largs = calloc(args[2].count, sizeof(dval));
    memcpy(largs, args[2].elements, sizeof(dval) * (args[2].count)); // Copy all elements into arg
    largs[args[0].integer] = args[1];
    return(dval_list(largs, args[2].count));
}

dval *builtin_first(denv *a, dval *args, unsigned int argc) {
    if (argc > 1 || argc == 0) {
        return(dval_error("Function 'first' must be passed only 1 argument."));
    }
    if (args[0].type != DVAL_LIST) {
        return(dval_error("Function 'first' must be passed a list."));
    }
    
    dval *arg = calloc(1, sizeof(dval));
    memcpy(arg, args[0].elements, sizeof(dval) * 1); // Copy only the first element into arg
    return(arg);
}

dval *builtin_last(denv *a, dval *args, unsigned int argc) {
    if (argc > 1 || argc == 0) {
        return(dval_error("Function 'last' must be passed only 1 argument."));
    }
    if (args[0].type != DVAL_LIST) {
        return(dval_error("Function 'last' must be passed a list."));
    }
    
    dval *arg = calloc(1, sizeof(dval));
    memcpy(arg, args[0].elements+(args[0].count-1), sizeof(dval) * 1); // Copy only the very last element into arg
    return(arg);
}

dval *builtin_head(denv *a, dval *args, unsigned int argc) {
    if (argc > 1 || argc == 0) {
        return(dval_error("Function 'head' must be passed only 1 argument."));
    }
    if (args[0].type != DVAL_LIST) {
        return(dval_error("Function 'head' must be passed a list."));
    }
    
    unsigned int count = args[0].count - 1;
    dval *largs = calloc(count, sizeof(dval));
    memcpy(largs, args[0].elements, sizeof(dval) * (args[0].count - 1)); // Copy all but last element into largs
    return(dval_list(largs, count));
}

dval *builtin_tail(denv *a, dval *args, unsigned int argc) {
    if (argc > 1 || argc == 0) {
        return(dval_error("Function 'head' must be passed only 1 argument."));
    }
    if (args[0].type != DVAL_LIST) {
        return(dval_error("Function 'head' must be passed a list."));
    }
    
    unsigned int count = args[0].count - 1;
    dval *largs = calloc(count, sizeof(dval));
    memcpy(largs, args[0].elements + 1, sizeof(dval) * (args[0].count - 1)); // Copy all but first element into largs
    return(dval_list(largs, count));
}

dval *builtin_join(denv *a, dval *args, unsigned int argc) {
    if (argc <= 1 || argc > 2) {
        return(dval_error("Function 'join' must be passed only 2 arguments."));
    }
    if (args[0].type != DVAL_LIST || args[1].type != DVAL_LIST) {
        return(dval_error("Function 'join' must be passed lists."));
    }
    
    unsigned int largc = args[0].count + args[1].count;
    dval *largs = calloc(largc, sizeof(dval));
    memcpy(largs, args[0].elements, sizeof(dval) * args[0].count);
    memcpy(largs + args[0].count, args[1].elements, sizeof(dval) * args[1].count);
    dval *list = dval_list(largs, largc);
    return(list);
}

dval *builtin_def(denv *a, dval *args, unsigned int argc) {
    if (argc <= 1 || argc > 2) {
        return(dval_error("Function 'def' must be passed only 2 arguments."));
    }
    if (args[0].type != DVAL_IDENT) {
        return(dval_error("Function 'def' must be passed quoted identifier for argument 1"));
    }
    
    
    
    return(dval_error("Function 'def' has not been implemented yet."));
}

dval *builtin_typeof(denv *a, dval *args, unsigned int argc) {
    if (argc < 1 || argc > 1) {
        return(dval_error("function 'typeof' must be passed one argument only."));
    }
    
    dval *type = dval_type(args[0].type);
    return type;
}

char *get_type_string(dval *type) {
    switch(type->typeValue) {
        case DVAL_ANY:
        {
            return("any");
        } break;
        
        case DVAL_INT:
        {
            return("int");
        } break;
        
        case DVAL_DOUBLE:
        {
            return("double");
        } break;
        
        case DVAL_CHARACTER:
        {
            return("char");
        } break;
        
        case DVAL_STRING:
        {
            return("string");
        } break;
        
        case DVAL_FUNC:
        {
            return("func");
        } break;
        
        case DVAL_LIST:
        {
            return("array");
        } break;
        
        case DVAL_TYPEVALUE:
        {
            return("type");
        } break;
        
        case DVAL_IDENT:
        {
            return("qident"); // Change this name
        } break;
        
        default:
        {
            return("unknown_type");
        } break;
    }
}

bool print_elem(dval arg, bool removeQuotations) { // Use pointer to dval?
    switch (arg.type) {
        case DVAL_INT:
        {
            printf("%d", arg.integer);
            return true;
        } break;
        
        case DVAL_DOUBLE:
        {
            printf("%f", arg.doub);
            return true;
        } break;
        
        case DVAL_CHARACTER:
        {
            if (removeQuotations) {
                printf("%c", arg.character);
            } else {
                printf("'%c'", arg.character);
            }
            return true;
        } break;
        
        case DVAL_STRING:
        {
            if (removeQuotations) {
                printf("%s", arg.nstr);
            } else {
                printf("\"%s\"", arg.nstr);
            }
            return true;
        } break;
        
        case DVAL_FUNC:
        {
            printf("(Func)");
            return true;
        } break;
        
        case DVAL_LIST:
        {
            printf("[");
            for (int i = 0; i < arg.count; i++) {
                print_elem(arg.elements[i], false);
                if (i != arg.count - 1) {
                    printf(", ");
                }
            }
            printf("]");
            return true;
        } break;
        
        case DVAL_EXPR:
        {
            printf("(");
            for (int i = 0; i < arg.count; i++) {
                print_elem(arg.elements[i], false);
                if (i != arg.count - 1 && i != 0) {
                    printf(", ");
                } else if (i == 0) {
                    printf(" ");
                }
            }
            printf(")");
            return true;
        } break;
        
        case DVAL_QEXPR:
        {
            printf("{");
            for (int i = 0; i < arg.count; i++) {
                print_elem(arg.elements[i], false);
                if (i != arg.count - 1) {
                    printf(" ");
                }
            }
            printf("}");
            return true;
        } break;
        
        case DVAL_IDENT:
        {
            printf(".%s", arg.str);
            return true;
        } break;
        
        case DVAL_TYPEVALUE:
        {
            printf("%s", get_type_string(&arg));
            return true;
        } break;
        
        default:
        {
            return false;
        }
    }
}

dval *builtin_print(denv *e, dval *args, unsigned int argc) {
    for (int i = 0; i < argc; i++) {
        if (!print_elem(args[i], true)) {
            return(dval_error("Cannot print value of type Unknown or Any!"));
        }
        printf(" ");
    }
    printf("\n");
    return(denv_get(e, "true"));
}

dval *builtin_read(denv *e, dval *args, unsigned int argc) { // TODO: Bug with strings not being printed correctly using this expression: print (read "> ");
    if (argc < 1 || argc > 1) {
        return(dval_error("Function 'read' must be passed only 1 argument"));
    }
    if (args[0].type != DVAL_STRING) {
        return(dval_error("Function 'read' must be passed a string for argument 1"));
    }
    
    printf("%s", args[0].str);
    char input[255];
    /*fgets(input, 255, stdin);
    char *pos;
    if ((pos=strchr(input, '\n')) != NULL)
     *pos = '\0';*/
    scanf("%s", input);
    return(dval_string((char *)input));
}

dval *builtin_clear(denv *e, dval *args, unsigned int argc) {
#ifdef _WIN32
    system("cls"); // TODO: Change this?
#else
    linenoiseClearScreen();
#endif
    return(denv_get(e, "true"));
}

dval *builtin_exit(denv *e, dval *args, unsigned int argc) {
    running = false;
    return(denv_get(e, "true"));
}

internal void denv_add_builtin(denv *e, char *name, dbuiltin func) {
    dval *v = dval_func(func, 1);
    dval_del(denv_put(e, name, v, v->constant)); // v is coppied (but not deleted in denv_put function)
    dval_del(v);
}

internal void denv_add_type(denv *e, char *name, DVAL_TYPE type) {
    dval *v = dval_type(type);
    dval_del(denv_put(e, name, v, v->constant));
    dval_del(v);
}

internal void denv_add_const(denv *e, char *name, dval *v) {
    v->constant = 1;
    dval_del(denv_put(e, name, v, 1));
    dval_del(v);
}

void denv_add_builtins(denv *e) {
    denv_add_builtin(e, "+", builtin_add);
    denv_add_builtin(e, "-", builtin_subtract);
    denv_add_builtin(e, "*", builtin_multiply);
    denv_add_builtin(e, "/", builtin_divide);
    denv_add_builtin(e, "mod", builtin_mod);
    // TODO: Add Power Operation [^]
    denv_add_builtin(e, "succ", builtin_succ);
    
    denv_add_builtin(e, "list", builtin_list);
    denv_add_builtin(e, "len", builtin_len);
    denv_add_builtin(e, "get", builtin_get);
    denv_add_builtin(e, "set", builtin_set);
    denv_add_builtin(e, "first", builtin_first);
    denv_add_builtin(e, "last", builtin_last);
    denv_add_builtin(e, "head", builtin_head);
    denv_add_builtin(e, "tail", builtin_tail);
    denv_add_builtin(e, "join", builtin_join);
    
    denv_add_builtin(e, "def", builtin_def);
    denv_add_builtin(e, "typeof", builtin_typeof);
    
    denv_add_builtin(e, "print", builtin_print);
    denv_add_builtin(e, "read", builtin_read);
    denv_add_builtin(e, "clear", builtin_clear);
    denv_add_builtin(e, "exit", builtin_exit);
    
    denv_add_type(e, "any", DVAL_ANY);
    denv_add_type(e, "int", DVAL_INT);
    denv_add_type(e, "double", DVAL_DOUBLE);
    denv_add_type(e, "char", DVAL_CHARACTER);
    denv_add_type(e, "string", DVAL_STRING);
    denv_add_type(e, "func", DVAL_FUNC);
    denv_add_type(e, "array", DVAL_LIST);
    denv_add_type(e, "type", DVAL_TYPEVALUE);
    
    denv_add_const(e, "true", dval_int(0));
}
