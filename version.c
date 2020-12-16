/* version.c -- version number */
#include "es.h"
static const char id[] = "@(#)nxes version 0.0-alpha0 20201215";
const char * const version = id + (sizeof("@(#)") - 1);
