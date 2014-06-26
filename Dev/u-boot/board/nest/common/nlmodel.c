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
 *      This file implements interfaces for working with Nest
 *      model identifiers.
 */

#include <common.h>
#include <malloc.h>

#include <linux/ctype.h>
#include <linux/string.h>

#include "nlmodel.h"

/* Global Variables */

const char *nlmodel_identifier_key = "nlmodel";

/*
 *  const char *nlmodel_identifier()
 *
 *  Description:
 *    This routine attempts to return the platform model identifier,
 *    if set.
 *
 *  Input(s):
 *    N/A
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    A pointer to the identifier if successful; otherwise, NULL;
 *
 */
const char *nlmodel_identifier(void)
{
	return getenv(nlmodel_identifier_key);
}

/*
 *  int nlmodel_init()
 *
 *  Description:
 *    This routine initializes the specified model.
 *
 *  Input(s):
 *    model - A pointer to the model to initialize.
 *
 *  Output(s):
 *    model - A pointer to the initialized model.
 *
 *  Returns:
 *    0 if the model was initialized successfully; otherwise, -1.
 *
 */
int nlmodel_init(struct nlmodel *model)
{
	if (!model)
		return -1;

	model->identifier	= NULL;
	model->family		= NULL;
	model->product		= NL_MODEL_UNKNOWN;
	model->revision		= NL_MODEL_UNKNOWN;

	return 0;
}

/*
 *  int nlmodel_parse()
 *
 *  Description:
 *    This routine attempts to parse the specified model identifier
 *    into its component parts.
 *
 *  Input(s):
 *    indentifier - A pointer to a NULL-terminated C string containing
 *                  the model identifier to parse.
 *    model       - A pointer to the model to populate.
 *
 *  Output(s):
 *    model       - A pointer to the parsed components of the model
 *                  identifier.
 *
 *  Returns:
 *    0 if the identifier was successfully parsed; otherwise, -1.
 *
 */
int nlmodel_parse(const char *identifier, struct nlmodel *model)
{
	int product = NL_MODEL_UNKNOWN;
	int revision = NL_MODEL_UNKNOWN;
	unsigned long value;
	const int base = 10;
	const char dash = '-';
	const char dot = '.';
	const char *dashp, *dotp;
	char *endp;

	if (identifier == NULL || model == NULL)
		return -1;

	/* Attempt to find the last dash ('-') in the identifier. */

	dashp = strrchr(identifier, dash);

	if (dashp == NULL)
		return -1;

	/* Parse and validate the product. */

	value = simple_strtoul(dashp + 1, &endp, base);

	if ((endp > (dashp + 1)) && ((*endp == '\0') || (*endp == dot))) {
		product = value;
	} else {
		return -1;
	}

	/* Parse and validate the optional revision. */

	dotp = strrchr(dashp, dot);

	if (dotp) {
		value = simple_strtoul(dotp + 1, &endp, base);

		if ((endp > (dotp + 1)) && (*endp == '\0')) {
			revision = value;
		}
	}

	/* At this point, everything has been successfully parsed, assign
	 * the results to the caller's model structure.
	 */

	model->identifier	= strdup(identifier);
	model->family		= strndup(identifier, dashp - identifier);
	model->product		= product;
	model->revision		= revision;

	return 0;
}

/*
 *  void nlmodel_destroy()
 *
 *  Description:
 *    This routine frees any resources associated with the specified
 *    model structure.
 *
 *  Input(s):
 *    model - A pointer to the model structure for which to free
 *            associated resources.
 *
 *  Output(s):
 *    model - A pointer to the model structure with its associated
 *            resources freed.
 *
 *  Returns:
 *    N/A
 *
 */
void nlmodel_destroy(struct nlmodel *model)
{
	if (model) {
		if (model->identifier) {
			free((char *)model->identifier);
		}

		if (model->family) {
			free((char *)model->family);
		}

		nlmodel_init(model);
	}
}
