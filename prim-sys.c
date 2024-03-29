/* prim-sys.c -- system call primitives */

#define	REQUIRE_IOCTL	1

#include "es.h"
#include "prim.h"
/* TODO: Double check necessity here */
#include <sys/stat.h>

#ifdef HAVE_SETRLIMIT
# define BSD_LIMITS 1
#else
# define BSD_LIMITS 0
#endif

#if BSD_LIMITS || BUILTIN_TIME
#include <sys/time.h>
#include <sys/resource.h>
#if !HAVE_WAIT3
#include <sys/times.h>
#include <limits.h>
#else
#include <time.h>
#endif /* !HAVE_WAIT3 */
#endif /* BSD_LIMITS || BUILTIN_TIME */

static List
*prim_newpgrp(List *list, Binding *binding, int evalflags) {
	int pid;
	if (list != NULL) {
		fail("$&newpgrp", "usage: newpgrp");
	}
	pid = getpid();
	setpgrp(pid, pid);
#ifdef TIOCSPGRP
	{
		Sigeffect sigtstp = esignal(SIGTSTP, sig_ignore);
		Sigeffect sigttin = esignal(SIGTTIN, sig_ignore);
		Sigeffect sigttou = esignal(SIGTTOU, sig_ignore);
		ioctl(2, TIOCSPGRP, &pid);
		esignal(SIGTSTP, sigtstp);
		esignal(SIGTTIN, sigttin);
		esignal(SIGTTOU, sigttou);
	}
#endif
	return true;
}

static List
*prim_background(List *list, Binding *binding, int evalflags) {
	int pid = efork(TRUE, TRUE);
	if (pid == 0) {
#if JOB_PROTECT
		/* job control safe version: put it in a new pgroup. */
		setpgrp(0, getpid());
#endif
		mvfd(eopen("/dev/null", oOpen), 0);
		exit(exitstatus(eval(list, NULL, evalflags | eval_inchild)));
	}
	return mklist(mkstr(str("%d", pid)), NULL);
}

static List
*prim_fork(List *list, Binding *binding, int evalflags) {
	int pid, status;
	pid = efork(TRUE, FALSE);
	if (pid == 0) {
		exit(exitstatus(eval(list, NULL, evalflags | eval_inchild)));
	}
	status = ewaitfor(pid);
	SIGCHK();
	printstatus(0, status);
	return mklist(mkstr(mkstatus(status)), NULL);
}

static List
*prim_run(List *list, Binding *binding, int evalflags) {
	char *file;
	if (list == NULL) {
		fail("$&run", "usage: %%run file argv0 argv1 ...");
	}
	Ref(List *, lp, list);
	file = getstr(lp->term);
	lp = forkexec(file, lp->next, (evalflags & eval_inchild) != 0);
	RefReturn(lp);
}

static List
*prim_umask(List *list, Binding *binding, int evalflags) {
	if (list == NULL) {
		/* XXX: Might need to convert back to an int temporarily if print() doesn't like mode_t and the like */
		mode_t mask = umask(0);
		umask(mask);
		print("%04o\n", mask);
		return true;
	}
	if (list->next == NULL) {
		mode_t mask;
		char *s, *t;
		s = getstr(list->term);
		mask = strtol(s, &t, 8);
		if ((t != NULL && *t != '\0') || ((unsigned) mask) > 07777) {
			fail("$&umask", "bad umask: %s", s);
		}
		if (umask(mask) == (mode_t)(~0)) {
			fail("$&umask", "umask %04o: %s", mask, esstrerror(errno));
		}
		return true;
	}
	fail("$&umask", "usage: umask [mask]");
	NOTREACHED;
}

/*
 * TODO: As a quality of life improvement, this should update
 * two environmental variables: PWD & OLDPWD, which can in
 * turn be used to somewhat simplify the implementation of 
 * pushd/popd and allow for built-in tracking in scripts instead
 * of having to parse `{pwd}
 */
static List
*prim_cd(List *list, Binding *binding, int evalflags) {
	char *dir;
	if (list == NULL || list->next != NULL) {
		fail("$&cd", "usage: $&cd directory");
	}
	dir = getstr(list->term);
	if (chdir(dir) == -1) {
		fail("$&cd", "chdir %s: %s", dir, esstrerror(errno));
	}
	return true;
}

static List
*prim_setsignals(List *list, Binding *binding, int evalflags) {
	int i;
	Sigeffect effects[NSIG]; /* XXX: Unused? */
	for (i = 0; i < NSIG; i++) {
		effects[i] = sig_default;
	}
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next) {
		int sig;
		const char *s = getstr(lp->term);
		Sigeffect effect = sig_catch;
		switch (*s) {
		case '-':	effect = sig_ignore;	s++; break;
		case '/':	effect = sig_noop;	s++; break;
		case '.':	effect = sig_special;	s++; break;
		}
		sig = signumber(s);
		if (sig < 0) {
			fail("$&setsignals", "unknown signal: %s", s);
		}
		effects[sig] = effect;
	}
	RefEnd(lp);
	blocksignals();
	setsigeffects(effects);
	unblocksignals();
	return mksiglist();
}

/*
 * limit builtin -- this is too much code for what it gives you
 */

#if BSD_LIMITS
typedef struct Suffix Suffix;
struct Suffix {
	const char *name;
	long amount;
	const Suffix *next;
};

static const Suffix sizesuf[] = {
	{ "g",	1024*1024*1024,	sizesuf + 1 },
	{ "m",	1024*1024,	sizesuf + 2 },
	{ "k",	1024,		NULL },
};

static const Suffix timesuf[] = {
	{ "h",	60 * 60,	timesuf + 1 },
	{ "m",	60,		timesuf + 2 },
	{ "s",	1,		NULL },
};

typedef struct {
	char *name;
	int flag;
	const Suffix *suffix;
} Limit;

static const Limit limits[] = {

	{ "cputime",		RLIMIT_CPU,	timesuf },
	{ "filesize",		RLIMIT_FSIZE,	sizesuf },
	{ "datasize",		RLIMIT_DATA,	sizesuf },
	{ "stacksize",		RLIMIT_STACK,	sizesuf },
	{ "coredumpsize",	RLIMIT_CORE,	sizesuf },

#ifdef RLIMIT_RSS	/* SysVr4 does not have this */
	{ "memoryuse",		RLIMIT_RSS,	sizesuf },
#endif
#ifdef RLIMIT_VMEM	/* instead, they have this! */
	{ "memorysize",		RLIMIT_VMEM,	sizesuf },
#endif

#ifdef RLIMIT_MEMLOCK	/* 4.4bsd adds an unimplemented limit on non-pageable memory */
	{ "lockedmemory",	RLIMIT_CORE,	sizesuf },
#endif

#ifdef RLIMIT_NOFILE	/* SunOS 4.1 adds a limit on file descriptors */
	{ "descriptors",	RLIMIT_NOFILE,	NULL },
#elif defined(RLIMIT_OFILE) /* but 4.4bsd uses this name for it */
	{ "descriptors",	RLIMIT_OFILE,	NULL },
#endif

#ifdef RLIMIT_NPROC	/* 4.4bsd adds a limit on child processes */
	{ "processes",		RLIMIT_NPROC,	NULL },
#endif

	{ NULL, 0, NULL }
};

static void
printlimit(const Limit *limit, Boolean hard) {
	struct rlimit rlim;
	LIMIT_T lim;
	getrlimit(limit->flag, &rlim);
	if (hard) {
		lim = rlim.rlim_max;
	} else {
		lim = rlim.rlim_cur;
	}
	if (lim == (LIMIT_T) RLIM_INFINITY) {
		print("%-8s\tunlimited\n", limit->name);
	} else {
		const Suffix *suf;

		for (suf = limit->suffix; suf != NULL; suf = suf->next) {
			if (lim % suf->amount == 0 && (lim != 0 || suf->amount > 1)) {
				lim /= suf->amount;
				break;
			}
		}
		print("%-8s\t%d%s\n", limit->name, (int)lim, (suf == NULL || lim == 0) ? "" : suf->name);
	}
}

static LIMIT_T
parselimit(const Limit *limit, char *s) {
	LIMIT_T lim;
	char *t;
	const Suffix *suf = limit->suffix;
	if (streq(s, "unlimited")) {
		return RLIM_INFINITY;
	}
	if (!isdigit(*s)) {
		fail("$&limit", "%s: bad limit value", s);
	}
	if (suf == timesuf && (t = strchr(s, ':')) != NULL) {
		char *u;
		lim = strtol(s, &u, 0) * 60;
		if (u != t) {
			fail("$&limit", "%s %s: bad limit value", limit->name, s);
		}
		lim += strtol(u + 1, &t, 0);
		if (t != NULL && *t == ':') {
			lim = lim * 60 + strtol(t + 1, &t, 0);
		}
		if (t != NULL && *t != '\0') {
			fail("$&limit", "%s %s: bad limit value", limit->name, s);
		}
	} else {
		lim = strtol(s, &t, 0);
		if (t != NULL && *t != '\0') {
			for (;; suf = suf->next) {
				if (suf == NULL) {
					fail("$&limit", "%s %s: bad limit value", limit->name, s);
				}
				if (streq(suf->name, t)) {
					lim *= suf->amount;
					break;
				}
			}
		}
	}
	return lim;
}

static List
*prim_limit(List *list, Binding *binding, int evalflags) {
	const Limit *lim = limits;
	Boolean hard = FALSE;
	Ref(List *, lp, list);

	if (lp != NULL && streq(getstr(lp->term), "-h")) {
		hard = TRUE;
		lp = lp->next;
	}

	if (lp == NULL) {
		for (; lim->name != NULL; lim++) {
			printlimit(lim, hard);
		}
	} else {
		char *name = getstr(lp->term);
		for (;; lim++) {
			if (lim->name == NULL) {
				fail("$&limit", "%s: no such limit", name);
			}
			if (streq(name, lim->name)) {
				break;
			}
		}
		lp = lp->next;
		if (lp == NULL) {
			printlimit(lim, hard);
		} else {
			LIMIT_T n;
			struct rlimit rlim;
			getrlimit(lim->flag, &rlim);
			if ((n = parselimit(lim, getstr(lp->term))) < 0) {
				fail("$&limit", "%s: bad limit value", getstr(lp->term));
			}
			if (hard) {
				rlim.rlim_max = n;
			} else {
				rlim.rlim_cur = n;
			}
			if (setrlimit(lim->flag, &rlim) == -1) {
				fail("$&limit", "setrlimit: %s", esstrerror(errno));
			}
		}
	}
	RefEnd(lp);
	return true;
}
#endif	/* BSD_LIMITS */

#if BUILTIN_TIME
static List
*prim_time(List *list, Binding *binding, int evalflags) {

#if HAVE_WAIT3

	int pid, status;
	time_t t0, t1;
	struct rusage r;

	Ref(List *, lp, list);

	gc();	/* do a garbage collection first to ensure reproducible results */
	t0 = time(NULL);
	pid = efork(TRUE, FALSE);
	if (pid == 0) {
		exit(exitstatus(eval(lp, NULL, evalflags | eval_inchild)));
	}
	status = ewait(pid, FALSE, &r);
	t1 = time(NULL);
	SIGCHK();
	printstatus(0, status);

	eprint(
		"%6ldr %5ld.%ldu %5ld.%lds\t%L\n",
		t1 - t0,
		r.ru_utime.tv_sec, (long) (r.ru_utime.tv_usec / 100000),
		r.ru_stime.tv_sec, (long) (r.ru_stime.tv_usec / 100000),
		lp, " "
	);

	RefEnd(lp);
	return mklist(mkstr(mkstatus(status)), NULL);

/*
 * FIXME: Should not care about wait3(2) anymore, use waitpid(2) instead
 */
#else	/* !HAVE_WAIT3 */

	int pid, status;
	Ref(List *, lp, list);

	gc();	/* do a garbage collection first to ensure reproducible results */
	pid = efork(TRUE, FALSE);
	if (pid == 0) {
		clock_t t0, t1;
		struct tms tms;
		static clock_t ticks = 0;

		if (ticks == 0)
			ticks = CLK_TCK;

		t0 = times(&tms);
		pid = efork(TRUE, FALSE);
		if (pid == 0)
			exit(exitstatus(eval(lp, NULL, evalflags | eval_inchild)));

		status = ewaitfor(pid);
		t1 = times(&tms);
		SIGCHK();
		printstatus(0, status);

		tms.tms_cutime += ticks / 20;
		tms.tms_cstime += ticks / 20;

		eprint(
			"%6ldr %5ld.%ldu %5ld.%lds\t%L\n",
			(t1 - t0 + ticks / 2) / ticks,
			tms.tms_cutime / ticks, ((tms.tms_cutime * 10) / ticks) % 10,
			tms.tms_cstime / ticks, ((tms.tms_cstime * 10) / ticks) % 10,
			lp, " "
		);
		exit(status);
	}
	status = ewaitfor(pid);
	SIGCHK();
	printstatus(0, status);

	RefEnd(lp);
	return mklist(mkstr(mkstatus(status)), NULL);

#endif	/* !HAVE_WAIT3 */

}
#endif	/* BUILTIN_TIME */

extern Dict *initprims_sys(Dict *primdict) {
	primdict = dictput(primdict, STRING(newpgrp), (void *)prim_newpgrp);
	primdict = dictput(primdict, STRING(background), (void *)prim_background);
	primdict = dictput(primdict, STRING(umask), (void *)prim_umask);
	primdict = dictput(primdict, STRING(cd), (void *)prim_cd);
	primdict = dictput(primdict, STRING(fork), (void *)prim_fork);
	primdict = dictput(primdict, STRING(run), (void *)prim_run);
	primdict = dictput(primdict, STRING(setsignals), (void *)prim_setsignals);
#if BSD_LIMITS
	primdict = dictput(primdict, STRING(limit), (void *)prim_limit);
#endif
#if BUILTIN_TIME
	primdict = dictput(primdict, STRING(time), (void *)prim_time);
#endif
	return primdict;
}
