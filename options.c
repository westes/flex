#include "options.h"

optspec_t  flexopts[] = {

{"--7bit",              OPT_7BIT,0},/* Generate 7-bit scanner. */
{"-7",                  OPT_7BIT,0},
{"--8bit",              OPT_8BIT,0},/* Generate 8-bit scanner. */
{"-8",                  OPT_8BIT,0},
{"--align",             OPT_ALIGN,0},/* Trade off larger tables for better memory alignment. */
{"--always-interactive",OPT_ALWAYS_INTERACTIVE,0},
{"--array",             OPT_ARRAY,0},
{"--backup",            OPT_BACKUP,0},/* Generate backing-up information to lex.backup. */
{"-b",                  OPT_BACKUP,0},
{"--batch",             OPT_BATCH,0},/* Generate batch scanner (opposite of -I). */
{"-B",                  OPT_BATCH,0},
{"--case-insensitive",  OPT_CASE_INSENSITIVE,0},/* Generate case-insensitive scanner. */
{"-i",                  OPT_CASE_INSENSITIVE,0},
{"-C[aefFmr]",          OPT_COMPRESSION,"Specify degree of table compression (default is -Cem)"},
{"--c++",               OPT_CPLUSPLUS,0},/* Generate C++ scanner class. */
{"-+",                  OPT_CPLUSPLUS,0},
{"--debug",             OPT_DEBUG,0},/* Turn on debug mode in generated scanner. */
{"-d",                  OPT_DEBUG,0},
{"--default",           OPT_DEFAULT,0},
{"-c",                  OPT_DONOTHING,0},/* For POSIX lex compatibility. */
{"-n",                  OPT_DONOTHING,0},/* For POSIX lex compatibility. */
{"--ecs",               OPT_ECS,0},/* Construct equivalence classes. */
{"--fast",              OPT_FAST,0},/* Same as -CFr. */
{"-F",                  OPT_FAST,0},
{"--full",              OPT_FULL,0},/* Same as -Cfr. */
{"-f",                  OPT_FULL,0},
{"--header[=FILE]",     OPT_HEADER,0},
{"--help",              OPT_HELP,0},/* Produce this help message. */
{"-?",                  OPT_HELP,0},
{"-h",                  OPT_HELP,0},
{"--interactive",       OPT_INTERACTIVE,0},/* Generate interactive scanner (opposite of -B). */
{"-I",                  OPT_INTERACTIVE,0},
{"--lex-compat",        OPT_LEX_COMPAT,0},/* Maximal compatibility with original lex. */
{"-l",                  OPT_LEX_COMPAT,0},
{"--main",              OPT_MAIN,0}, /* use built-in main() function. */
{"--meta-ecs",          OPT_META_ECS,0},/* Construct meta-equivalence classes. */
{"--never-interactive", OPT_NEVER_INTERACTIVE,0},
{"--nodefault",         OPT_NODEFAULT,0},/* Suppress default rule to ECHO unmatched text. */
{"-s",                  OPT_NODEFAULT,0},
{"--noline",            OPT_NOLINE,0},/* Suppress #line directives in scanner. */
{"-L",                  OPT_NOLINE,0},/* Suppress #line directives in scanner. */
{"--nowarn",            OPT_NOWARN,0},/* Suppress warning messages. */
{"-w",                  OPT_NOWARN,0},
{"--outfile=FILE",      OPT_OUTFILE,0},/* Write to FILE (default is lex.yy.c) */
{"-o FILE",             OPT_OUTFILE,0},
{"--perf-report",       OPT_PERF_REPORT,0},/* Generate performance report to stderr. */
{"-p",                  OPT_PERF_REPORT,0},
{"--pointer",           OPT_POINTER,0},
{"--prefix=PREFIX",     OPT_PREFIX,0},/* Use PREFIX (default is yy) */
{"-P PREFIX",           OPT_PREFIX,0},
{"-Dmacro",             OPT_PREPROCDEFINE,0},/* Define a preprocessor symbol. */
{"--read",              OPT_READ,0},/* Use read(2) instead of stdio. */
{"--reentrant",         OPT_REENTRANT,0},/* Generate a reentrant C scanner. */
{"-R[b]",               OPT_REENTRANT,0},
{"--reentrant-bison",   OPT_REENTRANT_BISON,0},/* Reentrant scanner to be called by a bison pure parser. */
{"--skel=FILE",         OPT_SKEL,0},/* Use skeleton from FILE */
{"-S FILE",             OPT_SKEL,0},
{"--stack",             OPT_STACK,0},
{"--stdinit",           OPT_STDINIT,0},
{"--stdout",            OPT_STDOUT,0},/* Write generated scanner to stdout. */
{"-t",                  OPT_STDOUT,0},
{"--trace",             OPT_TRACE,0},/* Flex should run in trace mode. */
{"-T",                  OPT_TRACE,0},
{"--verbose",           OPT_VERBOSE,0},/* Write summary of scanner statistics to stdout. */
{"-v",                  OPT_VERBOSE,0},
{"--version",           OPT_VERSION,0},/* Report flex version. */
{"-V",                  OPT_VERSION,0},
{"--yyclass=NAME",      OPT_YYCLASS,0},
{"--yylineno",          OPT_YYLINENO,0},
{"--yywrap" ,           OPT_YYWRAP,0},
{0,0,0} /* required final NULL entry.*/
};

/* vim:set tabstop=8 softtabstop=4 shiftwidth=4: */
