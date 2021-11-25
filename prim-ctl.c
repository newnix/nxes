/* prim-ctl.c -- control flow primitives */

#include "es.h"
#include "prim.h"

/*
 * TODO: Look into removing unused parameters from these
 * function definitions.
 */
static List
*prim_seq(List *list, Binding *binding, int evalflags) {
	Ref(List *, result, true);
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next) {
		result = eval1(lp->term, evalflags &~ (lp->next == NULL ? 0 : eval_inchild));
	}
	RefEnd(lp);
	RefReturn(result);
}

static List
*prim_if(List *list, Binding *binding, int evalflags) {
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next) {
		List *cond = eval1(lp->term, evalflags & (lp->next == NULL ? eval_inchild : 0));
		lp = lp->next;
		if (lp == NULL) {
			RefPop(lp);
			return cond;
		}
		if (istrue(cond)) {
			List *result = eval1(lp->term, evalflags);
			RefPop(lp);
			return result;
		}
	}
	RefEnd(lp);
	return true;
}

static List
*prim_forever(List *list, Binding *binding, int evalflags) {
	Ref(List *, body, list);
	for (;;) {
		list = eval(body, NULL, evalflags & eval_exitonfalse);
	}
	RefEnd(body);
	return list;
}

static List
*prim_throw(List *list, Binding *binding, int evalflags) {
	if (list == NULL) {
		fail("$&throw", "usage: throw exception [args ...]");
	}
	throw(list);
	NOTREACHED;
}

static List
*prim_catch(List *list, Binding *binding, int evalflags) {
	Atomic retry;

	if (list == NULL) {
		fail("$&catch", "usage: catch catcher body");
	}

	Ref(List *, result, NULL);
	Ref(List *, lp, list);

	do {
		retry = FALSE;

		ExceptionHandler

			result = eval(lp->next, NULL, evalflags);

		CatchException (frombody)

			blocksignals();
			ExceptionHandler
				result
				  = eval(mklist(mkstr("$&noreturn"),
					        mklist(lp->term, frombody)),
					 NULL,
					 evalflags);
				unblocksignals();
			CatchException (fromcatcher)

				if (termeq(fromcatcher->term, "retry")) {
					retry = TRUE;
					unblocksignals();
				} else {
					unblocksignals();
					throw(fromcatcher);
				}
			EndExceptionHandler

		EndExceptionHandler
	} while (retry);
	RefEnd(lp);
	RefReturn(result);
}

extern Dict *initprims_controlflow(Dict *primdict) {
	primdict = dictput(primdict, STRING(seq), (void *)prim_seq);
	primdict = dictput(primdict, STRING(if), (void *)prim_if);
	primdict = dictput(primdict, STRING(throw), (void *)prim_throw);
	primdict = dictput(primdict, STRING(forever), (void *)prim_forever);
	primdict = dictput(primdict, STRING(catch), (void *)prim_catch);
	return primdict;
}
