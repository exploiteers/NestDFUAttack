/*
 *    Copyright (c) 2011 Nest, Inc.
 *
 *    See file CREDITS for list of people who contributed to this
 *    project.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License as
 *    published by the Free Software Foundation; either version 2 of
 *    the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this program; if not, write to the Free
 *    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA 02111-1307 USA
 *
 *    Description:
 *      This file defines data types and interfaces for working with
 *      Nest model identifiers.
 */

#ifndef _NLMODEL_H_
#define _NLMODEL_H_

enum {
	NL_MODEL_UNKNOWN = -1
};

struct nlmodel {
	const char *identifier;
	const char *family;
	int product;
	int revision;
};

extern const char *nlmodel_identifier_key;

extern const char *nlmodel_identifier(void);
extern int nlmodel_init(struct nlmodel *model);
extern int nlmodel_parse(const char *indentifier, struct nlmodel *model);
extern void nlmodel_destroy(struct nlmodel *model);

#endif /* _NLMODEL_H_ */
