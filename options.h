#ifndef OPTIONS_H
#define OPTIONS_H
#include "scanopt.h"

extern optspec_t flexopts[];

enum flexopt_flag_t {
    /* Use positive integers only, since they are return codes for scanopt.
     * Order is not important. */
    OPT_7BIT=1,
    OPT_8BIT,
    OPT_ALIGN,
    OPT_ALWAYS_INTERACTIVE,
    OPT_ARRAY,
    OPT_BACKUP,
    OPT_BATCH,
    OPT_CASE_INSENSITIVE,
    OPT_COMPRESSION,
    OPT_CPLUSPLUS,
    OPT_DEBUG,
    OPT_DEFAULT,
    OPT_DONOTHING,
    OPT_ECS,
    OPT_FAST,
    OPT_FULL,
    OPT_HEADER,
    OPT_HELP,
    OPT_INTERACTIVE,
    OPT_LEX_COMPAT,
    OPT_MAIN,
    OPT_META_ECS,
    OPT_NEVER_INTERACTIVE,
    OPT_NODEFAULT,
    OPT_NOLINE,
    OPT_NOWARN,
    OPT_OUTFILE,
    OPT_PERF_REPORT,
    OPT_POINTER,
    OPT_PREFIX,
    OPT_PREPROCDEFINE,
    OPT_READ,
    OPT_REENTRANT,
    OPT_REENTRANT_BISON,
    OPT_SKEL,
    OPT_STACK,
    OPT_STDINIT,
    OPT_STDOUT,
    OPT_TRACE,
    OPT_VERBOSE,
    OPT_VERSION,
    OPT_YYCLASS,
    OPT_YYLINENO,
    OPT_YYWRAP
};

#endif

/* vim:set tabstop=8 softtabstop=4 shiftwidth=4 textwidth=0: */
