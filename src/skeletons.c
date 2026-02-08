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

/* Combine the format string from *_get_trace_line_format with its arguments. */
void _format_line_directive_out ( const struct flex_backend_t *b, FILE *output_file, char *path, int linenum ) {
	char   directive[MAXLINE*2], filename[MAXLINE];
	char   *s1, *s2, *s3;

	if (!ctrl.gen_line_dirs) {
		return;
	}
	
	/* char *infilename is in the global namespace */
	s1 = (path != NULL) ? path : infilename;

	if ((path != NULL) && !s1) {
		s1 = "<stdin>";
	}
    
	s2 = filename;
	s3 = &filename[sizeof (filename) - 2];

	while (s2 < s3 && *s1) {
		if (*s1 == '\\' || *s1 == '"') {
			/* Escape the '\' or '"' */
			*s2++ = '\\';
		}

		*s2++ = *s1++;
	}

	*s2 = '\0';

	if (path != NULL) {
		snprintf (directive, sizeof(directive), b->get_trace_line_format(b), linenum, filename);
	} else {
		snprintf (directive, sizeof(directive), b->get_trace_line_format(b), 0, filename);
	}

	/* If output_file is nil then we should put the directive in
	 * the accumulated actions.
	 */
	if (output_file) {
		fputs (directive, output_file);
	}
	else {
		add_action (directive);
	}
}

void _format_comment ( const struct flex_backend_t *b, const char *const c ) {
	b->indent(b);
	fputs(b->get_comment(b, c), stdout);
}

/*
void _format_open_table ( const struct flex_backend_t *b );

void _format_continue_table ( const struct flex_backend_t *b );

void _format_close_table ( const struct flex_backend_t *b );
*/

/* Intended to emit a macro call in C/CXX.
   Can also emit a bare string.
 */
void _verbatim ( const struct flex_backend_t *b, const char *s ) {
	if (s)
		fputs(s, stdout);
	
	return;
}

/* Emit a case for the main state switch. 
*/
void _format_normal_state_case_arm ( const struct flex_backend_t *b, int c ) {
	b->indent(b);
	fputs(b->get_normal_state_case_arm(b, c), stdout);
}

/* Emit the special case arm for EOF. */
void _format_eof_state_case_arm ( const struct flex_backend_t *b, const char *const c ) {
	b->indent(b);
	fputs(b->get_eof_state_case_arm(b, c), stdout);
}

/*
void _format_eof_state_case_fallthrough ( const struct flex_backend_t *b );

void _format_eof_state_case_terminate ( const struct flex_backend_t *b );
*/

/* Emit the action preamble. */
void _format_take_yytext ( const struct flex_backend_t *b ) {
	b->indent(b);
	fputs(b->get_take_yytext(b), stdout);
}

/* Emit the action postamble. */
void _format_release_yytext ( const struct flex_backend_t *b ) {
	b->indent(b);
	fputs( b->get_release_yytext(b), stdout);
}

/* Emit the buffer rewind sub-action. */
void _format_char_rewind ( const struct flex_backend_t *b, int c ) {
	b->indent(b);
	fputs(b->get_char_rewind(b, c), stdout);
}

/* Emit the line rewind sub-action. */
void _format_line_rewind ( const struct flex_backend_t *b, int l ) {
	b->indent(b);
	fputs(b->get_line_rewind(b,l), stdout);
}

/* Emit the buffer skip sub-action. */
void _format_char_forward ( const struct flex_backend_t *b, int c ) {
	b->indent(b);
	fputs(b->get_char_forward(b, c), stdout);
}

/* Emit the line skip sub-action. */
void _format_line_forward ( const struct flex_backend_t *b, int l ) {
	b->indent(b);
	fputs(b->get_line_forward(b,l), stdout);
}

/*
void _format_yy_decl ( const struct flex_backend_t *b, const char *d );

void _format_userinit ( const struct flex_backend_t *b, const char *d );
*/

/* Define a string constant. */
void _format_const ( const struct flex_backend_t *b, const char *n, const char *v ) {
	fputs(b->get_const(b, n, v), stdout);
}


/* Inject the rule_setup macro call where needed. */
void _format_rule_setup ( const struct flex_backend_t *b ) {
	fputs(b->get_rule_setup(b), stdout);
	b->newline(b);
}


/* Emit the user_action constant, if needed. */
void _format_user_preaction ( const struct flex_backend_t *b, const char *d ) {
	fputs(b->get_user_preaction(b, d), stdout);
}

/* End a state case arm, optionally inserting user postactions. */
void _format_state_case_break ( const struct flex_backend_t *b ) {
	b->indent(b);
	b->get_state_case_break(b);
}

/* Generate the definition of the STATE_CASE_BREAK end of action. */
void _format_user_postaction ( const struct flex_backend_t *b, const char *d ) {
	fputs(b->get_user_postaction(b, d), stdout);
}

/* Emit the fatal_error action. */
void _format_fatal_error ( const struct flex_backend_t *b, const char *e ) {
	b->indent(b);
	fputs(b->get_fatal_error(b, e), stdout);
}


/* Emit the echo action. */
void _echo ( const struct flex_backend_t *b ) {
	b->indent(b);
	fputs(b->get_echo(b), stdout);
}

void _format_yyterminate ( const struct flex_backend_t *b, const char *d ) {
	fputs(b->get_yyterminate(b, d), stdout);
}

/* Emit the reject special action. */
void _format_yyreject ( const struct flex_backend_t *b ) {
	b->indent(b);
	fputs(b->get_yyreject(b), stdout);
}

/* Define a symbol used by the output filter system. 
   Optionally, leave the definition open to encompass a block of verbatim output.
*/
void _filter_define_name ( const struct flex_backend_t *b, const char *n, const int leave_open ) {
	b->verbatim(b, "m4_define([[");
	b->verbatim(b, n);
	b->verbatim(b, "]], [[");
	if (leave_open)
		b->verbatim(b, "m4_dnl");
	else
		b->verbatim(b, "]])m4_dnl");
	b->newline(b);
}

/* Close a filter symbol definition that was left open by a call to filter_define_name. 
   Optionally, provide a final string of verbatim output to emit before closing the definition block.
*/
void _filter_define_close (const struct flex_backend_t *b, const char *v) {
	b->verbatim(b, v);
	b->verbatim(b, "]])m4_dnl");
	b->newline(b);
}

/* Define a variable used by the output filter system. 
   Provide a string value the filter will substitue for the variable when it is encountered
   later in the output. 
*/
void _filter_define_vars ( const struct flex_backend_t *b, const char *n, const char *v ) {
	b->filter_define_name(b, n, true);
	b->filter_define_close(b, v);
}

/* Define a variable used by the output filter system. 
   Provide a numeric value the filter will substitue for the variable when it is encountered
   later in the output. 
*/
void _filter_define_vard ( const struct flex_backend_t *b, const char *n, const int v ) {
	b->filter_define_name(b, n, true);
	fprintf(stdout, "%d", v);
	b->filter_define_close(b, NULL);
}

/* Format a macro replacement through the output filter system.
   Filter macros are defined like variables. The syntax for defining a filter macro depends on the 
   filter chain in use.
   
   This example assumes the M4 filter chain where: every variable is a macro; the tokens following
   the name are substituted for the macro name; if the first token following the name is an OPAREN,
   it is followed by a comma-delimited list of positional parameters that are themselves substituded
   into the text after the next CPAREN in place of the tokens '$1', '$2', etc.
   
   Flex's own filter macros only use one positional argument, currently.
*/
void _filter_call_macro ( const struct flex_backend_t *b, const char *n, const char *v ) {
	b->verbatim(b, n);
	b->verbatim(b, "( ");
	b->verbatim(b, v);
	b->verbatim(b, " )");
	b->newline(b);
}

/* end */
