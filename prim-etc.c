/* prim-etc.c -- miscellaneous primitives */

#define	REQUIRE_PWD	1
#define BASE10 10

#include <stdint.h>
#include "es.h"
#include "prim.h"

/*
 * TODO: Consider adding support for handling floating point values
 * as well as integer values. This may require adding an actual type system
 * to the shell, but that may be feasible with some boxing techniques.
 * However, it would be beneficial to be able to handle more than just
 * integer based operations to reduce the need for switching to a 
 * more general-purpose language.
 */

static List
*prim_result(List *list, Binding *binding, int evalflags) {
	return list;
}

static List
*prim_echo(List *list, Binding *binding, int evalflags) {
	const char *eol = "\n";
	if (list != NULL) {
		if (termeq(list->term, "-n")) {
			eol = "";
			list = list->next;
		} else {
			if (termeq(list->term, "--")) {
				list = list->next;
			}
		}
	}
	print("%L%s", list, " ", eol);
	return true;
}

/*
 * Add some basic arithmetic operations,
 * no attempt to avoid over/under-flows is made at this time,
 * nor is any attempt made to validate inputs.
 * TODO: Basic input validation and over/under-flow protection
 * TODO: Consider rewriting to evaluate recursively, may need to wait until
 * after tackling the tail call recursion issue mentioned in "TODO"
 */
static List
*prim_sum(List *list, Binding *binding, int evalflags) {
	int64_t sum = 0;
	for (; list != NULL; list = list->next) {
		sum += (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
	}
	return mklist(mkstr(str("%ld", sum)), NULL);
}

/* 
 * This function requires special treatment of the first argument, as
 * the first argument being '-1' will result in an evaluation of 
 * `0 - -1` with a resulting value of 1.
 */
static List
*prim_sub(List *list, Binding *binding, int evalflags) {
	int64_t sum = 0;

	if (list != NULL) {
		sum = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
		if (list->next != NULL) {
			list = list->next;
			for (; list != NULL; list = list->next) {
				sum -= (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
			}
		}
	}
	return mklist(mkstr(str("%ld", sum)), NULL);
}

static List
*prim_mul(List *list, Binding *binding, int evalflags) {
	int64_t prod = 1;
	for (; list != NULL; list = list->next) {
		prod *= (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
	}
	return mklist(mkstr(str("%ld", prod)), NULL);
}

/*
 * This operation requires some additional logic 
 * to ensure proper behaviour.
 */
static List
*prim_div(List *list, Binding *binding, int evalflags) {
	int64_t quot = 0;
	if ((list == NULL) || (list->next == NULL)) {
		fail("$&div", "Expected at least 2 integer arguments");
	}
	quot = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10); // NOLINT(clang-analyzer-core.NullDereference)
	for (list = list->next; list != NULL; list = list->next) {
		quot /= (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
	}
	return mklist(mkstr(str("%ld", quot)), NULL);
}

/*
 * Modulus operation, can't think of a reason why it should
 * accept more than 2 arguments
 */
static List
*prim_mod(List *list, Binding *binding, int evalflags) {
	uint64_t mod = 0;
	int64_t a1, a2;
	if ((list != NULL) && (length(list) == 2)) {
		a1 = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
		list = list->next;
		a2 = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
		mod = (uint64_t)(a1 % a2);
	}
	return mklist(mkstr(str("%ld", mod)), NULL);
}

/*
 * Enable greater than/less than comparisons
 * these return a bool, which is not the correct type
 * FIXME: This is wrong, but appears to be idiomatic for the codebase
 */
static List
*prim_greaterthan(List *list, Binding *binding, int evalflags) {
	int64_t a, b;
	a = b = 0;
	if ((list == NULL) || (list->next == NULL)) {
		fail("$&greaterthan", "Arity mismatch, requires 2 arguments");
	}
	a = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
	list = list->next;
	b = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
	return((a > b) ? true : false);
}

static List
*prim_lessthan(List *list, Binding *binding, int evalflags) {
	int64_t a, b;
	a = b = 0;
	if ((list == NULL) || (list->next == NULL)) {
		fail("$&lessthan", "Arity mismatch, requires 2 arguments");
	}
	a = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
	list = list->next;
	b = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
	return((a < b) ? true : false);
}

/*
 * Create bitwise primitives
 */
static List
*prim_band(List *list, Binding *binding, int evalflags) {
	int64_t a1, a2;
	a1 = a2 = 0;
	if ((list == NULL) || (list->next == NULL)) {
		fail("$&band", "Requires 2 arguments");
	}
	a1 = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10); list = list->next;
	a2 = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
	return mklist(mkstr(str("%ld", a1 & a2)), NULL);
}

static List
*prim_bxor(List *list, Binding *binding, int evalflags) {
	int64_t a1, a2;
	a1 = a2 = 0;
	if ((list == NULL) || (list->next == NULL)) {
		fail("$&bxor", "Requires 2 arguments");
	}
	a1 = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10); list = list->next;
	a2 = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
	return mklist(mkstr(str("%ld", a1 ^ a2)), NULL);
}

static List
*prim_bor(List *list, Binding *binding, int evalflags) {
	int64_t a1, a2;
	a1 = a2 = 0;
	if ((list == NULL) || (list->next == NULL)) {
		fail("$&bor", "Requires 2 arguments");
	}
	a1 = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10); list = list->next;
	a2 = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
	return mklist(mkstr(str("%ld", a1 | a2)), NULL);
}

static List
*prim_bnot(List *list, Binding *binding, int evalflags) {
	int64_t a1;
	a1 = 0;
	if (list == NULL) {
		fail("$&bnot", "Requires an argument");
	}
	a1 = (int64_t)strtol(getstr(list->term), (char **)NULL, BASE10);
	return mklist(mkstr(str("%ld", ~a1)), NULL);
}

static List
*prim_count(List *list, Binding *binding, int evalflags) {
	return mklist(mkstr(str("%d", length(list))), NULL);
}

static List
*prim_setnoexport(List *list, Binding *binding, int evalflags) {
	Ref(List *, lp, list);
	setnoexport(lp);
	RefReturn(lp);
}

static List
*prim_version(List *list, Binding *binding, int evalflags) {
	return mklist(mkstr((char *) version), NULL);
}

static List
*prim_exec(List *list, Binding *binding, int evalflags) {
	return eval(list, NULL, evalflags | eval_inchild);
}

static List
*prim_dot(List *list, Binding *binding, int evalflags) {
	int c, fd;
	Push zero, star;
	volatile int runflags = (evalflags & eval_inchild);
	const char * const usage = ". [-einvx] file [arg ...]";

	esoptbegin(list, "$&dot", usage);
	while ((c = esopt("einvx")) != EOF) {
		switch (c) {
		case 'e':	runflags |= eval_exitonfalse;	break;
		case 'i':	runflags |= run_interactive;	break;
		case 'n':	runflags |= run_noexec;		break;
		case 'v':	runflags |= run_echoinput;	break;
		case 'x':	runflags |= run_printcmds;	break;
		}
	}

	Ref(List *, result, NULL);
	Ref(List *, lp, esoptend());
	if (lp == NULL) {
		fail("$&dot", "usage: %s", usage);
	}

	Ref(char *, file, getstr(lp->term));
	lp = lp->next;
	fd = eopen(file, oOpen);
	if (fd == -1) {
		fail("$&dot", "%s: %s", file, esstrerror(errno));
	}

	varpush(&star, "*", lp);
	varpush(&zero, "0", mklist(mkstr(file), NULL));

	result = runfd(fd, file, runflags);

	varpop(&zero);
	varpop(&star);
	RefEnd2(file, lp);
	RefReturn(result);
}

static List
*prim_flatten(List *list, Binding *binding, int evalflags) {
	char *sep;
	if (list == NULL) {
		fail("$&flatten", "usage: %%flatten separator [args ...]");
	}
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = mklist(mkstr(str("%L", lp->next, sep)), NULL);
	RefReturn(lp);
}

/*
 * TODO: Since this apparently has duplicated logic,
 * see if there's a way to fold the operation into a singe,
 * possibly inlined function.
 */
static List
*prim_whatis(List *list, Binding *binding, int evalflags) {
	/* the logic in here is duplicated in eval() */
	if (list == NULL || list->next != NULL) {
		fail("$&whatis", "usage: $&whatis program");
	}
	Ref(Term *, term, list->term);
	if (getclosure(term) == NULL) {
		List *fn;
		Ref(char *, prog, getstr(term));
		assert(prog != NULL);
		fn = varlookup2("fn-", prog, binding);
		if (fn != NULL) {
			list = fn;
		} else {
			if (isabsolute(prog)) {
				char *error = checkexecutable(prog);
				if (error != NULL) {
					fail("$&whatis", "%s: %s", prog, error);
				}
			} else {
				list = pathsearch(term);
			}
		}
		RefEnd(prog);
	}
	RefEnd(term);
	return list;
}

static List
*prim_split(List *list, Binding *binding, int evalflags) {
	char *sep;
	if (list == NULL) {
		fail("$&split", "usage: %%split separator [args ...]");
	}
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = fsplit(sep, lp->next, TRUE);
	RefReturn(lp);
}

static List
*prim_fsplit(List *list, Binding *binding, int evalflags) {
	char *sep;
	if (list == NULL) {
		fail("$&fsplit", "usage: %%fsplit separator [args ...]");
	}
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = fsplit(sep, lp->next, FALSE);
	RefReturn(lp);
}

static List
*prim_var(List *list, Binding *binding, int evalflags) {
	Term *term;
	if (list == NULL) {
		return NULL;
	}
	Ref(List *, rest, list->next);
	Ref(char *, name, getstr(list->term));
	Ref(List *, defn, varlookup(name, NULL));
	rest = prim_var(rest, NULL, evalflags);
	term = mkstr(str("%S = %#L", name, defn, " "));
	list = mklist(term, rest);
	RefEnd3(defn, name, rest);
	return list;
}

static List
*prim_sethistory(List *list, Binding *binding, int evalflags) {
	if (list == NULL) {
		sethistory(NULL);
		return NULL;
	}
	Ref(List *, lp, list);
	sethistory(getstr(lp->term));
	RefReturn(lp);
}

/*
 * TODO: Look into exposing this as a means of allowing 
 * meta-programming and creating a shell DSL when needed
 */
static List
*prim_parse(List *list, Binding *binding, int evalflags) {
	List *result;
	Tree *tree;
	Ref(char *, prompt1, NULL);
	Ref(char *, prompt2, NULL);
	Ref(List *, lp, list);
	if (lp != NULL) {
		prompt1 = getstr(lp->term);
		if ((lp = lp->next) != NULL) {
			prompt2 = getstr(lp->term);
		}
	}
	RefEnd(lp);
	tree = parse(prompt1, prompt2);
	result = (tree == NULL)
		   ? NULL
		   : mklist(mkterm(NULL, mkclosure(mk(nThunk, tree), NULL)),
		     NULL);
	RefEnd2(prompt2, prompt1);
	return result;
}

/*
 * XXX: Is this function really necessary? It appears to 
 * simply be adding a flag to eval()
 */
static List
*prim_exitonfalse(List *list, Binding *binding, int evalflags) {
	return eval(list, NULL, evalflags | eval_exitonfalse);
}

static List
*prim_batchloop(List *list, Binding *binding, int evalflags) {
	Ref(List *, result, true);
	Ref(List *, dispatch, NULL);

	SIGCHK();

	ExceptionHandler

		for (;;) {
			List *parser, *cmd;
			parser = varlookup("fn-%parse", NULL);
			cmd = (parser == NULL)
					? prim("parse", NULL, NULL, 0)
					: eval(parser, NULL, 0);
			SIGCHK();
			dispatch = varlookup("fn-%dispatch", NULL);
			if (cmd != NULL) {
				if (dispatch != NULL) {
					cmd = append(dispatch, cmd);
				}
				result = eval(cmd, NULL, evalflags);
				SIGCHK();
			}
		}

	CatchException (e)

		if (!termeq(e->term, "eof")) {
			throw(e);
		}
		RefEnd(dispatch);
		if (result == true) {
			result = true;
		}
		RefReturn(result);

	EndExceptionHandler
}

static List
*prim_collect(List *list, Binding *binding, int evalflags) {
	gc();
	return true;
}

/*
 * FIXME: Use the thread-safe function getpwnam_r(3)
 * provided it's deemed necessary to use the facilities of
 * getpwnam(3) at all for this function
 */
static List
*prim_home(List *list, Binding *binding, int evalflags) {
	struct passwd *pw;
	if (list == NULL) {
		return varlookup("home", NULL);
	}
	if (list->next != NULL) {
		fail("$&home", "usage: %%home [user]");
	}
	pw = getpwnam(getstr(list->term));
	return (pw == NULL) ? NULL : mklist(mkstr(gcdup(pw->pw_dir)), NULL);
}

static List
*prim_vars(List *list, Binding *binding, int evalflags) {
	return listvars(FALSE);
}

static List
*prim_internals(List *list, Binding *binding, int evalflags) {
	return listvars(TRUE);
}

static List
*prim_isinteractive(List *list, Binding *binding, int evalflags) {
	return isinteractive() ? true : false;
}

static List
*prim_noreturn(List *list, Binding *binding, int evalflags) {
	if (list == NULL) {
		fail("$&noreturn", "usage: $&noreturn lambda args ...");
	}
	Ref(List *, lp, list);
	Ref(Closure *, closure, getclosure(lp->term));
	if (closure == NULL || closure->tree->kind != nLambda) {
		fail("$&noreturn", "$&noreturn: %E is not a lambda", lp->term);
	}
	Ref(Tree *, tree, closure->tree);
	Ref(Binding *, context, bindargs(tree->u[0].p, lp->next, closure->binding));
	lp = walk(tree->u[1].p, context, evalflags);
	RefEnd3(context, tree, closure);
	RefReturn(lp);
}

static List
*prim_setmaxevaldepth(List *list, Binding *binding, int evalflags) {
	char *s;
	long n;
	if (list == NULL) {
		maxevaldepth = MAXmaxevaldepth;
		return NULL;
	}
	if (list->next != NULL) {
		fail("$&setmaxevaldepth", "usage: $&setmaxevaldepth [limit]");
	}
	Ref(List *, lp, list);
	n = strtol(getstr(lp->term), &s, 0);
	if (n < 0 || (s != NULL && *s != '\0')) {
		fail("$&setmaxevaldepth", "max-eval-depth must be set to a positive integer");
	}
	if (n < MINmaxevaldepth) {
		n = (n == 0) ? MAXmaxevaldepth : MINmaxevaldepth;
	}
	maxevaldepth = n;
	RefReturn(lp);
}

#if READLINE
static List
*prim_resetterminal(List *list, Binding *binding, int evalflags) {
	resetterminal = TRUE;
	return true;
}
#endif


/*
 * initialization
 */

extern Dict *initprims_etc(Dict *primdict) {
	primdict = dictput(primdict, STRING(echo), (void *)prim_echo);
	primdict = dictput(primdict, STRING(count), (void *)prim_count);
	primdict = dictput(primdict, STRING(version), (void *)prim_version);
	primdict = dictput(primdict, STRING(exec), (void *)prim_exec);
	primdict = dictput(primdict, STRING(dot), (void *)prim_dot);
	primdict = dictput(primdict, STRING(flatten), (void *)prim_flatten);
	primdict = dictput(primdict, STRING(whatis), (void *)prim_whatis);
	primdict = dictput(primdict, STRING(sethistory), (void *)prim_sethistory);
	primdict = dictput(primdict, STRING(split), (void *)prim_split);
	primdict = dictput(primdict, STRING(fsplit), (void *)prim_fsplit);
	primdict = dictput(primdict, STRING(var), (void *)prim_var);
	primdict = dictput(primdict, STRING(parse), (void *)prim_parse);
	primdict = dictput(primdict, STRING(batchloop), (void *)prim_batchloop);
	primdict = dictput(primdict, STRING(collect), (void *)prim_collect);
	primdict = dictput(primdict, STRING(home), (void *)prim_home);
	primdict = dictput(primdict, STRING(sub), (void *)prim_sub);
	primdict = dictput(primdict, STRING(sum), (void *)prim_sum);
	primdict = dictput(primdict, STRING(mul), (void *)prim_mul);
	primdict = dictput(primdict, STRING(div), (void *)prim_div);
	primdict = dictput(primdict, STRING(mod), (void *)prim_mod);
	primdict = dictput(primdict, STRING(band), (void *)prim_band);
	primdict = dictput(primdict, STRING(bxor), (void *)prim_bxor);
	primdict = dictput(primdict, STRING(bnot), (void *)prim_bnot);
	primdict = dictput(primdict, STRING(bor), (void *)prim_bor);
	primdict = dictput(primdict, STRING(setnoexport), (void *)prim_setnoexport);
	primdict = dictput(primdict, STRING(vars), (void *)prim_vars);
	primdict = dictput(primdict, STRING(internals), (void *)prim_internals);
	primdict = dictput(primdict, STRING(result), (void *)prim_result);
	primdict = dictput(primdict, STRING(isinteractive), (void *)prim_isinteractive);
	primdict = dictput(primdict, STRING(exitonfalse), (void *)prim_exitonfalse);
	primdict = dictput(primdict, STRING(noreturn), (void *)prim_noreturn);
	primdict = dictput(primdict, STRING(setmaxevaldepth), (void *)prim_setmaxevaldepth);
	primdict = dictput(primdict, STRING(greaterthan), (void *)prim_greaterthan);
	primdict = dictput(primdict, STRING(lessthan), (void *)prim_lessthan);
#if READLINE
	primdict = dictput(primdict, STRING(resetterminal), (void *)prim_resetterminal);
#endif
	return primdict;
}
