/* version.c -- version number ($Revision: 1.2 $) */
#include "es.h"
static const char id[] = "@(#)nxes version 0.0-alpha0 20201010";
const char * const version = id + (sizeof("@(#)") - 1);
