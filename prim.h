/* prim.h -- definitions for es primitives */

/*
 * TODO: These functions seem to exist solely to expand what
 * appears to be a global dictionary in a way that allows
 * access to the functions defined with static visibility.
 * I don't see any particular problems with this approach other than the 
 * relative verbosity in expanding the dictionary once the 
 * macros are expanded.
 * It may be better to have this dict defined and accessible through
 * an interface in prim.c instead, but testing on size differences and 
 * performance would need to be conducted to see if the potentially
 * more involved process is worthwhile.
 */
extern Dict *initprims_controlflow(Dict *primdict);	/* prim-ctl.c */
extern Dict *initprims_io(Dict *primdict);		/* prim-io.c */
extern Dict *initprims_etc(Dict *primdict);		/* prim-etc.c */
extern Dict *initprims_sys(Dict *primdict);		/* prim-sys.c */
extern Dict *initprims_proc(Dict *primdict);		/* proc.c */
extern Dict *initprims_access(Dict *primdict);		/* access.c */
