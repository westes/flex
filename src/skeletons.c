/* flex - tool to generate fast lexical analyzers */

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
#include "tables.h"
#include "skeletons.h"


/* START digested skeletons */

#include "cpp-backend.h"

#include "c99-backend.h"

const char *go_skel[] = {
/* FIXME: Refactor like cpp-backend when Go backend is ready.
#include "go-flex.h"
*/
    0,
};

/* END digested skeletons */


/* backends is an array of skeleton code-emitting backend structs.
/* It uses the flex_backend_t enum so we don't have to care what order it's in.
 */
static struct flex_backend_t backends[FLEX_BACKEND_ID_MAX];

/* backend_stack is an array-based stack of flex_backend_ids.                  
/* It lets us keep track of which skeleton and emitter are in use and allows
/* us to switch to another backend during runtime, for example to dump an 
/* auxiliary table. 
/* The need to switch backends should be uncommon, the stack is only as deep
/* as the number of available backends. 
/* backen_stack_head is declared static so it must be manipulated through
/* the stack functions defined in this module.
 */
flex_backend_id_t backend_stack[FLEX_BACKEND_ID_MAX+1];
static unsigned int backend_stack_head = 0;

/* Push a flex_backend_id onto the backend stack. 
/* Returns false when the stack is full, and true otherwise.
 */
bool push_backend(flex_backend_id_t bid){
	++backend_stack_head;
	if( backend_stack_head >= FLEX_BACKEND_ID_MAX ){
		--backend_stack_head;
		flexerror(_("backend stack depth exceeded"));
		return false;
	}
	else {
		backend_stack[backend_stack_head-1] = bid;
	}
	return true;
}

/* Pop a flex_backend_id off of the backend stack.
/* Returns the FLEX_BACKEND_ID_MAX when the stack is empty, and the
/* popped backend id otherwise. */
flex_backend_id_t pop_backend(void) {
	flex_backend_id_t ret = FLEX_BACKEND_ID_MAX;
	if( backend_stack_head > 0 ) {
		ret = backend_stack[backend_stack_head-1];
		--backend_stack_head;
		return ret;
	}
	else {
		flexerror(_("attempt to pop empty backend stack"));
		return ret;
	}
}

/* Return the flex_backend_id on top of the backend stack.
/* Returns the FLEX_BACKEND_ID_MAX when the stack is empty, and the
/* top backend id otherwise. */
flex_backend_id_t top_backend(void){
	if( backend_stack_head > 0 ) {
		return backend_stack[backend_stack_head-1];
	}
	else {
		flexerror(_("attempt to read the top of empty backend stack"));
		return FLEX_BACKEND_ID_MAX;
	}
}




const struct flex_backend_t *get_backend(void) {
	return &backends[top_backend()];
}

/* Initialize backends */
void init_backends( void ) {
	backends[FLEX_BACKEND_CPP] = cpp_backend;
	backends[FLEX_BACKEND_C99] = c99_backend;
	backends[FLEX_BACKEND_GO].skel=go_skel;
	backends[FLEX_BACKEND_ID_MAX] = (struct flex_backend_t){NULL};
}

/* Functions for querying skeleton properties. */
const char *_skel_property(const flex_backend_id_t backend_id, const char *propname);
static bool _boneseeker(const flex_backend_id_t backend_id, const char *bone);

/* TODO: What does this mean now? */
bool is_default_backend(void)
{
    return top_backend() == FLEX_BACKEND_DEFAULT;
}

/* Search for a string in the skeleton prolog, where macros are defined.
 */
static bool boneseeker(const char *bone) {
	return _boneseeker(top_backend(), bone);
}

static bool _boneseeker(const flex_backend_id_t backend_id, const char *bone)
{
	int i;

	const struct flex_backend_t *backend = &backends[backend_id];

	for (i = 0; backend->skel[i] != NULL; i++) {
		const char *line = backend->skel[i];
		if (strstr(line, bone) != NULL)
			return true;
		else if (strncmp(line, "%%", 2) == 0)
			break;
	}
	return false;
}

flex_backend_id_t backend_by_name(const char *name)
{
	const char *prefix_property;
	flex_backend_id_t backend_id = FLEX_BACKEND_DEFAULT, i = FLEX_BACKEND_DEFAULT;

	if (name != NULL) {
		if (strcmp(name, "nr") == 0) {
			backend_id = FLEX_BACKEND_CPP;
			ctrl.reentrant = false;
			goto backend_ok;
		}
		if (strcmp(name, "r") == 0) {
			backend_id = FLEX_BACKEND_CPP;
			ctrl.reentrant = true;
			goto backend_ok;
		}
		for (i = 0; backends[i].skel != NULL && i < FLEX_BACKEND_ID_MAX; ++i) {
			if (strcasecmp(_skel_property(i, "M4_PROPERTY_BACKEND_NAME"), name) == 0) {
				backend_id = i;
				goto backend_ok;
			}
		}
		flexerror(_("no such back end"));
		return FLEX_BACKEND_ID_MAX;
	}
  backend_ok:
	ctrl.rewrite = !is_default_backend();
	ctrl.backend_name = xstrdup(_skel_property(backend_id, "M4_PROPERTY_BACKEND_NAME"));
	ctrl.traceline_re = xstrdup(_skel_property(backend_id, "M4_PROPERTY_TRACE_LINE_REGEXP"));
	ctrl.traceline_template = xstrdup(_skel_property(backend_id, "M4_PROPERTY_TRACE_LINE_TEMPLATE"));
	ctrl.have_state_entry_format = _boneseeker(backend_id, "m4_define([[M4_HOOK_STATE_ENTRY_FORMAT]]");
	prefix_property = _skel_property(backend_id, "M4_PROPERTY_PREFIX");
	if (prefix_property != NULL)
		ctrl.prefix = xstrdup(prefix_property);
	flex_init_regex(ctrl.traceline_re);
	return backend_id;
}

const char *suffix (void)
{
	const char   *suffix;

	if (is_default_backend()) {
		if (ctrl.C_plus_plus)
			suffix = "cc";
		else
			suffix = "c";
	} else {
		suffix = skel_property("M4_PROPERTY_SOURCE_SUFFIX");
	}
	
	return suffix;
}

/* Search for a m4 define of the property key, retrieve the value.  The
 * definition must be single-line.  Don't call this a second time before
 * stashing away the previous return, we cheat with static buffers.
 */
const char *skel_property(const char *propname){
	return _skel_property(top_backend(), propname);
}

const char *_skel_property(const flex_backend_id_t backend_id, const char *propname)
{
	int i;
	static char name[256], value[256], *np, *vp;;
	const char *cp;
	const struct flex_backend_t *backend = &backends[backend_id];

	for (i = 0; backend->skel[i] != NULL; i++) {
		const char *line = backend->skel[i];
		if (line[0] == '\0')
			continue;
		/* only scan before first skell breakpoint */
		if (strncmp(line, "%%", 2) == 0)
			break;
		/* ignore anything that's not a definition */
		if (strncmp(line, "m4_define(", 10) != 0)
			continue;
		/* skip space and quotes before macro name */
		for (cp = line + 10; isspace(*cp) || *cp == '['; *cp++)
			continue;
		/* copy up to following ] into the name buffer */
		np = name;
		while (*cp != ']' && *cp != '\0' && (np < name + sizeof(name)-1)) {
			*np++ = *cp++;
		}
		*np = '\0';
		/* check for valid and matching name */
		if (*cp == ']') {
			if (strcmp(name, propname) != 0)
				continue; /* try next line */
		} else {
			flexerror(_("unterminated or too long property name"));
			continue;
		}
		/* skip to the property value */
		while (*cp != '\0' && (*cp == ']' || isspace(*cp) || *cp == ','))
			cp++;
		while (*cp == '[')
			cp++;
		if (*cp == '\0')
			flexerror(_("garbled property line"));
		/* extract the value */
		vp = value;
		while (*cp != '\0' && vp < value + sizeof(value) - 1 && (cp[0] != ']' || cp[1] != ']'))
			*vp++ = *cp++;
		if (*cp == ']') {
			*vp = '\0';
			return value;
		} else {
			flexerror(_("unterminated or too long property value"));
		}
	}
	return NULL;
}

/* skelout - write out one section of the skeleton file
 *
 * Description
 *    Copies skelfile or skel array to stdout until a line beginning with
 *    "%%" or EOF is found.
 */
void skelout (bool announce)
{
	char    buf_storage[MAXLINE];
	char   *buf = buf_storage;
	bool   do_copy = true;
	const struct flex_backend_t *backend = get_backend();

	/* Loop pulling lines either from the skelfile, if we're using
	 * one, or from the selected back end's skel[] array.
	 */
	while (env.skelfile != NULL ?
	       (fgets (buf, MAXLINE, env.skelfile) != NULL) :
	       ((buf = (char *) backend->skel[skel_ind++]) != 0)) {

		if (env.skelfile != NULL)
			chomp (buf);

		/* copy from skel array */
		if (buf[0] == '%') {	/* control line */
			/* print the control line as a comment. */
			if (ctrl.ddebug && buf[1] != '#') {
			    backend->comment(backend, buf);
				backend->newline(backend);
			}
			if (buf[1] == '#') {
				/* %# indicates comment line to be ignored */
			} 
			else if (buf[1] == '%') {
				/* %% is a break point for skelout() */
				if (announce) {
					backend->comment(backend, buf);
					backend->newline(backend);
				}
				return;
			}
			else {
				flexfatal (_("bad line in skeleton file"));
			}
		}

		else if (do_copy) {
			backend->verbatim(backend, buf);
			backend->newline(backend);
		}
	}			/* end while */
}


/* end */
