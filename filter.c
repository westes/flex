/* filter - postprocessing of flex output through filters */

/*  Copyright (c) 1990 The Regents of the University of California. */
/*  All rights reserved. */

/*  This code is derived from software contributed to Berkeley by */
/*  Vern Paxson. */

/*  The United States Government has rights in this work pursuant */
/*  to contract no. DE-AC03-76SF00098 between the United States */
/*  Department of Energy and the University of California. */

/*  This file is part of flex. */

/*  Redistribution and use in source and binary forms, with or without */
/*  modification, are permitted provided that the following conditions */
/*  are met: */

/*  1. Redistributions of source code must retain the above copyright */
/*     notice, this list of conditions and the following disclaimer. */
/*  2. Redistributions in binary form must reproduce the above copyright */
/*     notice, this list of conditions and the following disclaimer in the */
/*     documentation and/or other materials provided with the distribution. */

/*  Neither the name of the University nor the names of its contributors */
/*  may be used to endorse or promote products derived from this software */
/*  without specific prior written permission. */

/*  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR */
/*  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR */
/*  PURPOSE. */

#include "flexdef.h"

/** global chain. */
struct filter *output_chain = NULL;

/* Allocate and initialize a filter.
 * @param chain the current chain or NULL for new chain
 * @param cmd the command to execute.
 * @param ... a NULL terminated list of (const char*) arguments to command,
 *            not including argv[0].
 * @return newest filter in chain
 */
struct filter *filter_create (struct filter *chain, const char *cmd, ...)
{
	struct filter *f;
	int     max_args;
	const char *s;
	va_list ap;

	/* allocate and initialize new filter */
	f = (struct filter *) flex_alloc (sizeof (struct filter));
	memset (f, 0, sizeof (*f));
	f->next = NULL;
	f->argc = 0;

	if (chain != NULL) {
		/* append f to end of chain */
		while (chain->next)
			chain = chain->next;
		chain->next = f;
	}


	/* allocate argv, and populate it with the argument list. */
	max_args = 8;
	f->argv =
		(const char **) flex_alloc (sizeof (char *) *
					    (max_args + 1));
	f->argv[f->argc++] = cmd;

	va_start (ap, cmd);
	while ((s = va_arg (ap, const char *)) != NULL) {
		if (f->argc >= max_args) {
			max_args += 8;
			f->argv =
				(const char **) flex_realloc (f->argv,
							      sizeof (char
								      *) *
							      (max_args +
							       1));
		}
		f->argv[f->argc++] = s;
	}
	f->argv[f->argc] = NULL;

	va_end (ap);
	return f;
}

/** Fork and exec entire filter chain.
 *  @param chain The head of the chain.
 *  @return true on success.
 */
bool filter_apply_chain (struct filter * chain)
{
	int     pid, pipes[2];

	if (chain == NULL)
		return true;

	fflush (stdout);
	fflush (stderr);

	if (pipe (pipes) == -1)
		flexerror (_("pipe failed"));

	if ((pid = fork ()) == -1)
		flexerror (_("fork failed"));

	if (pid == 0) {
		/* child */
		close (pipes[1]);
		if (dup2 (pipes[0], 0) == -1)
			flexfatal (_("dup2(pipes[0],0)"));
		close (pipes[0]);
		if ((stdin = fdopen (0, "r")) == NULL)
			flexfatal (_("fdopen(0) failed"));

		filter_apply_chain (chain->next);
		execvp (chain->argv[0], (char **const) (chain->argv));
		flexfatal (_("exec failed"));
		exit (1);
	}

	/* Parent */
	close (pipes[0]);
	if (dup2 (pipes[1], 1) == -1)
		flexfatal (_("dup2(pipes[1],1)"));
	close (pipes[1]);
	if ((stdout = fdopen (1, "w")) == NULL)
		flexfatal (_("fdopen(1) failed"));

	return true;
}

/** Truncate the chain to max_len number of filters.
 * @param chain the current chain.
 * @param max_len the maximum length of the chain.
 * @return the resulting length of the chain.
 */
int filter_truncate (struct filter *chain, int max_len)
{
	int     len = 1;

	if (!chain)
		return 0;

	while (chain->next && len < max_len) {
		chain = chain->next;
		++len;
	}

	chain->next = NULL;
	return len;
}

/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
