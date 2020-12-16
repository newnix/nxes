/* prim.c -- primitives and primitive dispatching */

#include "es.h"
#include "prim.h"

static Dict *prims;

extern List *prim(char *s, List *list, Binding *binding, int evalflags) {
	List *(*p)(List *, Binding *, int);
	p = (List *(*)(List *, Binding *, int)) dictget(prims, s);
	if (p == NULL)
		fail("es:prim", "unknown primitive: %s", s);
	return (*p)(list, binding, evalflags);
}

static List
*prim_primitives(List *list, Binding *binding, int evalflags) {
	static List *primlist = NULL;
	if (primlist == NULL) {
		globalroot(&primlist);
		dictforall(prims, addtolist, &primlist);
		primlist = sortlist(primlist);
	}
	return primlist;
}

extern void initprims(void) {
	prims = mkdict();
	globalroot(&prims);

	prims = initprims_controlflow(prims);
	prims = initprims_io(prims);
	prims = initprims_etc(prims);
	prims = initprims_sys(prims);
	prims = initprims_proc(prims);
	prims = initprims_access(prims);

	/*
	 * TODO: This is forbidden by ISO C, so 
	 * once I'm more familiar with the codebase,
	 * this needs to be corrected
	 */
	(prims = dictput(prims, STRING(primitives), (void *)prim_primitives));
}
