/*
 *    Copyright (c) 2010-2011 Nest Labs, Inc.
 *
 *      Author: Grant Erickson <grant@nestlabs.com>
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 2 as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this program. If not, see
 *    <http://www.gnu.org/licenses/>.
 *
 *    Description:
 *      This file defines platform-specific data for the Samsung
 *      LMS350DF03 LCD module driver.
 */

#ifndef _INCLUDE_LINUX_SPI_LMS350DF03_H_
#define _INCLUDE_LINUX_SPI_LMS350DF03_H_

struct lms350df03_platform_data {
	struct {
		unsigned long		gpio;
		bool				inverted;
	} reset;
	struct {
		const char			*vcc;
	} regulator;
};

#endif /* _INCLUDE_LINUX_SPI_LMS350DF03_H_ */
