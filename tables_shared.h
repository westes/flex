/*  tables_shared.h - tables serialization header
 *
 *  Copyright (c) 1990 The Regents of the University of California.
 *  All rights reserved.
 *
 *  This code is derived from software contributed to Berkeley by
 *  Vern Paxson.
 *
 *  The United States Government has rights in this work pursuant
 *  to contract no. DE-AC03-76SF00098 between the United States
 *  Department of Energy and the University of California.
 *
 *  This file is part of flex.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Neither the name of the University nor the names of its contributors
 *  may be used to endorse or promote products derived from this software
 *  without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE.
 */

/* This file is meant to be included in both the skeleton and the actual
 * flex code (hence the name "_shared").
 */

/* Structures and prototypes for serializing flex tables.  The
 * binary format is documented in the manual.
 *
 * Design considerations:
 *
 *  -  The format allows many tables per file.
 *  -  The tables can be streamed.
 *  -  All data is stored in network byte order.
 *  -  We do not hinder future unicode support.
 *  -  We can lookup tables by name.
 */

/** Magic number for serialized format. */
#define YYTBL_MAGIC 0xF13C57B1

/** Possible values for t_id field. Each one corresponds to a
 *  scanner table of the same name.
 */
enum yytbl_id {
	YYT_ID_ACCEPT = 0x01,
	YYT_ID_BASE = 0x02,
	YYT_ID_CHK = 0x03,
	YYT_ID_DEF = 0x04,
	YYT_ID_EC = 0x05,
	YYT_ID_META = 0x06,
	YYT_ID_NUL_TRANS = 0x07,
	YYT_ID_NXT = 0x08,		/**< may be 2 dimensional array */
	YYT_ID_RULE_CAN_MATCH_EOL = 0x09,
	YYT_ID_START_STATE_LIST = 0x0A,	/**< array of pointers */
	YYT_ID_TRANSITION = 0x0B	/**< array of structs */
};

/** bit flags for t_flags field of struct yytbl_data */
enum yytbl_flags {
	YYT_DATA8 = 0x01,   /**< data is an array of type int8_t */
	YYT_DATA16 = 0x02,  /**< data is an array of type int16_t */
	YYT_DATA32 = 0x04,  /**< data is an array of type int32_t */
	YYT_PTRANS = 0x08,  /**< data is a list of indexes of entries
                                 into the expanded `yy_transition'
                                 array. See notes in manual. */
};

/* The serialized tables header. */
struct yytbl_hdr {
	uint32_t th_magic;  /**< Must be 0xF13C57B1 (comes from "Flex Table") */
	uint32_t th_hsize;  /**< Size of this header in bytes. */
	uint32_t th_ssize;  /**< Size of this dataset, in bytes, including header. */
	uint16_t th_flags;  /**< Currently unused, must be 0 */
	char   *th_version; /**< Flex version string. NUL terminated. */
	char   *th_name;    /**< The name of this table set. NUL terminated. */
};

/** A single serialized table */
struct yytbl_data {
	enum yytbl_id t_id; /**< table identifier */
	uint16_t t_flags;   /**< how to interpret this data */
	uint32_t t_hilen;   /**< num elements in highest dimension array */
	uint32_t t_lolen;   /**< num elements in lowest dimension array */
	void   *t_data;	    /**< table data */
};
