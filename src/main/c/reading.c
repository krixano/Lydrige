#include "../headers/reading.h"

dval* dval_read_lambda(denv* e, mpc_ast_t* t) {
	dval* error;
	
	dval* qexpr1 = dval_read(e, t->children[0]);
	dval* qexpr2 = dval_read(e, t->children[2]);
	dval* sexpr = dval_sexpr();
	dval_add(sexpr, qexpr1);
	dval_add(sexpr, qexpr2);
	dval* lambda = builtin_lambda(e, sexpr);
	
	return errno != ERANGE ? lambda : error;
}

dval* dval_read_range(mpc_ast_t* t) {
	errno = 0;
	long min = 0;
	long max = 0;
	sscanf(t->children[0]->contents, "%ld", &min);
	sscanf(t->children[2]->contents, "%ld", &max);
	if (min < 0 || max < 0) {
		return dval_err((char*) "invalid range. Range cannot start from or end at a negative number!");
	} else if (min == max) {
		return dval_err((char*) "invalid range. The two bounds of a range cannot be the same number!");
	}
	return errno != ERANGE ? dval_range(min, max) : dval_err((char*) "invalid range");
}

dval* dval_read_int(mpc_ast_t* t) {
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ? dval_int(x) : dval_err((char*) "invalid integer");
}

dval* dval_read_double(mpc_ast_t* t) {
	errno = 0;
	double x = strtod(t->contents, NULL);
	return errno != ERANGE ? dval_double(x) : dval_err((char*) "invalid double");
}

dval* dval_read_byte(mpc_ast_t* t) {
	errno = 0;
	unsigned int hex = 0x00;
	sscanf(t->contents, "0x%02x", &hex);
	return errno != ERANGE ? dval_byte((byte) hex) : dval_err((char*) "invalid byte (hex)");
}

dval* dval_read_string(mpc_ast_t* t) {
	t->contents[strlen(t->contents) - 1] = '\0';
	char* unescaped = (char*)malloc(strlen(t->contents + 1) + 1);
	strcpy(unescaped, t->contents + 1);
	unescaped = (char*)mpcf_unescape(unescaped);
	dval* str = dval_string(unescaped);
	free(unescaped);
	return str;
}

dval* dval_read_character(mpc_ast_t* t) {
	// Remove the quotes
	errno = 0;
	char c = 'a';
	sscanf(t->contents, "'%c'", &c); // TODO: Unescape char
	return errno != ERANGE ? dval_char(c) : dval_err((char*) "invalid character");
}

dval* dval_read(denv* e, mpc_ast_t* t) {
	if (strstr(t->tag, "lambda")) {
		return dval_read_lambda(e, t);
	} else if (strstr(t->tag, "double")) {
		return dval_read_double(t);
	} else if (strstr(t->tag, "range")) {
		return dval_read_range(t);
	} else if (strstr(t->tag, "integer")) {
		return dval_read_int(t);
	} else if (strstr(t->tag, "byte")) {
		return dval_read_byte(t);
	} else if (strstr(t->tag, "string")) {
		return dval_read_string(t);
	} else if (strstr(t->tag, "symbol")) {
		return dval_sym(t->contents, DVAL_SYM);
	} else if (strstr(t->tag, "character")) {
		return dval_read_character(t);
	}

	dval* x = NULL;
	if (strcmp(t->tag, ">") == 0) { // Not working correctly in REPL. Should not make sexpr if statements are being used!
		x = dval_sexpr();
	} else if (strstr(t->tag, "statement")) {
		x = dval_sexpr();
	} else if (strstr(t->tag, "ssexpr")) {
		x = dval_ssexpr();
	} else if (strstr(t->tag, "sexpr")) {
		x = dval_sexpr();
	} else if (strstr(t->tag, "sqexpr")) {
		x = dval_sqexpr();
	} else if (strstr(t->tag, "qexpr")) {
		x = dval_qexpr();
	} else if (strstr(t->tag, "slist")) {
		x = dval_slist();
	} else if (strstr(t->tag, "list")) {
		x = dval_list();
	} else if (strstr(t->tag, "note")) {
		x = dval_note();
	}

	for (int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->contents, "(") == 0) continue;
		else if (strcmp(t->children[i]->contents, ")") == 0) continue;
		else if (strcmp(t->children[i]->contents, "{") == 0) continue;
		else if (strcmp(t->children[i]->contents, "}") == 0) continue;
		else if (strcmp(t->children[i]->contents, "[") == 0) continue;
		else if (strcmp(t->children[i]->contents, "]") == 0) continue;
		else if (strcmp(t->children[i]->contents, "'[") == 0) continue;
		else if (strcmp(t->children[i]->contents, "'(") == 0) continue;
		else if (strcmp(t->children[i]->contents, "'{") == 0) continue;
		else if (strcmp(t->children[i]->contents, ",") == 0) continue;
		else if (strcmp(t->children[i]->contents, ":") == 0) continue;
		else if (strcmp(t->children[i]->contents, ";") == 0) continue;
		else if (strcmp(t->children[i]->contents, "->") == 0) continue;
		else if (strcmp(t->children[i]->tag, "regex") == 0) continue;
		else if (strstr(t->children[i]->tag, "comment")) continue;
		dval* v = dval_read(e, t->children[i]);
		if (x->type == DVAL_NOTE && v->type != DVAL_TYPE) {
			dval_del(v);
			return dval_err("Only types are allowed in Notes! Found %s.", dtype_name(v->type));
		}
		x = dval_add(x, v);
	}

	return x;
}
