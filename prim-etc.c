/* prim-etc.c -- miscellaneous primitives ($Revision: 1.2 $) */

#define	REQUIRE_PWD	1

#include <stdint.h>
#include "es.h"
#include "prim.h"

PRIM(result) {
	return list;
}

PRIM(echo) {
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
PRIM(sum) {
	int64_t sum = 0;
	for (; list != NULL; list = list->next) {
		sum += (int64_t)strtol(getstr(list->term), (char **)NULL, 10);
	}
	return mklist(mkstr(str("%ld", sum)), NULL);
}

/* 
 * This function requires special treatment of the first argument, as
 * the first argument being '-1' will result in an evaluation of 
 * `0 - -1` with a resulting value of 1.
 */
PRIM(sub) {
	int64_t sum = 0;

	if (list != NULL) {
		sum = (int64_t)strtol(getstr(list->term), (char **)NULL, 10);
		for (list = list->next; list != NULL; list = list->next) {
			sum -= (int64_t)strtol(getstr(list->term), (char **)NULL, 10);
		}
	}
	return mklist(mkstr(str("%ld", sum)), NULL);
}

PRIM(mul) {
	int64_t prod = 1;
	for (; list != NULL; list = list->next) {
		prod *= (int64_t)strtol(getstr(list->term), (char **)NULL, 10);
	}
	return mklist(mkstr(str("%ld", prod)), NULL);
}

/* 
 * This operation requires some additional logic 
 * to ensure proper behaviour.
 */
PRIM(div) {
	int64_t quot = 0;
	if ((list == NULL) || (list->next == NULL)) {
		fail("$&div", "Expected at least 2 integer arguments");
	}
	quot = (int64_t)strtol(getstr(list->term), (char **)NULL, 10);
	for (list = list->next; list != NULL; list = list->next) {
		quot /= (int64_t)strtol(getstr(list->term), (char **)NULL, 10);
	}
	return mklist(mkstr(str("%ld", quot)), NULL);
}

/* 
 * Modulus operation, can't think of a reason why it should
 * accept more than 2 arguments
 */
PRIM(mod) {
	uint64_t mod = 0;
	int64_t a1, a2;
	if ((list != NULL) && (length(list) == 2)) {
		a1 = (int64_t)strtol(getstr(list->term), (char **)NULL, 10);
		list = list->next;
		a2 = (int64_t)strtol(getstr(list->term), (char **)NULL, 10);
		mod = (uint64_t)(a1 % a2);
	}
	return mklist(mkstr(str("%ld", mod)), NULL);
}

/*
 * Create bitwise primitives
 */
PRIM(band) {
	int64_t a1, a2;
	a1 = a2 = 0;
	if ((list == NULL) || (list->next == NULL)) {
		fail("$&band", "Requires 2 arguments");
	}
	a1 = (int64_t)strtol(getstr(list->term), (char **)NULL, 10); list = list->next;
	a2 = (int64_t)strtol(getstr(list->term), (char **)NULL, 10);
	return mklist(mkstr(str("%ld", a1 & a2)), NULL);
}

PRIM(bxor) {
	int64_t a1, a2;
	a1 = a2 = 0;
	if ((list == NULL) || (list->next == NULL)) {
		fail("$&bxor", "Requires 2 arguments");
	}
	a1 = (int64_t)strtol(getstr(list->term), (char **)NULL, 10); list = list->next;
	a2 = (int64_t)strtol(getstr(list->term), (char **)NULL, 10);
	return mklist(mkstr(str("%ld", a1 ^ a2)), NULL);
}

PRIM(bor) {
	int64_t a1, a2;
	a1 = a2 = 0;
	if ((list == NULL) || (list->next == NULL)) {
		fail("$&bor", "Requires 2 arguments");
	}
	a1 = (int64_t)strtol(getstr(list->term), (char **)NULL, 10); list = list->next;
	a2 = (int64_t)strtol(getstr(list->term), (char **)NULL, 10);
	return mklist(mkstr(str("%ld", a1 | a2)), NULL);
}

PRIM(bnot) {
	int64_t a1;
	a1 = 0;
	if (list == NULL) {
		fail("$&bnot", "Requires an argument");
	}
	a1 = (int64_t)strtol(getstr(list->term), (char **)NULL, 10);
	return mklist(mkstr(str("%ld", ~a1)), NULL);
}

PRIM(count) {
	return mklist(mkstr(str("%d", length(list))), NULL);
}

PRIM(setnoexport) {
	Ref(List *, lp, list);
	setnoexport(lp);
	RefReturn(lp);
}

PRIM(version) {
	return mklist(mkstr((char *) version), NULL);
}

PRIM(exec) {
	return eval(list, NULL, evalflags | eval_inchild);
}

PRIM(dot) {
	int c, fd;
	Push zero, star;
	volatile int runflags = (evalflags & eval_inchild);
	const char * const usage = ". [-einvx] file [arg ...]";

	esoptbegin(list, "$&dot", usage);
	while ((c = esopt("einvx")) != EOF)
		switch (c) {
		case 'e':	runflags |= eval_exitonfalse;	break;
		case 'i':	runflags |= run_interactive;	break;
		case 'n':	runflags |= run_noexec;		break;
		case 'v':	runflags |= run_echoinput;	break;
		case 'x':	runflags |= run_printcmds;	break;
		}

	Ref(List *, result, NULL);
	Ref(List *, lp, esoptend());
	if (lp == NULL)
		fail("$&dot", "usage: %s", usage);

	Ref(char *, file, getstr(lp->term));
	lp = lp->next;
	fd = eopen(file, oOpen);
	if (fd == -1)
		fail("$&dot", "%s: %s", file, esstrerror(errno));

	varpush(&star, "*", lp);
	varpush(&zero, "0", mklist(mkstr(file), NULL));

	result = runfd(fd, file, runflags);

	varpop(&zero);
	varpop(&star);
	RefEnd2(file, lp);
	RefReturn(result);
}

PRIM(flatten) {
	char *sep;
	if (list == NULL)
		fail("$&flatten", "usage: %%flatten separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = mklist(mkstr(str("%L", lp->next, sep)), NULL);
	RefReturn(lp);
}

PRIM(whatis) {
	/* the logic in here is duplicated in eval() */
	if (list == NULL || list->next != NULL)
		fail("$&whatis", "usage: $&whatis program");
	Ref(Term *, term, list->term);
	if (getclosure(term) == NULL) {
		List *fn;
		Ref(char *, prog, getstr(term));
		assert(prog != NULL);
		fn = varlookup2("fn-", prog, binding);
		if (fn != NULL)
			list = fn;
		else {
			if (isabsolute(prog)) {
				char *error = checkexecutable(prog);
				if (error != NULL)
					fail("$&whatis", "%s: %s", prog, error);
			} else
				list = pathsearch(term);
		}
		RefEnd(prog);
	}
	RefEnd(term);
	return list;
}

PRIM(split) {
	char *sep;
	if (list == NULL)
		fail("$&split", "usage: %%split separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = fsplit(sep, lp->next, TRUE);
	RefReturn(lp);
}

PRIM(fsplit) {
	char *sep;
	if (list == NULL)
		fail("$&fsplit", "usage: %%fsplit separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = fsplit(sep, lp->next, FALSE);
	RefReturn(lp);
}

PRIM(var) {
	Term *term;
	if (list == NULL)
		return NULL;
	Ref(List *, rest, list->next);
	Ref(char *, name, getstr(list->term));
	Ref(List *, defn, varlookup(name, NULL));
	rest = prim_var(rest, NULL, evalflags);
	term = mkstr(str("%S = %#L", name, defn, " "));
	list = mklist(term, rest);
	RefEnd3(defn, name, rest);
	return list;
}

PRIM(sethistory) {
	if (list == NULL) {
		sethistory(NULL);
		return NULL;
	}
	Ref(List *, lp, list);
	sethistory(getstr(lp->term));
	RefReturn(lp);
}

PRIM(parse) {
	List *result;
	Tree *tree;
	Ref(char *, prompt1, NULL);
	Ref(char *, prompt2, NULL);
	Ref(List *, lp, list);
	if (lp != NULL) {
		prompt1 = getstr(lp->term);
		if ((lp = lp->next) != NULL)
			prompt2 = getstr(lp->term);
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

PRIM(exitonfalse) {
	return eval(list, NULL, evalflags | eval_exitonfalse);
}

PRIM(batchloop) {
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
				if (dispatch != NULL)
					cmd = append(dispatch, cmd);
				result = eval(cmd, NULL, evalflags);
				SIGCHK();
			}
		}

	CatchException (e)

		if (!termeq(e->term, "eof"))
			throw(e);
		RefEnd(dispatch);
		if (result == true)
			result = true;
		RefReturn(result);

	EndExceptionHandler
}

PRIM(collect) {
	gc();
	return true;
}

PRIM(home) {
	struct passwd *pw;
	if (list == NULL)
		return varlookup("home", NULL);
	if (list->next != NULL)
		fail("$&home", "usage: %%home [user]");
	pw = getpwnam(getstr(list->term));
	return (pw == NULL) ? NULL : mklist(mkstr(gcdup(pw->pw_dir)), NULL);
}

PRIM(vars) {
	return listvars(FALSE);
}

PRIM(internals) {
	return listvars(TRUE);
}

PRIM(isinteractive) {
	return isinteractive() ? true : false;
}

PRIM(noreturn) {
	if (list == NULL)
		fail("$&noreturn", "usage: $&noreturn lambda args ...");
	Ref(List *, lp, list);
	Ref(Closure *, closure, getclosure(lp->term));
	if (closure == NULL || closure->tree->kind != nLambda)
		fail("$&noreturn", "$&noreturn: %E is not a lambda", lp->term);
	Ref(Tree *, tree, closure->tree);
	Ref(Binding *, context, bindargs(tree->u[0].p, lp->next, closure->binding));
	lp = walk(tree->u[1].p, context, evalflags);
	RefEnd3(context, tree, closure);
	RefReturn(lp);
}

PRIM(setmaxevaldepth) {
	char *s;
	long n;
	if (list == NULL) {
		maxevaldepth = MAXmaxevaldepth;
		return NULL;
	}
	if (list->next != NULL)
		fail("$&setmaxevaldepth", "usage: $&setmaxevaldepth [limit]");
	Ref(List *, lp, list);
	n = strtol(getstr(lp->term), &s, 0);
	if (n < 0 || (s != NULL && *s != '\0'))
		fail("$&setmaxevaldepth", "max-eval-depth must be set to a positive integer");
	if (n < MINmaxevaldepth)
		n = (n == 0) ? MAXmaxevaldepth : MINmaxevaldepth;
	maxevaldepth = n;
	RefReturn(lp);
}

#if READLINE
PRIM(resetterminal) {
	resetterminal = TRUE;
	return true;
}
#endif


/*
 * initialization
 */

extern Dict *initprims_etc(Dict *primdict) {
	X(echo);
	X(count);
	X(version);
	X(exec);
	X(dot);
	X(flatten);
	X(whatis);
	X(sethistory);
	X(split);
	X(fsplit);
	X(var);
	X(parse);
	X(batchloop);
	X(collect);
	X(home);
	X(sub);
	X(sum);
	X(mul);
	X(div);
	X(mod);
	X(band);
	X(bxor);
	X(bnot);
	X(bor);
	X(setnoexport);
	X(vars);
	X(internals);
	X(result);
	X(isinteractive);
	X(exitonfalse);
	X(noreturn);
	X(setmaxevaldepth);
#if READLINE
	X(resetterminal);
#endif
	return primdict;
}
