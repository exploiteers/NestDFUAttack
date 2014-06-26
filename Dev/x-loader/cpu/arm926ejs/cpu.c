/*
 * (C) Copyright 2004
 * Texas Instruments <www.ti.com>
 * Jian Zhang <jzhang@ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
  
#define TIMER_LOAD_VAL 0xffffffff

/* macro to read the 32 bit timer */
#define READ_TIMER (*(volatile ulong *)(CFG_TIMERBASE+8))

static ulong timestamp;
static ulong lastdec;

static void reset_timer_masked (void);

/* nothing really to do with cpu, just starts up a counter. */
int cpu_init (void)
{
 	int32_t val;

	/* Start the decrementer ticking down from 0xffffffff */
	*((int32_t *) (CFG_TIMERBASE + LOAD_TIM)) = TIMER_LOAD_VAL;
	val = MPUTIM_ST | MPUTIM_AR | MPUTIM_CLOCK_ENABLE | (CFG_PVT << MPUTIM_PTV_BIT);
	*((int32_t *) (CFG_TIMERBASE + CNTL_TIMER)) = val;

	/* init the timestamp and lastdec value */
	reset_timer_masked();
 	
 	return (0);
}

static void reset_timer_masked (void)
{
	/* reset time */
	lastdec = READ_TIMER;  /* capure current decrementer value time */
	timestamp = 0;         /* start "advancing" time stamp from 0 */
}

#ifdef  CFG_UDELAY

static ulong get_timer_masked (void);
static ulong get_timer (ulong base);

/* delay x useconds AND perserve advance timstamp value */
void udelay (unsigned long usec)
{
	ulong tmo, tmp;

	if(usec >= 1000){               /* if "big" number, spread normalization to seconds */
		tmo = usec / 1000;      /* start to normalize for usec to ticks per sec */
		tmo *= CFG_HZ;          /* find number of "ticks" to wait to achieve target */
		tmo /= 1000;            /* finish normalize. */
	}else{                          /* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CFG_HZ;
		tmo /= (1000*1000);
	}

	tmp = get_timer (0);		/* get current timestamp */
	if( (tmo + tmp) < tmp ) 	/* if setting this fordward will roll time stamp */
		reset_timer_masked ();	/* reset "advancing" timestamp to 0, set lastdec value */
	else
		tmo += tmp;		/* else, set advancing stamp wake up time */

	while (get_timer_masked () < tmo)/* loop till event */
		/*NOP*/;
}
 
static ulong get_timer_masked (void)
{
	ulong now = READ_TIMER;		/* current tick value */

	if (lastdec >= now) {		/* normal mode (non roll) */
		/* normal mode */
		timestamp += lastdec - now; /* move stamp fordward with absoulte diff ticks */
	} else {			/* we have overflow of the count down timer */
		/* nts = ts + ld + (TLV - now)
		 * ts=old stamp, ld=time that passed before passing through -1
		 * (TLV-now) amount of time after passing though -1
		 * nts = new "advancing time stamp"...it could also roll and cause problems.
		 */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;

	return timestamp;
}

static ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

#endif 		/* CFG_UDELAY */

